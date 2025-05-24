#pragma once

#include "convoengine.h"
#include "IPlugConstants.h"
#include <math.h>

BEGIN_IPLUG_NAMESPACE

class WdlConvolver {
public:
    WdlConvolver()
    {
        mImpulse.SetNumChannels(1);
    }
    ~WdlConvolver() {}

    void OnReset(double sampleRate) {
        mImpulse.samplerate = sampleRate;
        mEngine.Reset();
    }

    int SetIr(float* ir, size_t length, double sampleRate) {
        mCanProcess = false;

        mImpulse.samplerate = sampleRate;
        if (length > mImpulse.SetLength(length)) {
            mCanProcess = false;
            return 1;
        }

        WDL_FFT_REAL* buffPtr = mImpulse.impulses[0].Get();
        for (int i = 0; i < length; i++) {
            buffPtr[i] = static_cast<WDL_FFT_REAL>(ir[i]);
        }

        mEngine.SetImpulse(&mImpulse);
        mCanProcess = true;
        return 0;
    }

    int SetIr(double* ir, size_t length, double sampleRate) {
        mCanProcess = false;

        mImpulse.samplerate = sampleRate;
        if (length > mImpulse.SetLength(length)) {
            mCanProcess = false;
            return 1;
        }

        WDL_FFT_REAL* buffPtr = mImpulse.impulses[0].Get();
        for (int i = 0; i < length; i++) {
            buffPtr[i] = static_cast<WDL_FFT_REAL>(ir[i]);
        }

        mEngine.SetImpulse(&mImpulse);
        mCanProcess = true;
        return 0;
    }

    int GetLatency() {
        return mEngine.GetLatency();
    }

    void process(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
        if (mCanProcess) {
            // TODO: Hopefully match to WDL_FFT_REAL type
            mEngine.Add((WDL_FFT_REAL**)inputs, nFrames, 1);
            int nAvailableSamples = wdl_min(mEngine.Avail(nFrames), nFrames);

            // If not enough samples are available yet, then only output the dry signal
            unsigned long unprocessed = nFrames - nAvailableSamples;
            unsigned long i = 0;
            for (; i < unprocessed; ++i) {
                outputs[0][i] = inputs[0][i];
            }
            if (nAvailableSamples > 0)
            {
                // Apply the dry/wet mix
                WDL_FFT_REAL** pWetSignal = mEngine.Get();
                for (auto s = i; s < nAvailableSamples; ++s) {
                    outputs[0][s] = inputs[0][i];
                }

                // Remove the sample block from the convolution engine's buffer
                mEngine.Advance(nAvailableSamples);
            }
            return;
        }
        for (int s = 0; s < nFrames; s++) {
            outputs[0][s] = inputs[0][s];
        }
    }

private:
    WDL_ImpulseBuffer mImpulse;
    WDL_ConvolutionEngine mEngine;

    bool mCanProcess = false;
};

END_IPLUG_NAMESPACE