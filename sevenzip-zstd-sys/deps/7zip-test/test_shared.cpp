/**
 * 7-Zip 动态库 (DLL) 测试程序
 *
 * 使用 LoadLibrary 动态加载 7z.dll 并测试其导出函数：
 * 1. CreateObject - 创建对象
 * 2. GetNumberOfFormats - 获取支持的格式数量
 * 3. GetHandlerProperty2 - 获取格式属性
 * 4. GetNumberOfMethods - 获取支持的压缩方法数量
 */

#include <iostream>
#include <cstdio>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

// 7-Zip 接口
#include "Common/MyCom.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/ICoder.h"
#include "7zip/IPassword.h"

// ============================================================================
// 7z DLL 导出函数类型定义
// ============================================================================

typedef HRESULT(WINAPI *Func_CreateObject)(const GUID *clsID, const GUID *iid, void **outObject);
typedef HRESULT(WINAPI *Func_GetNumberOfFormats)(UInt32 *numFormats);
typedef HRESULT(WINAPI *Func_GetHandlerProperty2)(UInt32 formatIndex, PROPID propID, PROPVARIANT *value);
typedef HRESULT(WINAPI *Func_GetNumberOfMethods)(UInt32 *numMethods);
typedef HRESULT(WINAPI *Func_GetMethodProperty)(UInt32 codecIndex, PROPID propID, PROPVARIANT *value);

// ============================================================================
// 全局变量
// ============================================================================

static HMODULE g_hModule = nullptr;
static Func_CreateObject g_CreateObject = nullptr;
static Func_GetNumberOfFormats g_GetNumberOfFormats = nullptr;
static Func_GetHandlerProperty2 g_GetHandlerProperty2 = nullptr;
static Func_GetNumberOfMethods g_GetNumberOfMethods = nullptr;
static Func_GetMethodProperty g_GetMethodProperty = nullptr;

// ============================================================================
// 辅助函数
// ============================================================================

std::wstring GetDllPath(int argc, char *argv[])
{
    if (argc > 1)
    {
        // 从命令行获取
        wchar_t widePath[MAX_PATH];
        MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, widePath, MAX_PATH);
        return widePath;
    }
    else
    {
        // 默认路径
        return L"7z.dll";
    }
}

bool LoadDll(const std::wstring &dllPath)
{
    std::wcout << L"[INFO] Loading DLL: " << dllPath << std::endl;

    g_hModule = LoadLibraryW(dllPath.c_str());
    if (!g_hModule)
    {
        DWORD error = GetLastError();
        std::cerr << "[ERROR] Failed to load DLL, error code: " << error << std::endl;
        return false;
    }

    // 获取导出函数
    g_CreateObject = (Func_CreateObject)GetProcAddress(g_hModule, "CreateObject");
    g_GetNumberOfFormats = (Func_GetNumberOfFormats)GetProcAddress(g_hModule, "GetNumberOfFormats");
    g_GetHandlerProperty2 = (Func_GetHandlerProperty2)GetProcAddress(g_hModule, "GetHandlerProperty2");
    g_GetNumberOfMethods = (Func_GetNumberOfMethods)GetProcAddress(g_hModule, "GetNumberOfMethods");
    g_GetMethodProperty = (Func_GetMethodProperty)GetProcAddress(g_hModule, "GetMethodProperty");

    return true;
}

void UnloadDll()
{
    if (g_hModule)
    {
        FreeLibrary(g_hModule);
        g_hModule = nullptr;
    }
}

std::wstring PropVariantToString(const PROPVARIANT &prop)
{
    switch (prop.vt)
    {
    case VT_BSTR:
        return prop.bstrVal ? prop.bstrVal : L"(null)";
    case VT_UI4:
        return std::to_wstring(prop.ulVal);
    case VT_UI8:
        return std::to_wstring(prop.uhVal.QuadPart);
    case VT_BOOL:
        return prop.boolVal ? L"true" : L"false";
    case VT_EMPTY:
        return L"(empty)";
    default:
        return L"(type=" + std::to_wstring(prop.vt) + L")";
    }
}

// ============================================================================
// 测试函数
// ============================================================================

bool Test_DllLoad(const std::wstring &dllPath)
{
    std::cout << "[TEST] Load 7z.dll... ";

    if (!LoadDll(dllPath))
    {
        std::cout << "FAILED" << std::endl;
        return false;
    }

    std::cout << "OK" << std::endl;
    return true;
}

bool Test_ExportFunctions()
{
    std::cout << "[TEST] Export Functions... ";

    bool hasCreateObject = (g_CreateObject != nullptr);
    bool hasGetNumberOfFormats = (g_GetNumberOfFormats != nullptr);
    bool hasGetHandlerProperty2 = (g_GetHandlerProperty2 != nullptr);
    bool hasGetNumberOfMethods = (g_GetNumberOfMethods != nullptr);

    std::cout << std::endl;
    std::cout << "       CreateObject:        " << (hasCreateObject ? "OK" : "MISSING") << std::endl;
    std::cout << "       GetNumberOfFormats:  " << (hasGetNumberOfFormats ? "OK" : "MISSING") << std::endl;
    std::cout << "       GetHandlerProperty2: " << (hasGetHandlerProperty2 ? "OK" : "MISSING") << std::endl;
    std::cout << "       GetNumberOfMethods:  " << (hasGetNumberOfMethods ? "OK" : "MISSING") << std::endl;

    // 至少需要 CreateObject 函数
    if (hasCreateObject)
    {
        return true;
    }

    return false;
}

bool Test_GetFormats()
{
    std::cout << "[TEST] Get Supported Formats... ";

    if (!g_GetNumberOfFormats || !g_GetHandlerProperty2)
    {
        std::cout << "SKIPPED (functions not available)" << std::endl;
        return true;
    }

    UInt32 numFormats = 0;
    HRESULT hr = g_GetNumberOfFormats(&numFormats);
    if (FAILED(hr))
    {
        std::cout << "FAILED (GetNumberOfFormats error: " << std::hex << hr << ")" << std::endl;
        return false;
    }

    std::cout << numFormats << " formats found:" << std::endl;

    // 列出前几个格式
    int maxShow = (numFormats > 10) ? 10 : numFormats;
    for (UInt32 i = 0; i < (UInt32)maxShow; i++)
    {
        PROPVARIANT prop;
        PropVariantInit(&prop);

        // NArchive::NHandlerPropID::kName = 0
        hr = g_GetHandlerProperty2(i, 0, &prop);
        if (SUCCEEDED(hr))
        {
            std::wcout << L"       [" << i << L"] " << PropVariantToString(prop) << std::endl;
        }
        PropVariantClear(&prop);
    }

    if (numFormats > 10)
    {
        std::cout << "       ... and " << (numFormats - 10) << " more" << std::endl;
    }

    return numFormats > 0;
}

bool Test_GetMethods()
{
    std::cout << "[TEST] Get Compression Methods... ";

    if (!g_GetNumberOfMethods || !g_GetMethodProperty)
    {
        std::cout << "SKIPPED (functions not available)" << std::endl;
        return true;
    }

    UInt32 numMethods = 0;
    HRESULT hr = g_GetNumberOfMethods(&numMethods);
    if (FAILED(hr))
    {
        std::cout << "FAILED (GetNumberOfMethods error: " << std::hex << hr << ")" << std::endl;
        return false;
    }

    std::cout << numMethods << " methods found:" << std::endl;

    // 列出前几个方法
    int maxShow = (numMethods > 10) ? 10 : numMethods;
    for (UInt32 i = 0; i < (UInt32)maxShow; i++)
    {
        PROPVARIANT prop;
        PropVariantInit(&prop);

        // NMethodPropID::kName = 1
        hr = g_GetMethodProperty(i, 1, &prop);
        if (SUCCEEDED(hr))
        {
            std::wcout << L"       [" << i << L"] " << PropVariantToString(prop) << std::endl;
        }
        PropVariantClear(&prop);
    }

    if (numMethods > 10)
    {
        std::cout << "       ... and " << (numMethods - 10) << " more" << std::endl;
    }

    return numMethods > 0;
}

bool Test_CreateInArchive()
{
    std::cout << "[TEST] Create IInArchive (7z format)... ";

    if (!g_CreateObject)
    {
        std::cout << "SKIPPED (CreateObject not available)" << std::endl;
        return true;
    }

    // 7z CLSID: {23170F69-40C1-278A-1000-000110070000}
    // 格式索引 7 对应 7z
    GUID CLSID_CFormat7z = {0x23170F69, 0x40C1, 0x278A, {0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00}};
    GUID IID_IInArchive = {0x23170F69, 0x40C1, 0x278A, {0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00}};

    void *archive = nullptr;
    HRESULT hr = g_CreateObject(&CLSID_CFormat7z, &IID_IInArchive, &archive);

    if (FAILED(hr))
    {
        std::cout << "FAILED (CreateObject error: 0x" << std::hex << hr << std::dec << ")" << std::endl;
        return false;
    }

    if (archive)
    {
        std::cout << "OK" << std::endl;

        // 释放对象
        IInArchive *pArchive = static_cast<IInArchive *>(archive);
        pArchive->Release();
        return true;
    }

    std::cout << "FAILED (archive is null)" << std::endl;
    return false;
}

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char *argv[])
{
    std::cout << "================================================" << std::endl;
    std::cout << "  7-Zip Dynamic Library (DLL) Test" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;

    // 获取 DLL 路径
    std::wstring dllPath = GetDllPath(argc, argv);

    int passed = 0;
    int failed = 0;

    // 测试 1: 加载 DLL
    if (!Test_DllLoad(dllPath))
    {
        std::cout << std::endl;
        std::cout << "!! Cannot load DLL, aborting tests !!" << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: " << argv[0] << " [path/to/7z.dll]" << std::endl;
        return 1;
    }
    passed++;

    // 测试 2: 导出函数
    if (Test_ExportFunctions())
        passed++;
    else
        failed++;

    // 测试 3: 获取格式列表
    if (Test_GetFormats())
        passed++;
    else
        failed++;

    // 测试 4: 获取方法列表
    if (Test_GetMethods())
        passed++;
    else
        failed++;

    // 测试 5: 创建 IInArchive
    if (Test_CreateInArchive())
        passed++;
    else
        failed++;

    // 清理
    UnloadDll();

    // 结果汇总
    std::cout << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "  Results: " << passed << " passed, " << failed << " failed" << std::endl;
    std::cout << "================================================" << std::endl;

    if (failed > 0)
    {
        std::cout << std::endl;
        std::cout << "!! DLL HAS ISSUES !!" << std::endl;
        return 1;
    }

    std::cout << std::endl;
    std::cout << "DLL is working correctly!" << std::endl;
    return 0;
}

