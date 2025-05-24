#include "NeZcab.h"
#include "IPlug_include_in_plug_src.h"
#include <string>

NeZcab::NeZcab(const InstanceInfo& info)
    : iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets)),
    irBuffer(GetSampleRate())
{
    GetParam(kParamGain)->InitDouble("Gain", 100., 0., 120.0, 0.01, "%");
    GetParam(kParamResample)->InitEnum("ResampleType", 1, 4, "", 0, "", "WDL Resampler", "Custom Resampler", "Linear Resampler", "R8Brain Resampler");


#if IPLUG_EDITOR // http://bit.ly/2S64BDd
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
        };

    mLayoutFunc = [&](IGraphics* pGraphics) {
        pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
        pGraphics->AttachPanelBackground(COLOR_GRAY);
        pGraphics->EnableMouseOver(true);
        pGraphics->EnableMultiTouch(true);

#ifdef OS_WEB
        pGraphics->AttachPopupMenuControl();
#endif

        //    pGraphics->EnableLiveEdit(true);
        pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
        const IRECT b = pGraphics->GetBounds().GetPadded(-20.f);
        const IRECT controls = b.GetGridCell(1, 2, 2);
        pGraphics->AttachControl(new IVLEDMeterControl<2>(controls.GetFromRight(10).GetPadded(10)), kCtrlTagMeter);

        auto loadHandler = [&](IControl* pControl) {
            WDL_String filePath;
            WDL_String dirPath;
            IRECT bounds = pControl->GetRECT();
            GetUI()->PromptForFile(filePath, dirPath, EFileAction::Open, "wav");

            irBuffer.LoadIr(filePath, dirPath);
        };
        pGraphics->AttachControl(new IVButtonControl(IRECT(0, 0, 75, 25), loadHandler, "Load"));

        pGraphics->AttachControl(new ICaptionControl(IRECT(0, 30, 150, 55), kParamResample, IText(16.f), DEFAULT_FGCOLOR, false));
        };
#endif
}

#if IPLUG_DSP
void NeZcab::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
    const double gain = GetParam(kParamGain)->Value() / 100.;
    //const int nChansIn = NInChansConnected();
    const int nChans = NOutChansConnected();

    convolutionDsp[0].process(inputs, outputs, nFrames);
    sample** inR = inputs + 1;
    sample** outR = outputs + 1;
    convolutionDsp[1].process(inR, outR, nFrames);

    mMeterSender.ProcessBlock(outputs, nFrames, kCtrlTagMeter);
}

void NeZcab::OnIdle() {
    mMeterSender.TransmitData(*this);
    
    if (irBuffer.mIsStaged) {
#if defined(USE_WDL_CONVOLVER)
        convolutionDsp[0].SetIr(irBuffer.Get(), irBuffer.GetSize(), GetSampleRate());
        convolutionDsp[1].SetIr(irBuffer.Get(), irBuffer.GetSize(), GetSampleRate());
#else
        convolutionDsp[0].SetIr(irBuffer.Get(), irBuffer.GetSize());
        convolutionDsp[1].SetIr(irBuffer.Get(), irBuffer.GetSize());
#endif
        irBuffer.mIsStaged = false;
    }
}

void NeZcab::OnReset() {
    irBuffer.OnReset(GetSampleRate());
    
    if (irBuffer.mIsStaged) {
#if defined(USE_WDL_CONVOLVER)
        convolutionDsp[0].SetIr(irBuffer.Get(), irBuffer.GetSize(), GetSampleRate());
        convolutionDsp[1].SetIr(irBuffer.Get(), irBuffer.GetSize(), GetSampleRate());
#else
        convolutionDsp[0].SetIr(irBuffer.Get(), irBuffer.GetSize());
        convolutionDsp[1].SetIr(irBuffer.Get(), irBuffer.GetSize());
#endif
        irBuffer.mIsStaged = false;
    }

#if defined(USE_WDL_CONVOLVER)
    convolutionDsp[0].OnReset(GetSampleRate());
    convolutionDsp[1].OnReset(GetSampleRate());
    SetLatency(convolutionDsp->GetLatency());
#else
    convolutionDsp[0].OnReset();
    convolutionDsp[1].OnReset();
#endif

    mMeterSender.Reset(GetSampleRate());
}

void NeZcab::OnParamChange(int paramIdx) {
    if (paramIdx == kParamResample) {
        irBuffer.SetResampler((IrBuffer::ResamplerType)GetParam(kParamResample)->Value());
    }
}

void NeZcab::OnParamChangeUI(int paramIdx, EParamSource source) {
#if IPLUG_EDITOR
    if (auto pGraphics = GetUI()) {
    }
#endif
}

bool NeZcab::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) {
    return false;
}
#endif
