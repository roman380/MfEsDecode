# Decode H.264 ES using Media Foundation

This small project shows how to set up Media Foundation [H.264 Video Decoder](https://docs.microsoft.com/en-us/windows/win32/medfound/h-264-video-decoder) transform to process raw H.264 ES.

The project comes with a sample file derived from [MSDN Forums question](https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/30341030-8fd4-4535-971e-31f1ff487cdf/h264-decoder-problem-artifacts-on-decoded-image?forum=mediafoundationdevelopment#44585441-6846-4c55-b570-16b9a2fe3dcd)

## Instructions

Build the project and run from debugger. The project will pick up [test.mp4.h264](test.mp4.h264) file and process it decoding video frames. The will be `MFSampleExtension_FrameCorruption` information included in the output.

Sample project output is [included](test.mp4.h264.txt).

## Miscellaneous

- See [#1](https://github.com/roman380/MfEsDecode/pull/1) to find out how hardware assisted decoding is different; the branch uses Direct3D 11 device and DXGI device manager with the decoder and obtains decoded frames in Direct3D 11 textures
