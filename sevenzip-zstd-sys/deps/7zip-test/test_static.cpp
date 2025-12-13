/**
 * 7-Zip 静态库测试程序
 *
 * 测试内容：
 * 1. CRC 表初始化
 * 2. Codec 注册检查
 * 3. Archive Handler 注册检查
 * 4. LZMA 压缩/解压功能测试
 * 5. 7z Handler 创建测试
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>

// Windows 头文件
#ifdef _WIN32
#include <windows.h>
#endif

// 7-Zip C 接口
extern "C"
{
#include "7zTypes.h"
#include "7zCrc.h"
#include "LzmaEnc.h"
#include "LzmaDec.h"
#include "Alloc.h"

    // 全局计数器 - 通过 extern 声明（在 CodecExports.cpp 中定义）
    extern unsigned g_NumCodecs;
    
    // 导出函数获取 Archive 数量
    HRESULT GetNumberOfFormats(UInt32 *numFormats);
    HRESULT GetNumberOfMethods(UInt32 *numMethods);
}

// 7-Zip CPP 接口
#include "Common/MyCom.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/ICoder.h"
#include "7zip/Archive/7z/7zHandler.h"

// ============================================================================
// 辅助类
// ============================================================================

// LZMA 内存分配器
static void *LzmaAlloc(ISzAllocPtr p, size_t size)
{
    (void)p;
    return MyAlloc(size);
}

static void LzmaFree(ISzAllocPtr p, void *address)
{
    (void)p;
    MyFree(address);
}

static ISzAlloc g_LzmaAlloc = {LzmaAlloc, LzmaFree};

// ============================================================================
// 测试函数
// ============================================================================

bool Test_CrcInit()
{
    std::cout << "[TEST] CRC Table Initialization... ";

    try
    {
        CrcGenerateTable();

        // 验证 CRC 计算
        const char *testData = "Hello, 7-Zip!";
        UInt32 crc = CrcCalc(testData, strlen(testData));

        if (crc != 0)
        {
            std::cout << "OK (CRC = 0x" << std::hex << crc << std::dec << ")" << std::endl;
            return true;
        }
        else
        {
            std::cout << "FAILED (CRC is zero)" << std::endl;
            return false;
        }
    }
    catch (...)
    {
        std::cout << "FAILED (exception)" << std::endl;
        return false;
    }
}

bool Test_CodecRegistration()
{
    std::cout << "[TEST] Codec Registration... ";
    
    UInt32 numMethods = 0;
    HRESULT hr = GetNumberOfMethods(&numMethods);
    
    if (FAILED(hr))
    {
        std::cout << "FAILED (GetNumberOfMethods error: 0x" << std::hex << hr << std::dec << ")" << std::endl;
        return false;
    }
    
    std::cout << "numMethods = " << numMethods;

    if (numMethods > 0)
    {
        std::cout << " OK" << std::endl;
        return true;
    }
    else
    {
        std::cout << " FAILED (no codecs registered!)" << std::endl;
        return false;
    }
}

bool Test_ArchiveRegistration()
{
    std::cout << "[TEST] Archive Handler Registration... ";
    
    UInt32 numFormats = 0;
    HRESULT hr = GetNumberOfFormats(&numFormats);
    
    if (FAILED(hr))
    {
        std::cout << "FAILED (GetNumberOfFormats error: 0x" << std::hex << hr << std::dec << ")" << std::endl;
        return false;
    }
    
    std::cout << "numFormats = " << numFormats;

    if (numFormats > 0)
    {
        std::cout << " OK" << std::endl;
        return true;
    }
    else
    {
        std::cout << " FAILED (no archive handlers registered!)" << std::endl;
        return false;
    }
}

bool Test_LzmaCompress()
{
    std::cout << "[TEST] LZMA Compression... ";

    try
    {
        // 测试数据
        const char *testStr = "Hello, 7-Zip! This is a test string for LZMA compression. "
                              "We need some repetitive data to test compression efficiency. "
                              "LZMA is a very effective compression algorithm!";
        size_t srcLen = strlen(testStr);

        // 准备压缩缓冲区
        size_t propsSize = LZMA_PROPS_SIZE;
        size_t destLen = srcLen + srcLen / 3 + 128; // 留足够空间
        std::vector<Byte> dest(destLen);
        std::vector<Byte> props(LZMA_PROPS_SIZE);

        // LZMA 属性
        CLzmaEncProps encProps;
        LzmaEncProps_Init(&encProps);
        encProps.level = 5;     // 压缩级别
        encProps.dictSize = 1 << 16; // 64KB 字典

        // 压缩
        SRes res = LzmaEncode(
            dest.data(), &destLen,
            (const Byte *)testStr, srcLen,
            &encProps, props.data(), &propsSize,
            0, nullptr, &g_LzmaAlloc, &g_LzmaAlloc);

        if (res != SZ_OK)
        {
            std::cout << "FAILED (compress error: " << res << ")" << std::endl;
            return false;
        }

        std::cout << "compressed " << srcLen << " -> " << destLen << " bytes";

        // 解压缩测试
        std::vector<Byte> decompressed(srcLen + 16);
        size_t decompLen = srcLen;
        size_t compLen = destLen;
        ELzmaStatus status;

        res = LzmaDecode(
            decompressed.data(), &decompLen,
            dest.data(), &compLen,
            props.data(), (unsigned)propsSize,
            LZMA_FINISH_END, &status, &g_LzmaAlloc);

        if (res != SZ_OK)
        {
            std::cout << " FAILED (decompress error: " << res << ")" << std::endl;
            return false;
        }

        // 验证数据
        if (decompLen != srcLen || memcmp(testStr, decompressed.data(), srcLen) != 0)
        {
            std::cout << " FAILED (data mismatch)" << std::endl;
            return false;
        }

        std::cout << " -> decompressed OK" << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "FAILED (exception: " << e.what() << ")" << std::endl;
        return false;
    }
    catch (...)
    {
        std::cout << "FAILED (unknown exception)" << std::endl;
        return false;
    }
}

bool Test_Create7zHandler()
{
    std::cout << "[TEST] Create 7z Handler... ";

    try
    {
        // 直接创建 7z Handler 实例（不需要 DLL）
        NArchive::N7z::CHandler *handler = new NArchive::N7z::CHandler();

        // 转换为 IInArchive 接口
        CMyComPtr<IInArchive> archive = handler;

        if (archive)
        {
            std::cout << "OK" << std::endl;

            // 测试获取属性数量
            UInt32 numProps = 0;
            HRESULT hr = archive->GetNumberOfArchiveProperties(&numProps);
            if (SUCCEEDED(hr))
            {
                std::cout << "       Archive properties count: " << numProps << std::endl;
            }

            return true;
        }
        else
        {
            std::cout << "FAILED (handler is null)" << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "FAILED (exception: " << e.what() << ")" << std::endl;
        return false;
    }
    catch (...)
    {
        std::cout << "FAILED (unknown exception)" << std::endl;
        return false;
    }
}

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char *argv[])
{
    std::cout << "================================================" << std::endl;
    std::cout << "  7-Zip Static Library Test" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;

    int passed = 0;
    int failed = 0;

    // 测试 1: CRC 初始化
    if (Test_CrcInit())
        passed++;
    else
        failed++;

    // 测试 2: Codec 注册
    if (Test_CodecRegistration())
        passed++;
    else
        failed++;

    // 测试 3: Archive Handler 注册
    if (Test_ArchiveRegistration())
        passed++;
    else
        failed++;

    // 测试 4: LZMA 压缩/解压
    if (Test_LzmaCompress())
        passed++;
    else
        failed++;

    // 测试 5: 创建 7z Handler
    if (Test_Create7zHandler())
        passed++;
    else
        failed++;

    // 结果汇总
    std::cout << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "  Results: " << passed << " passed, " << failed << " failed" << std::endl;
    std::cout << "================================================" << std::endl;

    if (failed > 0)
    {
        std::cout << std::endl;
        std::cout << "!! STATIC LIBRARY HAS ISSUES !!" << std::endl;
        std::cout << std::endl;
        std::cout << "If g_NumCodecs or g_NumArcs is 0, try:" << std::endl;
        std::cout << "  1. Use --whole-archive linker option (GCC)" << std::endl;
        std::cout << "  2. Use /WHOLEARCHIVE linker option (MSVC)" << std::endl;
        return 1;
    }

    std::cout << std::endl;
    std::cout << "Static library is working correctly!" << std::endl;
    return 0;
}

