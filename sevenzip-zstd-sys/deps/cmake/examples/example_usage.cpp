/**
 * 7-Zip 静态库使用示例
 *
 * 演示如何使用静态链接的 7-Zip 库进行压缩和解压
 */

#include <iostream>
#include <string>

// 7-Zip 头文件
#include "Common/MyCom.h"
#include "Common/MyString.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "Windows/FileIO.h"
#include "Windows/FileFind.h"
#include "Windows/PropVariant.h"

// 直接使用 7z Handler
#include "7zip/Archive/7z/7zHandler.h"

// 静态库初始化
#include "SevenZipInit.h"

// ============================================================================
// 文件流实现
// ============================================================================
class CInFileStream : public IInStream,
                      public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(IInStream)

    NWindows::NFile::NIO::CInFile File;

    bool Open(const wchar_t *fileName)
    {
        return File.Open(fileName);
    }

    STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize)
    {
        UInt32 realProcessedSize;
        bool result = File.Read(data, size, realProcessedSize);
        if (processedSize)
            *processedSize = realProcessedSize;
        return result ? S_OK : E_FAIL;
    }

    STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
    {
        UInt64 realNewPosition;
        bool result = File.Seek(offset, seekOrigin, realNewPosition);
        if (newPosition)
            *newPosition = realNewPosition;
        return result ? S_OK : E_FAIL;
    }
};

class COutFileStream : public IOutStream,
                       public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(IOutStream)

    NWindows::NFile::NIO::COutFile File;

    bool Create(const wchar_t *fileName, bool createAlways)
    {
        return File.Create(fileName, createAlways);
    }

    STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize)
    {
        UInt32 realProcessedSize;
        bool result = File.Write(data, size, realProcessedSize);
        if (processedSize)
            *processedSize = realProcessedSize;
        return result ? S_OK : E_FAIL;
    }

    STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
    {
        UInt64 realNewPosition;
        bool result = File.Seek(offset, seekOrigin, realNewPosition);
        if (newPosition)
            *newPosition = realNewPosition;
        return result ? S_OK : E_FAIL;
    }

    STDMETHOD(SetSize)(UInt64 newSize)
    {
        return File.SetLength(newSize) ? S_OK : E_FAIL;
    }
};

// ============================================================================
// 解压回调实现
// ============================================================================
class CArchiveExtractCallback : public IArchiveExtractCallback,
                                public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(IArchiveExtractCallback)

    CMyComPtr<IInArchive> Archive;
    UString OutputPath;
    UInt32 CurrentIndex;
    COutFileStream *OutFileStreamSpec;
    CMyComPtr<ISequentialOutStream> OutFileStream;

    void Init(IInArchive *archive, const UString &outputPath)
    {
        Archive = archive;
        OutputPath = outputPath;
    }

    STDMETHOD(SetTotal)(UInt64 /* total */) { return S_OK; }
    STDMETHOD(SetCompleted)(const UInt64 * /* completeValue */) { return S_OK; }

    STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode)
    {
        *outStream = nullptr;
        CurrentIndex = index;

        if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
            return S_OK;

        // 获取文件名
        NWindows::NCOM::CPropVariant prop;
        Archive->GetProperty(index, kpidPath, &prop);

        UString fileName;
        if (prop.vt == VT_BSTR)
            fileName = prop.bstrVal;
        else
            fileName = L"unknown";

        // 创建输出文件
        UString fullPath = OutputPath + L"\\" + fileName;

        OutFileStreamSpec = new COutFileStream;
        CMyComPtr<ISequentialOutStream> outStreamLoc(OutFileStreamSpec);

        if (!OutFileStreamSpec->Create(fullPath, true))
        {
            std::wcerr << L"Cannot create file: " << (const wchar_t *)fullPath << std::endl;
            return E_FAIL;
        }

        OutFileStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
        return S_OK;
    }

    STDMETHOD(PrepareOperation)(Int32 /* askExtractMode */) { return S_OK; }

    STDMETHOD(SetOperationResult)(Int32 /* operationResult */)
    {
        OutFileStream.Release();
        return S_OK;
    }
};

// ============================================================================
// 示例函数
// ============================================================================

/**
 * 列出压缩包内容
 */
bool ListArchive(const wchar_t *archivePath)
{
    std::wcout << L"Listing archive: " << archivePath << std::endl;

    // 创建文件输入流
    CInFileStream *fileStreamSpec = new CInFileStream;
    CMyComPtr<IInStream> fileStream = fileStreamSpec;

    if (!fileStreamSpec->Open(archivePath))
    {
        std::wcerr << L"Cannot open archive file" << std::endl;
        return false;
    }

    // 创建 7z Handler（静态链接，不需要 DLL）
    NArchive::N7z::CHandler *handler = new NArchive::N7z::CHandler;
    CMyComPtr<IInArchive> archive = handler;

    // 打开压缩包
    HRESULT hr = archive->Open(fileStream, nullptr, nullptr);
    if (hr != S_OK)
    {
        std::wcerr << L"Cannot open archive. Error code: " << hr << std::endl;
        return false;
    }

    // 获取文件数量
    UInt32 numItems;
    archive->GetNumberOfItems(&numItems);
    std::wcout << L"Number of items: " << numItems << std::endl;

    // 列出所有文件
    for (UInt32 i = 0; i < numItems; i++)
    {
        NWindows::NCOM::CPropVariant prop;

        // 文件名
        archive->GetProperty(i, kpidPath, &prop);
        std::wcout << L"  [" << i << L"] ";
        if (prop.vt == VT_BSTR)
            std::wcout << prop.bstrVal;

        // 文件大小
        archive->GetProperty(i, kpidSize, &prop);
        if (prop.vt == VT_UI8)
            std::wcout << L" (" << prop.uhVal.QuadPart << L" bytes)";

        std::wcout << std::endl;
    }

    archive->Close();
    return true;
}

/**
 * 解压压缩包
 */
bool ExtractArchive(const wchar_t *archivePath, const wchar_t *outputPath)
{
    std::wcout << L"Extracting archive: " << archivePath << std::endl;
    std::wcout << L"Output path: " << outputPath << std::endl;

    // 创建文件输入流
    CInFileStream *fileStreamSpec = new CInFileStream;
    CMyComPtr<IInStream> fileStream = fileStreamSpec;

    if (!fileStreamSpec->Open(archivePath))
    {
        std::wcerr << L"Cannot open archive file" << std::endl;
        return false;
    }

    // 创建 7z Handler
    NArchive::N7z::CHandler *handler = new NArchive::N7z::CHandler;
    CMyComPtr<IInArchive> archive = handler;

    // 打开压缩包
    HRESULT hr = archive->Open(fileStream, nullptr, nullptr);
    if (hr != S_OK)
    {
        std::wcerr << L"Cannot open archive" << std::endl;
        return false;
    }

    // 创建解压回调
    CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
    CMyComPtr<IArchiveExtractCallback> extractCallback = extractCallbackSpec;
    extractCallbackSpec->Init(archive, outputPath);

    // 解压所有文件
    hr = archive->Extract(nullptr, (UInt32)-1, false, extractCallback);
    if (hr != S_OK)
    {
        std::wcerr << L"Extract failed. Error code: " << hr << std::endl;
        return false;
    }

    archive->Close();
    std::wcout << L"Extraction completed!" << std::endl;
    return true;
}

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    // *** 重要：必须首先调用初始化函数 ***
    SevenZip_StaticInit();

    // 验证初始化成功
    std::cout << "Registered codecs: " << SevenZip_GetNumCodecs() << std::endl;
    std::cout << "Registered archive handlers: " << SevenZip_GetNumArcs() << std::endl;

    if (SevenZip_GetNumCodecs() == 0 || SevenZip_GetNumArcs() == 0)
    {
        std::cerr << "ERROR: Static initialization failed!" << std::endl;
        std::cerr << "Codecs and/or archive handlers were not registered." << std::endl;
        return 1;
    }

    // 示例用法
    if (argc < 2)
    {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " list <archive.7z>" << std::endl;
        std::cout << "  " << argv[0] << " extract <archive.7z> <output_dir>" << std::endl;
        return 0;
    }

    std::string command = argv[1];

    if (command == "list" && argc >= 3)
    {
        // 转换为宽字符
        std::wstring archivePath(argv[2], argv[2] + strlen(argv[2]));
        ListArchive(archivePath.c_str());
    }
    else if (command == "extract" && argc >= 4)
    {
        std::wstring archivePath(argv[2], argv[2] + strlen(argv[2]));
        std::wstring outputPath(argv[3], argv[3] + strlen(argv[3]));
        ExtractArchive(archivePath.c_str(), outputPath.c_str());
    }
    else
    {
        std::cerr << "Invalid command or arguments" << std::endl;
        return 1;
    }

    return 0;
}