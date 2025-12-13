/**
 * 7-Zip 静态库初始化 - 简化版
 *
 * 这个版本不需要修改 7-Zip 源码。
 * 它通过 #include 直接包含所有 *Register.cpp 文件，
 * 确保静态对象被正确初始化。
 *
 * 使用方法：
 * 1. 将此文件添加到你的项目中
 * 2. 在使用 7-Zip 功能前调用 SevenZip_Init()
 */

#include "SevenZipInit.h"

// ============================================================================
// 包含所有必要的头文件
// ============================================================================

// 7-Zip 标准头文件
#include "7zTypes.h"

// C 语言接口
extern "C"
{
    void CrcGenerateTable(void);
    void AesGenTables(void);
}

// ============================================================================
// 直接包含所有 Register.cpp 文件
// 这是确保静态对象被链接的最可靠方法
// ============================================================================

// 注意：需要根据你的 7-Zip 源码路径调整这些 include 路径
// 如果你使用 CMake，可以设置 SEVENZIP_SOURCE_DIR 变量

// Archive Handlers
#include "7zip/Archive/7z/7zRegister.cpp"
#include "7zip/Archive/Zip/ZipRegister.cpp"
#include "7zip/Archive/Tar/TarRegister.cpp"
#include "7zip/Archive/GZip/GZipRegister.cpp"
#include "7zip/Archive/BZip2/BZip2Register.cpp"
#include "7zip/Archive/Xz/XzRegister.cpp"
#include "7zip/Archive/Cab/CabRegister.cpp"
#include "7zip/Archive/Lzma/LzmaArcRegister.cpp"

// Codecs
#include "7zip/Compress/LzmaRegister.cpp"
#include "7zip/Compress/Lzma2Register.cpp"
#include "7zip/Compress/BcjRegister.cpp"
#include "7zip/Compress/Bcj2Register.cpp"
#include "7zip/Compress/CopyRegister.cpp"
#include "7zip/Compress/DeltaRegister.cpp"
#include "7zip/Compress/BZip2Register.cpp"
#include "7zip/Compress/DeflateRegister.cpp"
#include "7zip/Compress/Deflate64Register.cpp"
#include "7zip/Compress/PpmdRegister.cpp"

// Crypto (如果启用)
#ifndef _NO_CRYPTO
#include "7zip/Crypto/7zAesRegister.cpp"
#endif

// ============================================================================
// 全局计数器
// ============================================================================
extern unsigned g_NumCodecs;
extern unsigned g_NumArcs;

// ============================================================================
// 初始化标志
// ============================================================================
static bool g_SevenZipInitialized = false;

// ============================================================================
// 初始化函数
// ============================================================================

extern "C" int SevenZip_StaticInit(void)
{
    if (g_SevenZipInitialized)
    {
        return 0;
    }

    // 初始化 CRC 表
    CrcGenerateTable();

#ifndef _NO_CRYPTO
    // 初始化 AES 表
    AesGenTables();
#endif

    g_SevenZipInitialized = true;

    return 0;
}

extern "C" int SevenZip_GetNumCodecs(void)
{
    return static_cast<int>(g_NumCodecs);
}

extern "C" int SevenZip_GetNumArcs(void)
{
    return static_cast<int>(g_NumArcs);
}