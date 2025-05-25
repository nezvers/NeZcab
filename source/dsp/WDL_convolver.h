#pragma once

#include "convoengine.h"
#include "IPlugConstants.h"
#include <math.h>
#include <memory>

BEGIN_IPLUG_NAMESPACE

class WdlConvolver {
public:
    WdlConvolver(){}
    ~WdlConvolver() {}

    void OnReset() {
        if (mEngine != nullptr) {
            mEngine->Reset();
        }
    }

    int SetIr(float* ir, size_t length, double sampleRate, int blocksize) {
        mCanProcess = false;
        mImpulse = std::make_unique<WDL_ImpulseBuffer>();

        mImpulse->SetNumChannels(1);
        if (length > mImpulse->SetLength(length)) {
            return 1;
        }
        mImpulse->samplerate = sampleRate;

        WDL_FFT_REAL* buffPtr = mImpulse->impulses[0].Get();
        for (int i = 0; i < length; i++) {
            buffPtr[i] = static_cast<WDL_FFT_REAL>(ir[i]);
        }

        mEngine = std::make_unique< WDL_ConvolutionEngine_Div>();
        if (mEngine == nullptr) {
            return 1;
        }

        mEngine->Reset();
        mEngine->SetImpulse(mImpulse.get(), 0, blocksize);
        mCanProcess = true;
        return 0;
    }

    int SetIr(double* ir, size_t length, double sampleRate, int blocksize) {
        mCanProcess = false;
        mImpulse = std::make_unique<WDL_ImpulseBuffer>();

        mImpulse->SetNumChannels(1);
        if (length > mImpulse->SetLength(length)) {
            return 1;
        }
        mImpulse->samplerate = sampleRate;

        WDL_FFT_REAL* buffPtr = mImpulse->impulses[0].Get();
        for (int i = 0; i < length; i++) {
            buffPtr[i] = static_cast<WDL_FFT_REAL>(ir[i]);
        }

        mEngine = std::make_unique< WDL_ConvolutionEngine_Div>();
        if (mEngine == nullptr) {
            return 1;
        }

        mEngine->Reset();
        mEngine->SetImpulse(mImpulse.get(), 0, blocksize);
        mCanProcess = true;
        return 0;
    }

    int GetLatency() {
        if (mEngine == nullptr) { return 0; }
        return mEngine->GetLatency();
    }

    void process(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
        iplug::sample* inPtr = inputs[0];
        iplug::sample* outPtr = outputs[0];
        if (mCanProcess) {
            mEngine->Add((WDL_FFT_REAL**)inputs, nFrames, 1);
            int nAvailableSamples = wdl_min(mEngine->Avail(nFrames), nFrames);

            // If not enough samples are available yet, then only output the dry signal
            const unsigned long unprocessed = nFrames - nAvailableSamples;

            if (nAvailableSamples < nFrames) {
                memset(outPtr, 0, unprocessed);
            }

            WDL_FFT_REAL* convOut = mEngine->Get()[0];
            if ((nAvailableSamples > 0) && (nFrames > 0) && (nAvailableSamples <= nFrames)) {
                memcpy(&outPtr[unprocessed], convOut, nAvailableSamples * sizeof(iplug::sample));
            }
            mEngine->Advance(nAvailableSamples);
            return;
        }
        for (int s = 0; s < nFrames; s++) {
            outPtr[s] = inPtr[s];
        }
    }

private:
    std::unique_ptr<WDL_ImpulseBuffer> mImpulse;
    std::unique_ptr<WDL_ConvolutionEngine_Div> mEngine;

    bool mCanProcess = false;
};

END_IPLUG_NAMESPACE