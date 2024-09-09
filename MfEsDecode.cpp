#include <iostream>
#include <string>
#include <format>
#include <sstream>

#include <unknwn.h>
#include <winrt\base.h>

#pragma comment(lib, "windowsapp.lib")
#pragma comment(lib, "shlwapi.lib")

#include <propvarutil.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")

int main()
{
    winrt::init_apartment();
    winrt::check_hresult(MFStartup(MF_VERSION));

    winrt::com_ptr<IMFTransform> Transform;
    winrt::check_hresult(CoCreateInstance(CLSID_MSH264DecoderMFT, nullptr, CLSCTX_ALL, IID_PPV_ARGS(Transform.put())));
    winrt::com_ptr<IMFMediaType> InputMediaType;
    winrt::check_hresult(MFCreateMediaType(InputMediaType.put()));
    winrt::check_hresult(InputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
    winrt::check_hresult(InputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264_ES));
    winrt::check_hresult(Transform->SetInputType(0, InputMediaType.get(), 0));

    winrt::com_ptr<IMFMediaType> OutputMediaType;
    winrt::check_hresult(Transform->GetOutputAvailableType(0, 0, OutputMediaType.put()));
    winrt::check_hresult(Transform->SetOutputType(0, OutputMediaType.get(), 0));

    winrt::file_handle File { CreateFileW(L"test.mp4.h264", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) };
    LARGE_INTEGER FileSize;
    winrt::check_bool(GetFileSizeEx(File.get(), &FileSize));

    winrt::com_ptr<IMFMediaBuffer> InputMediaBuffer;
    winrt::check_hresult(MFCreateMemoryBuffer(static_cast<DWORD>(FileSize.QuadPart), InputMediaBuffer.put()));
    UINT8* Data;
    DWORD DataCapacity, DataSize;
    winrt::check_hresult(InputMediaBuffer->Lock(&Data, &DataCapacity, &DataSize));
    DWORD ReadDataSize;
    winrt::check_bool(ReadFile(File.get(), Data, DataCapacity, &ReadDataSize, nullptr));
    WINRT_ASSERT(ReadDataSize == DataCapacity);
    winrt::check_hresult(InputMediaBuffer->Unlock());
    winrt::check_hresult(InputMediaBuffer->SetCurrentLength(ReadDataSize));

    winrt::com_ptr<IMFSample> InputSample;
    winrt::check_hresult(MFCreateSample(InputSample.put()));
    winrt::check_hresult(InputSample->AddBuffer(InputMediaBuffer.get()));
    winrt::check_hresult(InputSample->SetSampleTime(0));
    winrt::check_hresult(InputSample->SetSampleDuration(1));
    winrt::check_hresult(Transform->ProcessInput(0, InputSample.get(), 0));

    winrt::check_hresult(Transform->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN, 0));

    UINT32 SampleSize = 0;
    for(; ; )
    {
        winrt::com_ptr<IMFSample> OutputSample;
        winrt::check_hresult(MFCreateSample(OutputSample.put()));
        if(SampleSize)
        {
            winrt::com_ptr<IMFMediaBuffer> OutputMediaBuffer;
            winrt::check_hresult(MFCreateMemoryBuffer(static_cast<DWORD>(SampleSize), OutputMediaBuffer.put()));
            winrt::check_hresult(OutputSample->AddBuffer(OutputMediaBuffer.get()));
        }
        MFT_OUTPUT_DATA_BUFFER OutputDataBuffer { 0, OutputSample.get() };
        DWORD Status;
        const HRESULT ProcessOutputResult = Transform->ProcessOutput(0, 1, &OutputDataBuffer, &Status);
        if(ProcessOutputResult == MF_E_TRANSFORM_STREAM_CHANGE)
        {
            OutputMediaType = nullptr;
            winrt::check_hresult(Transform->GetOutputAvailableType(0, 0, OutputMediaType.put()));
            winrt::check_hresult(OutputMediaType->GetUINT32(MF_MT_SAMPLE_SIZE, &SampleSize));
            winrt::check_hresult(Transform->SetOutputType(0, OutputMediaType.get(), 0));
            std::cout << std::format("MediaType, SampleSize {:d}", SampleSize) << std::endl;
            continue;
        }
        if(FAILED(ProcessOutputResult))
            break;
        std::ostringstream Stream;
        LONGLONG SampleTime;
        winrt::check_hresult(OutputSample->GetSampleTime(&SampleTime));
        Stream << std::format("Sample, SampleTime {:.3f}", SampleTime / 1E7);
        UINT32 FrameCorruption;
        if(SUCCEEDED(OutputSample->GetUINT32(MFSampleExtension_FrameCorruption, &FrameCorruption)))
            Stream << std::format(", FrameCorruption {}", FrameCorruption);
        std::cout << Stream.str() << std::endl;
    }

    // TODO: Clean things up
    return 0;
}
