# NeZcab

Convolution example/showcase project of different libraries for resampling and convolution.
Clone project recursively next to [iPlug2 library](github.com/olilarkin/iplug2).
```
git clone --recursive https://github.com/nezvers/NeZcab.git
```

Important notes:
I don't have Mac to setup project yet but config should be set with includes and defs.
Add to project `*cpp` files from `IPlug2/WDL` and projects `source` folders.
Defines used ```WDL_RESAMPLE_TYPE=float; WDL_FFT_REALSIZE=4; SAMPLE_TYPE_FLOAT```


Resampling:
[Custom resampling from AlexHarker/OctetViolins](https://github.com/AlexHarker/OctetViolins/blob/main/source/Resampler.h)    
[WDL resampling from Iplug2](https://github.com/olilarkin/iPlug2/blob/master/WDL/resample.h)    
[R8Brain-free](https://github.com/avaneev/r8brain-free-src)    
[Linear from IPlug2 example](https://github.com/olilarkin/iPlug2/blob/master/Examples/IPlugConvoEngine/IPlugConvoEngine.h)    

Convolver:    
[HISSTools_Library Convolver](https://github.com/AlexHarker/HISSTools_Library/tree/main/HIRT_Multichannel_Convolution)    
[TwoStageFFTConvolver](https://github.com/falkTX/FFTConvolver)    
[ WIP ] [WDL Convoengine from IPlug2](https://github.com/olilarkin/iPlug2/blob/master/WDL/convoengine.h)    
