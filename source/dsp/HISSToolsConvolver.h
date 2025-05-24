
#pragma once

#include "Convolver.h"
#include "IPlugConstants.h"

BEGIN_IPLUG_NAMESPACE

class HISSToolsConvolver {
public:
    HISSToolsConvolver():
        mConvolver(1, 1, kLatencyZero)
    {}
    ~HISSToolsConvolver() {}

    void OnReset() {
        mConvolver.reset();
    }

    ConvolveError SetIr(float* ir, size_t length) {
        ConvolveError err = mConvolver.set(0, 0, ir, length, true);
        mCanProcess = (err == CONVOLVE_ERR_NONE);
        return err;
    }

    ConvolveError SetIr(double* ir, size_t length) {
        ConvolveError err = mConvolver.set(0, 0, ir, length, true);
        mCanProcess = (err == CONVOLVE_ERR_NONE);
        return err;
    }

    void process(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
        if (mCanProcess) {
            mConvolver.process(inputs, outputs, 1, 1, (size_t)nFrames);
            return;
        }
        for (int s = 0; s < nFrames; s++) {
            outputs[0][s] = inputs[0][s];
        }
    }

private:
    HISSTools::Convolver mConvolver;

    bool mCanProcess = false;
};

END_IPLUG_NAMESPACE