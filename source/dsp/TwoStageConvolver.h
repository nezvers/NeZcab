#pragma once

#include "IPlugConstants.h"
#include "TwoStageFFTConvolver.h"
#include <vector>

BEGIN_IPLUG_NAMESPACE

class TwoStageConvolver {
public:
    TwoStageConvolver()
    {
    }
    ~TwoStageConvolver() {}

    void OnReset() {
        /*if (mConvolver == nullptr) { return; }
        mConvolver.reset();*/
    }

    int SetIr(float* ir, size_t length) {
        std::unique_ptr<fftconvolver::TwoStageFFTConvolver> temp = std::make_unique<fftconvolver::TwoStageFFTConvolver>();
        if (temp == nullptr) { return -1; }

        mIR.resize(length);
        for (int i = 0; i < length; i++) {
            mIR[i] = static_cast<fftconvolver::Sample>(ir[i]);
        }

        if (!temp->init(kHeadBlockSize, kTailBlockSize, mIR.data(), length)) {
            return -1;
        }

        mConvolver = std::move(temp);

        mCanProcess = true;
        return 0;
    }

    int SetIr(double* ir, size_t length) {
        std::unique_ptr<fftconvolver::TwoStageFFTConvolver> temp = std::make_unique<fftconvolver::TwoStageFFTConvolver>();
        if (temp == nullptr) { return -1; }

        mIR.resize(length);
        for (int i = 0; i < length; i++) {
            mIR[i] = static_cast<fftconvolver::Sample>(ir[i]);
        }

        if (!temp->init(kHeadBlockSize, kTailBlockSize, mIR.data(), length)) {
            return -1;
        }

        mConvolver = std::move(temp);

        mCanProcess = true;
        return 0;
    }

    void process(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
        if (mCanProcess) {
            mInput.resize(nFrames);
            for (int i = 0; i < nFrames; i++) {
                mInput[i] = static_cast<fftconvolver::Sample>(inputs[0][i]);
            }

            mOutput.resize(nFrames);
            mConvolver->process(mInput.data(), mOutput.data(), nFrames);

            for (int i = 0; i < nFrames; i++) {
                outputs[0][i] = static_cast<iplug::sample>(mOutput[i]);
            }
            return;
        }
        for (int s = 0; s < nFrames; s++) {
            outputs[0][s] = inputs[0][s];
        }
    }

private:
    static constexpr const size_t kHeadBlockSize = 128;
    static constexpr const size_t kTailBlockSize = 1024;
    std::vector<fftconvolver::Sample> mIR;
    std::vector<fftconvolver::Sample> mInput;
    std::vector<fftconvolver::Sample> mOutput;
    std::unique_ptr<fftconvolver::TwoStageFFTConvolver> mConvolver;
    bool mCanProcess = false;
};

END_IPLUG_NAMESPACE