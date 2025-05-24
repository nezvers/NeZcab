#pragma once

#include "IAudioFile.h"
#include "HISSTools_Pointers.hpp"
#include "Resampler.h"
#include <memory>
#include <vector>
#include <cmath>
#include "string.h"

#include "resample.h"
#include "CDSPResampler.h"
#include "fft.h"
using namespace r8b;




class IrBuffer {
public:
    enum ResamplerType { WDL_RESAMPLER, CUSTOM_RESAMPLE, LINEAR_RESAMPLE, R8BRAIN_RESAMPLE, RESAMPLE_COUNT };

    IrBuffer(double sampleRate, enum ResamplerType resamplerType = R8BRAIN_RESAMPLE) :
        mSampleRate(sampleRate),
        mResamplerType(resamplerType)
    {
    }

    void SetResampler(ResamplerType resamplerType = R8BRAIN_RESAMPLE) {
        mResamplerType = resamplerType;

        if (baseIR == nullptr) { return; }

        if (Resample() != 0) {
            return;
        }
        mIsStaged = true;
    }

    void OnReset(double sampleRate) {
        if (sampleRate == mSampleRate) { return; }
        mSampleRate = sampleRate;

        if (baseIR == nullptr) { return; }

        if (Resample() != 0) {
            return;
        }
        mIsStaged = true;
    }

    int LoadIr(WDL_String& filePath, WDL_String& directory) {
        HISSTools::IAudioFile file(filePath.Get());
        if (file.getIsError()) {
            // TODO: return error
            return 1;
        }

        // TODO: check if valid audio file
        baseIR = std::make_unique<HISSTools_RefPtr<float>>(file.getFrames());
        file.readChannel(baseIR->get(), file.getFrames(), 0);
        mBaseSampleRate = file.getSamplingRate();

        if (Resample() != 0) {
            baseIR = nullptr;
            mBaseSampleRate = 0.;
            return 1;
        }
        int sampleSize = sizeof(mIR->get()[0]);

        mIsStaged = true;

        mFilePath = filePath;
        mDirPath = directory;
        return 0;
    }

    WDL_FFT_REAL* Get() {
        return mIR->get();
    }

    size_t GetSize() {
        return mIR->getSize();
    }


    bool mIsStaged = false;
private:
    int ResampleLength(int srcLength, double srcRate, double destRate) {
        return int(ceil(destRate / srcRate * (double)srcLength));
    }

    int Resample() {
        if ((int)mBaseSampleRate == (int)mSampleRate) {
            unsigned long sampleCount = baseIR->getSize();
            mIR = std::make_unique<HISSTools_RefPtr<WDL_FFT_REAL>>(sampleCount);

            if (mIR == nullptr) { return 1; }

            for (int i = 0; i < sampleCount; i++) {
                (*mIR)[i] = static_cast<WDL_FFT_REAL>((*baseIR)[i]);
            }
            return 0;
        }
        switch (mResamplerType) {
        case WDL_RESAMPLER:
            return ResampleWDL();
            break;
        case R8BRAIN_RESAMPLE:
            return ResampleR8brain();
            break;
        case CUSTOM_RESAMPLE:
            return ResampleCustom();
            break;
        case LINEAR_RESAMPLE:
            return ResampleLinear();
            break;
        }
        return 1;
    }

    int ResampleCustom() {
        unsigned long outLength = 0;
        Resampler resampler;
        float* temp = resampler.process(baseIR->get(), baseIR->getSize(), outLength, mBaseSampleRate, mSampleRate);
        if (outLength == 0) {
            if (temp != NULL) {
                delete[] temp;
            }
            return 1;
        }

        mIR = std::make_unique<HISSTools_RefPtr<WDL_FFT_REAL>>(outLength);

        for (unsigned long i = 0; i < outLength; i++) {
            (*mIR)[i] = static_cast<WDL_FFT_REAL>(temp[i]);
        }

        delete[] temp;
        return 0;
    }
    int ResampleWDL() {
        WDL_Resampler resampler;
        double srcRate = mBaseSampleRate;
        double dstRate = mSampleRate;
        constexpr unsigned long blockLength = 64;


        unsigned long srcLength = baseIR->getSize();
        unsigned long dstLength = ResampleLength(srcLength, srcRate, dstRate);
        std::unique_ptr<HISSTools_RefPtr<WDL_FFT_REAL>> tempBuff = std::make_unique<HISSTools_RefPtr<WDL_FFT_REAL>>(dstLength);
        float* pSrc = baseIR->get();
        WDL_FFT_REAL* pDest = tempBuff->get();

        resampler.SetRates(srcRate, dstRate);
        resampler.SetFeedMode(true);
        double scale = srcRate / dstRate;

        while (dstLength > 0) {
            WDL_ResampleSample* p;
            int n = resampler.ResamplePrepare(blockLength, 1, &p);
            int m = n;

            if (n > srcLength) {
                n = srcLength;
            }
            for (int i = 0; i < n; ++i) {
                *p++ = (WDL_ResampleSample)*pSrc++;
            }
            if (n < m) {
                memset(p, 0, (m - n) * sizeof(WDL_ResampleSample));
            }
            srcLength -= n;

            WDL_ResampleSample buf[blockLength];
            n = resampler.ResampleOut(buf, m, m, 1);
            if (n > dstLength) {
                n = dstLength;
            }
            p = buf;
            for (int i = 0; i < n; ++i) {
                *pDest++ = static_cast<WDL_FFT_REAL>(scale * *p++);
            }
            dstLength -= n;
        }
        resampler.Reset();
        mIR = std::move(tempBuff);

        return 0;
    }

    int ResampleLinear() {
        WDL_Resampler resampler;
        double srcRate = mBaseSampleRate;
        double dstRate = mSampleRate;


        unsigned long srcLength = baseIR->getSize();
        unsigned long dstLength = ResampleLength(srcLength, srcRate, dstRate);
        std::unique_ptr<HISSTools_RefPtr<WDL_FFT_REAL>> tempBuff = std::make_unique<HISSTools_RefPtr<WDL_FFT_REAL>>(dstLength);
        float* pSrc = baseIR->get();
        WDL_FFT_REAL* pDest = tempBuff->get();

        double pos = 0.;
        double delta = srcRate / dstRate;
        for (int i = 0; i < dstLength; ++i) {
            int idx = int(pos);
            if (idx < srcLength) {
                double frac = pos - floor(pos);
                double interp = (1. - frac) * pSrc[idx];
                if (++idx < srcLength) interp += frac * pSrc[idx];
                pos += delta;
                *pDest++ = static_cast<WDL_FFT_REAL>(delta * interp);
            }
            else {
                *pDest++ = 0;
            }
        }
        mIR = std::move(tempBuff);

        return 0;
    }

    int ResampleR8brain() {
        double srcRate = mBaseSampleRate;
        double dstRate = mSampleRate;
        unsigned long srcLength = baseIR->getSize();
        float* pSrc = baseIR->get();

        std::unique_ptr<CDSPResampler16IR> resampler = std::make_unique<CDSPResampler16IR>(srcRate, dstRate, srcLength);
        unsigned long dstLength = resampler->getMaxOutLen(0);

        if (!(dstLength > 0)) {
            return 1;
        }

        std::unique_ptr<HISSTools_RefPtr<WDL_FFT_REAL>> tempBuff = std::make_unique<HISSTools_RefPtr<WDL_FFT_REAL>>(dstLength);
        resampler->oneshot(pSrc, srcLength, tempBuff->get(), dstLength);


        WDL_FFT_REAL* pDest = tempBuff->get();
        mIR = std::move(tempBuff);

        return 0;
    }

    double mSampleRate = 0.;
    double mBaseSampleRate = 0.;
    std::unique_ptr<HISSTools_RefPtr<float>> baseIR;
    std::unique_ptr<HISSTools_RefPtr<WDL_FFT_REAL>> mIR;
    WDL_String mFilePath;
    WDL_String mDirPath;
    ResamplerType mResamplerType;
};
