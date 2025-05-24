#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"

#include "IrBuffer.h"

#define USE_HISSTOOLS_CONVOLVER

#if defined(USE_HISSTOOLS_CONVOLVER)
    #include "HISSToolsConvolver.h"
#elif defined(TWOSTAGE_CONVOLVER)
    #include "TwoStageConvolver.h"
#elif defined(USE_WDL_CONVOLVER)
    #include "WDL_convolver.h"
#endif


const int kNumPresets = 1;

enum EParams {
    kParamGain = 0,
    kParamResample,
    kNumParams
};

enum EControlTags {
    kCtrlTagMeter = 0,
    kNumCtrlTags
};

using namespace iplug;
using namespace igraphics;

class NeZcab final : public Plugin {
public:
    NeZcab(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
public:
    void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
    void OnReset() override;
    void OnParamChange(int paramIdx) override;
    void OnParamChangeUI(int paramIdx, EParamSource source) override;
    void OnIdle() override;
    bool OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) override;

private:
    IPeakAvgSender<2> mMeterSender;
#endif

private:
    
    IrBuffer irBuffer;

    #if defined(USE_HISSTOOLS_CONVOLVER)
        HISSToolsConvolver convolutionDsp[2] = { HISSToolsConvolver() , HISSToolsConvolver() };
    #elif defined(TWOSTAGE_CONVOLVER)
        TwoStageConvolver convolutionDsp[2] = { TwoStageConvolver() , TwoStageConvolver() };
    #elif defined(USE_WDL_CONVOLVER)
        WdlConvolver convolutionDsp[2] = { WdlConvolver() , WdlConvolver() };
    #endif
    
};
