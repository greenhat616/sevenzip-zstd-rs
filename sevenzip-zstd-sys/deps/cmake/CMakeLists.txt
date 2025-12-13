cmake_minimum_required(VERSION 3.16)
project(7zip LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# 构建类型选项
# ============================================================================
option(SEVENZIP_BUILD_SHARED "Build shared library (7z.dll)" OFF)
option(SEVENZIP_BUILD_STATIC "Build static library (7z.lib)" ON)

# ============================================================================
# 功能选项
# ============================================================================
option(SEVENZIP_ENABLE_ENCRYPTION "Enable AES encryption support" ON)
option(SEVENZIP_ENABLE_MULTITHREADING "Enable multithreading support" ON)

# 7-Zip-zstd 额外的压缩算法 - 默认全部开启
option(SEVENZIP_ENABLE_ZSTD "Enable Zstandard support" ON)
option(SEVENZIP_ENABLE_LZ4 "Enable LZ4 support" ON)
option(SEVENZIP_ENABLE_LZ5 "Enable LZ5 support" ON)
option(SEVENZIP_ENABLE_LIZARD "Enable Lizard support" ON)
option(SEVENZIP_ENABLE_BROTLI "Enable Brotli support" ON)
option(SEVENZIP_ENABLE_FAST_LZMA2 "Enable Fast LZMA2 support" ON)

# 7-Zip 源码路径
set(SEVENZIP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../7-Zip-zstd" CACHE PATH "Path to 7-Zip source code")

# ============================================================================
# 基础编译定义
# ============================================================================
set(SEVENZIP_COMPILE_DEFINITIONS
    _REENTRANT
    _FILE_OFFSET_BITS=64
    _LARGEFILE_SOURCE
    UNICODE
    _UNICODE
)

if(WIN32)
    list(APPEND SEVENZIP_COMPILE_DEFINITIONS
        _WIN32
        WIN32
        _WINDOWS
    )
endif()

if(NOT SEVENZIP_ENABLE_MULTITHREADING)
    list(APPEND SEVENZIP_COMPILE_DEFINITIONS _7ZIP_ST)
endif()

if(NOT SEVENZIP_ENABLE_ENCRYPTION)
    list(APPEND SEVENZIP_COMPILE_DEFINITIONS _NO_CRYPTO)
endif()

# Zstd 相关定义
if(SEVENZIP_ENABLE_ZSTD)
    list(APPEND SEVENZIP_COMPILE_DEFINITIONS
        ZSTD_LEGACY_SUPPORT
        ZSTD_MULTITHREAD
        # 禁用 ASM 优化（避免链接未定义符号）
        ZSTD_DISABLE_ASM
        # 避免 debug 全局变量冲突
        DEBUGLEVEL=0
    )
endif()

# ============================================================================
# C 源文件 - 核心算法
# ============================================================================
set(SEVENZIP_C_SOURCES
    ${SEVENZIP_SOURCE_DIR}/C/7zBuf2.c
    ${SEVENZIP_SOURCE_DIR}/C/7zStream.c
    ${SEVENZIP_SOURCE_DIR}/C/Alloc.c
    ${SEVENZIP_SOURCE_DIR}/C/Bcj2.c
    ${SEVENZIP_SOURCE_DIR}/C/Bcj2Enc.c
    ${SEVENZIP_SOURCE_DIR}/C/Blake2s.c
    ${SEVENZIP_SOURCE_DIR}/C/Bra.c
    ${SEVENZIP_SOURCE_DIR}/C/Bra86.c
    ${SEVENZIP_SOURCE_DIR}/C/BraIA64.c
    ${SEVENZIP_SOURCE_DIR}/C/BwtSort.c
    ${SEVENZIP_SOURCE_DIR}/C/CpuArch.c
    ${SEVENZIP_SOURCE_DIR}/C/Delta.c
    ${SEVENZIP_SOURCE_DIR}/C/HuffEnc.c
    ${SEVENZIP_SOURCE_DIR}/C/LzFind.c
    ${SEVENZIP_SOURCE_DIR}/C/LzFindMt.c
    ${SEVENZIP_SOURCE_DIR}/C/LzFindOpt.c
    ${SEVENZIP_SOURCE_DIR}/C/Lzma2Dec.c
    ${SEVENZIP_SOURCE_DIR}/C/Lzma2DecMt.c
    ${SEVENZIP_SOURCE_DIR}/C/Lzma2Enc.c
    ${SEVENZIP_SOURCE_DIR}/C/LzmaDec.c
    ${SEVENZIP_SOURCE_DIR}/C/LzmaEnc.c
    ${SEVENZIP_SOURCE_DIR}/C/Md5.c
    ${SEVENZIP_SOURCE_DIR}/C/MtCoder.c
    ${SEVENZIP_SOURCE_DIR}/C/MtDec.c
    ${SEVENZIP_SOURCE_DIR}/C/Ppmd7.c
    ${SEVENZIP_SOURCE_DIR}/C/Ppmd7Dec.c
    ${SEVENZIP_SOURCE_DIR}/C/Ppmd7aDec.c
    ${SEVENZIP_SOURCE_DIR}/C/Ppmd7Enc.c
    ${SEVENZIP_SOURCE_DIR}/C/Ppmd8.c
    ${SEVENZIP_SOURCE_DIR}/C/Ppmd8Dec.c
    ${SEVENZIP_SOURCE_DIR}/C/Ppmd8Enc.c
    ${SEVENZIP_SOURCE_DIR}/C/Sha1.c
    ${SEVENZIP_SOURCE_DIR}/C/Sha1Opt.c
    ${SEVENZIP_SOURCE_DIR}/C/Sha256.c
    ${SEVENZIP_SOURCE_DIR}/C/Sha256Opt.c
    ${SEVENZIP_SOURCE_DIR}/C/Sha3.c
    ${SEVENZIP_SOURCE_DIR}/C/Sha512.c
    ${SEVENZIP_SOURCE_DIR}/C/Sha512Opt.c
    ${SEVENZIP_SOURCE_DIR}/C/Sort.c
    ${SEVENZIP_SOURCE_DIR}/C/SwapBytes.c
    ${SEVENZIP_SOURCE_DIR}/C/Threads.c
    ${SEVENZIP_SOURCE_DIR}/C/Xxh64.c
    ${SEVENZIP_SOURCE_DIR}/C/Xz.c
    ${SEVENZIP_SOURCE_DIR}/C/XzCrc64.c
    ${SEVENZIP_SOURCE_DIR}/C/XzCrc64Opt.c
    ${SEVENZIP_SOURCE_DIR}/C/XzDec.c
    ${SEVENZIP_SOURCE_DIR}/C/XzEnc.c
    ${SEVENZIP_SOURCE_DIR}/C/XzIn.c
    ${SEVENZIP_SOURCE_DIR}/C/7zCrc.c
    ${SEVENZIP_SOURCE_DIR}/C/7zCrcOpt.c
)

# AES 加密
if(SEVENZIP_ENABLE_ENCRYPTION)
    list(APPEND SEVENZIP_C_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/Aes.c
        ${SEVENZIP_SOURCE_DIR}/C/AesOpt.c
    )
endif()

# Hashes
set(SEVENZIP_HASHES_SOURCES
    ${SEVENZIP_SOURCE_DIR}/C/hashes/blake3.c
    ${SEVENZIP_SOURCE_DIR}/C/hashes/md2.c
    ${SEVENZIP_SOURCE_DIR}/C/hashes/md4.c
    ${SEVENZIP_SOURCE_DIR}/C/hashes/md5c.c
    ${SEVENZIP_SOURCE_DIR}/C/hashes/sha3c.c
    ${SEVENZIP_SOURCE_DIR}/C/hashes/sha512c.c
)

# ============================================================================
# Zstd 源文件
# ============================================================================
if(SEVENZIP_ENABLE_ZSTD)
    set(SEVENZIP_ZSTD_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/zstd/debug.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/entropy_common.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/error_private.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/fse_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/fse_decompress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/hist.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/huf_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/huf_decompress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/pool.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/threading.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/xxhash.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_common.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_compress_literals.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_compress_sequences.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_compress_superblock.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_ddict.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_decompress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_decompress_block.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_double_fast.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_fast.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_lazy.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_ldm.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_opt.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_preSplit.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstdmt_compress.c
        # Legacy support
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_v01.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_v02.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_v03.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_v04.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_v05.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_v06.c
        ${SEVENZIP_SOURCE_DIR}/C/zstd/zstd_v07.c
    )
    
    # Zstd 多线程封装
    set(SEVENZIP_ZSTDMT_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/zstd-mt_threading.c
    )
endif()

# ============================================================================
# LZ4 源文件
# ============================================================================
if(SEVENZIP_ENABLE_LZ4)
    set(SEVENZIP_LZ4_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/lz4/lz4.c
        ${SEVENZIP_SOURCE_DIR}/C/lz4/lz4frame.c
        ${SEVENZIP_SOURCE_DIR}/C/lz4/lz4hc.c
    )
    
    list(APPEND SEVENZIP_ZSTDMT_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/lz4-mt_common.c
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/lz4-mt_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/lz4-mt_decompress.c
    )
endif()

# ============================================================================
# LZ5 源文件
# ============================================================================
if(SEVENZIP_ENABLE_LZ5)
    set(SEVENZIP_LZ5_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/lz5/lz5.c
        ${SEVENZIP_SOURCE_DIR}/C/lz5/lz5frame.c
        ${SEVENZIP_SOURCE_DIR}/C/lz5/lz5hc.c
    )
    
    list(APPEND SEVENZIP_ZSTDMT_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/lz5-mt_common.c
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/lz5-mt_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/lz5-mt_decompress.c
    )
endif()

# ============================================================================
# Lizard 源文件
# ============================================================================
if(SEVENZIP_ENABLE_LIZARD)
    set(SEVENZIP_LIZARD_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/lizard/lizard_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/lizard/lizard_decompress.c
        ${SEVENZIP_SOURCE_DIR}/C/lizard/lizard_frame.c
        # 排除 liz_debug.c 避免与 zstd/debug.c 的 g_debuglevel 冲突
        ${SEVENZIP_SOURCE_DIR}/C/lizard/liz_entropy_common.c
        ${SEVENZIP_SOURCE_DIR}/C/lizard/liz_fse_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/lizard/liz_fse_decompress.c
        ${SEVENZIP_SOURCE_DIR}/C/lizard/liz_huf_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/lizard/liz_huf_decompress.c
        ${SEVENZIP_SOURCE_DIR}/C/lizard/liz_xxhash.c
    )
    
    list(APPEND SEVENZIP_ZSTDMT_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/lizard-mt_common.c
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/lizard-mt_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/lizard-mt_decompress.c
    )
endif()

# ============================================================================
# Brotli 源文件
# ============================================================================
if(SEVENZIP_ENABLE_BROTLI)
    set(SEVENZIP_BROTLI_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_backward_references.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_backward_references_hq.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_bit_cost.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_bit_reader.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_block_splitter.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_brotli_bit_stream.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_cluster.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_command.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_compound_dictionary.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_compress_fragment.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_compress_fragment_two_pass.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_constants.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_context.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_decode.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_dictionary.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_dictionary_hash.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_encode.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_encoder_dict.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_entropy_encode.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_fast_log.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_histogram.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_huffman.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_literal_cost.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_memory.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_metablock.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_platform.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_prefix.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_shared_dictionary.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_state.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_static_dict.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_static_dict_lut.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_static_init_dec.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_static_init_enc.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_transform.c
        ${SEVENZIP_SOURCE_DIR}/C/brotli/br_utf8_util.c
    )
    
    list(APPEND SEVENZIP_ZSTDMT_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/brotli-mt_common.c
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/brotli-mt_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt/brotli-mt_decompress.c
    )
endif()

# ============================================================================
# Fast LZMA2 源文件
# ============================================================================
if(SEVENZIP_ENABLE_FAST_LZMA2)
    set(SEVENZIP_FAST_LZMA2_SOURCES
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/dict_buffer.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/fl2_common.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/fl2_compress.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/fl2_pool.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/fl2_threading.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/lzma2_enc.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/radix_bitpack.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/radix_mf.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/radix_struct.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/range_enc.c
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2/util.c
    )
endif()

# ============================================================================
# CPP/Common 源文件
# ============================================================================
set(SEVENZIP_CPP_COMMON_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/CRC.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/CrcReg.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/DynLimBuf.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/IntToString.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/LzFindPrepare.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Md4Reg.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Md5Reg.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/MyMap.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/MyString.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/MyVector.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/MyWindows.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/MyXml.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/NewHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Sha1Prepare.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Sha1Reg.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Sha256Prepare.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Sha256Reg.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Sha3Reg.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Sha512Prepare.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Sha512Reg.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/StringConvert.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/StringToInt.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/UTFConvert.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Wildcard.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/Xxh64Reg.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/XzCrc64Init.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Common/XzCrc64Reg.cpp
)

# ============================================================================
# CPP/Windows 源文件
# ============================================================================
set(SEVENZIP_CPP_WINDOWS_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/FileDir.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/FileFind.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/FileIO.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/FileName.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/PropVariant.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/PropVariantConv.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/PropVariantUtils.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/Synchronization.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/System.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows/TimeUtils.cpp
)

# ============================================================================
# CPP/7zip/Common 源文件
# ============================================================================
set(SEVENZIP_7ZIP_COMMON_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/CreateCoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/CWrappers.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/InBuffer.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/InOutTempBuffer.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/FilterCoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/LimitedStreams.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/LockedStream.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/MemBlocks.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/MethodId.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/MethodProps.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/OffsetStream.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/OutBuffer.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/OutMemStream.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/ProgressMt.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/ProgressUtils.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/PropId.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/StreamBinder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/StreamObjects.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/StreamUtils.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/UniqBlocks.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common/VirtThread.cpp
)

# ============================================================================
# CPP/7zip/Archive 源文件
# ============================================================================
set(SEVENZIP_ARCHIVE_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ApfsHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ApmHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ArHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ArjHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/AvbHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Base64Handler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Bz2Handler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ComHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/CpioHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/CramfsHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/DeflateProps.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/DmgHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ElfHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ExtHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/FatHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/FlvHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/GzHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/GptHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/HandlerCont.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/HfsHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/IhexHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/LpHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/LvmHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/LzHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/LzhHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/LzmaHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/MachoHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/MbrHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/MslzHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/MubHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/NtfsHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/PeHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/PpmdHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/QcowHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/RpmHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/SparseHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/SplitHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/SquashfsHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/SwfHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/UefiHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/VdiHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/VhdHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/VhdxHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/VmdkHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/XarHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/XzHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ZHandler.cpp
)

# Zstd/Brotli/LZ4/LZ5/Lizard Archive Handlers
if(SEVENZIP_ENABLE_ZSTD)
    list(APPEND SEVENZIP_ARCHIVE_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ZstdHandler.cpp
    )
endif()

if(SEVENZIP_ENABLE_BROTLI)
    list(APPEND SEVENZIP_ARCHIVE_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/BrotliHandler.cpp
    )
endif()

if(SEVENZIP_ENABLE_LZ4)
    list(APPEND SEVENZIP_ARCHIVE_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Lz4Handler.cpp
    )
endif()

if(SEVENZIP_ENABLE_LZ5)
    list(APPEND SEVENZIP_ARCHIVE_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Lz5Handler.cpp
    )
endif()

if(SEVENZIP_ENABLE_LIZARD)
    list(APPEND SEVENZIP_ARCHIVE_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/LizardHandler.cpp
    )
endif()

# Archive/Common
set(SEVENZIP_ARCHIVE_COMMON_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/CoderMixer2.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/DummyOutStream.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/FindSignature.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/InStreamWithCRC.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/ItemNameUtils.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/MultiStream.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/OutStreamWithCRC.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/OutStreamWithSha1.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/HandlerOut.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Common/ParseProperties.cpp
)

# Archive/7z
set(SEVENZIP_ARCHIVE_7Z_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zCompressionMode.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zDecode.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zEncode.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zExtract.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zFolderInStream.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zHandlerOut.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zHeader.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zIn.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zOut.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zProperties.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zSpecStream.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zUpdate.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z/7zRegister.cpp
)

# Archive/Cab
set(SEVENZIP_ARCHIVE_CAB_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Cab/CabBlockInStream.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Cab/CabHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Cab/CabHeader.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Cab/CabIn.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Cab/CabRegister.cpp
)

# Archive/Chm
set(SEVENZIP_ARCHIVE_CHM_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Chm/ChmHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Chm/ChmIn.cpp
)

# Archive/Iso
set(SEVENZIP_ARCHIVE_ISO_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Iso/IsoHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Iso/IsoHeader.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Iso/IsoIn.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Iso/IsoRegister.cpp
)

# Archive/Nsis
set(SEVENZIP_ARCHIVE_NSIS_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Nsis/NsisDecode.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Nsis/NsisHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Nsis/NsisIn.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Nsis/NsisRegister.cpp
)

# Archive/Rar
set(SEVENZIP_ARCHIVE_RAR_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Rar/RarHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Rar/Rar5Handler.cpp
)

# Archive/Tar
set(SEVENZIP_ARCHIVE_TAR_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Tar/TarHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Tar/TarHandlerOut.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Tar/TarHeader.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Tar/TarIn.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Tar/TarOut.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Tar/TarUpdate.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Tar/TarRegister.cpp
)

# Archive/Udf
set(SEVENZIP_ARCHIVE_UDF_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Udf/UdfHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Udf/UdfIn.cpp
)

# Archive/Wim
set(SEVENZIP_ARCHIVE_WIM_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Wim/WimHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Wim/WimHandlerOut.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Wim/WimIn.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Wim/WimRegister.cpp
)

# Archive/Zip
set(SEVENZIP_ARCHIVE_ZIP_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Zip/ZipAddCommon.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Zip/ZipHandler.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Zip/ZipHandlerOut.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Zip/ZipIn.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Zip/ZipItem.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Zip/ZipOut.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Zip/ZipUpdate.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Zip/ZipRegister.cpp
)

# Archive/GZip
set(SEVENZIP_ARCHIVE_GZIP_SOURCES
    # GzHandler.cpp already included in main archive sources
)

# Archive/BZip2
set(SEVENZIP_ARCHIVE_BZIP2_SOURCES
    # Bz2Handler.cpp already included in main archive sources
)

# Archive/Lzma
set(SEVENZIP_ARCHIVE_LZMA_SOURCES
    # LzmaHandler.cpp already included in main archive sources
)

# ============================================================================
# CPP/7zip/Compress 源文件
# ============================================================================
set(SEVENZIP_COMPRESS_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Bcj2Coder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Bcj2Register.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BcjCoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BcjRegister.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BitlDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BranchMisc.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BranchRegister.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/ByteSwap.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BZip2Crc.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BZip2Decoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BZip2Encoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BZip2Register.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/CopyCoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/CopyRegister.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Deflate64Register.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/DeflateDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/DeflateEncoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/DeflateRegister.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/DeltaFilter.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/ImplodeDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LzfseDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LzhDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Lzma2Decoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Lzma2Encoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Lzma2Register.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LzmaDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LzmaEncoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LzmaRegister.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LzmsDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LzOutWindow.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LzxDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/PpmdDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/PpmdEncoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/PpmdRegister.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/PpmdZip.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/QuantumDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Rar1Decoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Rar2Decoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Rar3Decoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Rar3Vm.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Rar5Decoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/RarCodecsRegister.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/ShrinkDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/XpressDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/XzDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/XzEncoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/ZlibDecoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/ZlibEncoder.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/ZDecoder.cpp
)

# Zstd Compress
if(SEVENZIP_ENABLE_ZSTD)
    list(APPEND SEVENZIP_COMPRESS_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/ZstdDecoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/ZstdEncoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/ZstdRegister.cpp
    )
endif()

# Brotli Compress
if(SEVENZIP_ENABLE_BROTLI)
    list(APPEND SEVENZIP_COMPRESS_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BrotliDecoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BrotliEncoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/BrotliRegister.cpp
    )
endif()

# LZ4 Compress
if(SEVENZIP_ENABLE_LZ4)
    list(APPEND SEVENZIP_COMPRESS_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Lz4Decoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Lz4Encoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Lz4Register.cpp
    )
endif()

# LZ5 Compress
if(SEVENZIP_ENABLE_LZ5)
    list(APPEND SEVENZIP_COMPRESS_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Lz5Decoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Lz5Encoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/Lz5Register.cpp
    )
endif()

# Lizard Compress
if(SEVENZIP_ENABLE_LIZARD)
    list(APPEND SEVENZIP_COMPRESS_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LizardDecoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LizardEncoder.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/LizardRegister.cpp
    )
endif()

# Fast LZMA2 Register
if(SEVENZIP_ENABLE_FAST_LZMA2)
    list(APPEND SEVENZIP_COMPRESS_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/FastLzma2Register.cpp
    )
endif()

# ============================================================================
# CPP/7zip/Crypto 源文件
# ============================================================================
if(SEVENZIP_ENABLE_ENCRYPTION)
    set(SEVENZIP_CRYPTO_SOURCES
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/7zAes.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/7zAesRegister.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/HmacSha1.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/HmacSha256.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/MyAes.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/MyAesReg.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/Pbkdf2HmacSha1.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/RandGen.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/Rar20Crypto.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/Rar5Aes.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/RarAes.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/WzAes.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/ZipCrypto.cpp
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto/ZipStrong.cpp
    )
else()
    set(SEVENZIP_CRYPTO_SOURCES "")
endif()

# ============================================================================
# 收集所有源文件
# ============================================================================
set(SEVENZIP_ALL_SOURCES
    ${SEVENZIP_C_SOURCES}
    ${SEVENZIP_HASHES_SOURCES}
    ${SEVENZIP_CPP_COMMON_SOURCES}
    ${SEVENZIP_CPP_WINDOWS_SOURCES}
    ${SEVENZIP_7ZIP_COMMON_SOURCES}
    ${SEVENZIP_ARCHIVE_SOURCES}
    ${SEVENZIP_ARCHIVE_COMMON_SOURCES}
    ${SEVENZIP_ARCHIVE_7Z_SOURCES}
    ${SEVENZIP_ARCHIVE_CAB_SOURCES}
    ${SEVENZIP_ARCHIVE_CHM_SOURCES}
    ${SEVENZIP_ARCHIVE_ISO_SOURCES}
    ${SEVENZIP_ARCHIVE_NSIS_SOURCES}
    ${SEVENZIP_ARCHIVE_RAR_SOURCES}
    ${SEVENZIP_ARCHIVE_TAR_SOURCES}
    ${SEVENZIP_ARCHIVE_UDF_SOURCES}
    ${SEVENZIP_ARCHIVE_WIM_SOURCES}
    ${SEVENZIP_ARCHIVE_ZIP_SOURCES}
    ${SEVENZIP_COMPRESS_SOURCES}
    ${SEVENZIP_CRYPTO_SOURCES}
)

# 添加额外压缩算法的源文件
if(SEVENZIP_ENABLE_ZSTD)
    list(APPEND SEVENZIP_ALL_SOURCES ${SEVENZIP_ZSTD_SOURCES})
endif()

if(SEVENZIP_ENABLE_LZ4)
    list(APPEND SEVENZIP_ALL_SOURCES ${SEVENZIP_LZ4_SOURCES})
endif()

if(SEVENZIP_ENABLE_LZ5)
    list(APPEND SEVENZIP_ALL_SOURCES ${SEVENZIP_LZ5_SOURCES})
endif()

if(SEVENZIP_ENABLE_LIZARD)
    list(APPEND SEVENZIP_ALL_SOURCES ${SEVENZIP_LIZARD_SOURCES})
endif()

if(SEVENZIP_ENABLE_BROTLI)
    list(APPEND SEVENZIP_ALL_SOURCES ${SEVENZIP_BROTLI_SOURCES})
endif()

if(SEVENZIP_ENABLE_FAST_LZMA2)
    list(APPEND SEVENZIP_ALL_SOURCES ${SEVENZIP_FAST_LZMA2_SOURCES})
endif()

# 多线程封装
if(SEVENZIP_ENABLE_ZSTD OR SEVENZIP_ENABLE_LZ4 OR SEVENZIP_ENABLE_LZ5 OR SEVENZIP_ENABLE_LIZARD OR SEVENZIP_ENABLE_BROTLI)
    list(APPEND SEVENZIP_ALL_SOURCES ${SEVENZIP_ZSTDMT_SOURCES})
endif()

# ============================================================================
# 导出/注册文件 - 静态库和动态库都需要
# ============================================================================
# ArchiveExports.cpp - 定义 RegisterArc, g_NumArcs 等
# CodecExports.cpp - 定义 g_NumCodecs, CreateCoder 等  
# DllExports2.cpp - 包含 MyInitGuid.h，初始化所有 GUID (IID_*)
set(SEVENZIP_EXPORT_SOURCES
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/ArchiveExports.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/DllExports2.cpp
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress/CodecExports.cpp
)

# ============================================================================
# 头文件包含路径
# ============================================================================
set(SEVENZIP_INCLUDE_DIRS
    ${SEVENZIP_SOURCE_DIR}/C
    ${SEVENZIP_SOURCE_DIR}/C/hashes
    ${SEVENZIP_SOURCE_DIR}/CPP
    ${SEVENZIP_SOURCE_DIR}/CPP/Common
    ${SEVENZIP_SOURCE_DIR}/CPP/Windows
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Compress
    ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Crypto
)

# 添加额外压缩算法的头文件路径
if(SEVENZIP_ENABLE_ZSTD)
    list(APPEND SEVENZIP_INCLUDE_DIRS
        ${SEVENZIP_SOURCE_DIR}/C/zstd
        ${SEVENZIP_SOURCE_DIR}/C/zstdmt
    )
endif()

if(SEVENZIP_ENABLE_LZ4)
    list(APPEND SEVENZIP_INCLUDE_DIRS ${SEVENZIP_SOURCE_DIR}/C/lz4)
endif()

if(SEVENZIP_ENABLE_LZ5)
    list(APPEND SEVENZIP_INCLUDE_DIRS ${SEVENZIP_SOURCE_DIR}/C/lz5)
endif()

if(SEVENZIP_ENABLE_LIZARD)
    list(APPEND SEVENZIP_INCLUDE_DIRS ${SEVENZIP_SOURCE_DIR}/C/lizard)
endif()

if(SEVENZIP_ENABLE_BROTLI)
    list(APPEND SEVENZIP_INCLUDE_DIRS
        ${SEVENZIP_SOURCE_DIR}/C/brotli
        ${SEVENZIP_SOURCE_DIR}/C/brotli/common
        ${SEVENZIP_SOURCE_DIR}/C/brotli/enc
        ${SEVENZIP_SOURCE_DIR}/C/brotli/dec
    )
endif()

if(SEVENZIP_ENABLE_FAST_LZMA2)
    list(APPEND SEVENZIP_INCLUDE_DIRS ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2)
endif()

# ============================================================================
# 创建静态库
# ============================================================================
if(SEVENZIP_BUILD_STATIC)
    add_library(7zip_static STATIC 
        ${SEVENZIP_ALL_SOURCES}
        ${SEVENZIP_EXPORT_SOURCES}
    )
    
    target_compile_definitions(7zip_static PRIVATE
        ${SEVENZIP_COMPILE_DEFINITIONS}
        _7ZIP_STATIC
        Z7_EXTERNAL_CODECS=0
    )
    
    target_include_directories(7zip_static PUBLIC ${SEVENZIP_INCLUDE_DIRS})
    
    if(WIN32)
        target_link_libraries(7zip_static PUBLIC
            oleaut32
            ole32
            user32
            advapi32
        )
    endif()
    
    if(MSVC)
        target_compile_options(7zip_static PRIVATE
            /wd4996  # deprecated functions
            /wd4267  # size_t to int conversion
            /wd4244  # type conversion warnings
            /wd4018  # signed/unsigned mismatch
            /wd4146  # unary minus operator
            /wd4334  # shift result implicitly converted
            /wd4311  # pointer truncation
            /wd4312  # conversion to greater size
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # GCC/Clang 需要的兼容性选项
        target_compile_options(7zip_static PRIVATE
            -Wno-deprecated-declarations
            -Wno-unused-parameter
            -Wno-sign-compare
            -Wno-conversion
            -include "${CMAKE_CURRENT_SOURCE_DIR}/compat.h"
        )
        # C++ 特定选项
        target_compile_options(7zip_static PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-reorder>
        )
    endif()
    
    set_target_properties(7zip_static PROPERTIES
        OUTPUT_NAME "7z"
        ARCHIVE_OUTPUT_NAME "7z"
    )
endif()

# ============================================================================
# 创建动态库
# ============================================================================
if(SEVENZIP_BUILD_SHARED)
    add_library(7zip_shared SHARED
        ${SEVENZIP_ALL_SOURCES}
        ${SEVENZIP_EXPORT_SOURCES}
    )
    
    target_compile_definitions(7zip_shared PRIVATE
        ${SEVENZIP_COMPILE_DEFINITIONS}
        Z7_EXTERNAL_CODECS
    )
    
    target_include_directories(7zip_shared PUBLIC ${SEVENZIP_INCLUDE_DIRS})
    
    if(WIN32)
        target_link_libraries(7zip_shared PRIVATE
            oleaut32
            ole32
            user32
            advapi32
        )
        
        # 使用 .def 文件定义导出
        set(DEF_FILE "${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/Archive2.def")
        if(EXISTS ${DEF_FILE})
            target_sources(7zip_shared PRIVATE ${DEF_FILE})
        endif()
    endif()
    
    if(MSVC)
        target_compile_options(7zip_shared PRIVATE
            /wd4996
            /wd4267
            /wd4244
            /wd4018
            /wd4146
            /wd4334
            /wd4311
            /wd4312
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # GCC/Clang 需要的兼容性选项
        target_compile_options(7zip_shared PRIVATE
            -Wno-deprecated-declarations
            -Wno-unused-parameter
            -Wno-sign-compare
            -Wno-conversion
            -include "${CMAKE_CURRENT_SOURCE_DIR}/compat.h"
        )
        # C++ 特定选项
        target_compile_options(7zip_shared PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-reorder>
        )
        # 允许多重定义（zstdmt threading 与系统 pthread 冲突）
        target_link_options(7zip_shared PRIVATE -Wl,--allow-multiple-definition)
    endif()
    
    set_target_properties(7zip_shared PROPERTIES
        OUTPUT_NAME "7z"
        RUNTIME_OUTPUT_NAME "7z"
    )
endif()

# ============================================================================
# 打印配置信息
# ============================================================================
message(STATUS "============================================")
message(STATUS "7-Zip Library Configuration:")
message(STATUS "============================================")
message(STATUS "  Source directory: ${SEVENZIP_SOURCE_DIR}")
message(STATUS "  Build static library: ${SEVENZIP_BUILD_STATIC}")
message(STATUS "  Build shared library: ${SEVENZIP_BUILD_SHARED}")
message(STATUS "--------------------------------------------")
message(STATUS "  Encryption: ${SEVENZIP_ENABLE_ENCRYPTION}")
message(STATUS "  Multithreading: ${SEVENZIP_ENABLE_MULTITHREADING}")
message(STATUS "--------------------------------------------")
message(STATUS "  Zstd: ${SEVENZIP_ENABLE_ZSTD}")
message(STATUS "  LZ4: ${SEVENZIP_ENABLE_LZ4}")
message(STATUS "  LZ5: ${SEVENZIP_ENABLE_LZ5}")
message(STATUS "  Lizard: ${SEVENZIP_ENABLE_LIZARD}")
message(STATUS "  Brotli: ${SEVENZIP_ENABLE_BROTLI}")
message(STATUS "  Fast LZMA2: ${SEVENZIP_ENABLE_FAST_LZMA2}")
message(STATUS "============================================")

# 统计源文件数量
list(LENGTH SEVENZIP_ALL_SOURCES _all_count)
message(STATUS "  Total sources: ${_all_count}")
