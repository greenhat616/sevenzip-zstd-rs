cmake_minimum_required(VERSION 3.16)
project(7zip_test LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# 配置路径
# ============================================================================

# 7-Zip 源码路径
set(SEVENZIP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../7-Zip-zstd" CACHE PATH "7-Zip source")

# 静态库路径 (GCC/Ninja 构建)
set(SEVENZIP_STATIC_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/build/lib7z.a" CACHE FILEPATH "7-Zip static lib")

# 动态库路径
set(SEVENZIP_SHARED_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/build/7z.dll" CACHE FILEPATH "7-Zip shared lib")

# ============================================================================
# 测试类型选项
# ============================================================================
option(TEST_STATIC "Test static library" ON)
option(TEST_SHARED "Test shared library (via LoadLibrary)" OFF)

# ============================================================================
# 编译定义
# ============================================================================
set(COMMON_DEFINITIONS
    UNICODE
    _UNICODE
    _REENTRANT
    _FILE_OFFSET_BITS=64
    _LARGEFILE_SOURCE
)

if(WIN32)
    list(APPEND COMMON_DEFINITIONS
        _WIN32
        WIN32
        _WINDOWS
    )
endif()

# ============================================================================
# 测试程序 - 静态库测试
# ============================================================================
if(TEST_STATIC)
    add_executable(7zip_static_test
        test_static.cpp
    )
    
    target_compile_definitions(7zip_static_test PRIVATE
        ${COMMON_DEFINITIONS}
        _7ZIP_STATIC
        Z7_EXTERNAL_CODECS=0
    )
    
    # 头文件路径
    target_include_directories(7zip_static_test PRIVATE
        ${SEVENZIP_SOURCE_DIR}/C
        ${SEVENZIP_SOURCE_DIR}/C/hashes
        ${SEVENZIP_SOURCE_DIR}/C/zstd
        ${SEVENZIP_SOURCE_DIR}/C/lz4
        ${SEVENZIP_SOURCE_DIR}/C/lz5
        ${SEVENZIP_SOURCE_DIR}/C/lizard
        ${SEVENZIP_SOURCE_DIR}/C/brotli
        ${SEVENZIP_SOURCE_DIR}/C/fast-lzma2
        ${SEVENZIP_SOURCE_DIR}/CPP
        ${SEVENZIP_SOURCE_DIR}/CPP/Common
        ${SEVENZIP_SOURCE_DIR}/CPP/Windows
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Common
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip/Archive/7z
    )
    
    # 链接静态库
    target_link_libraries(7zip_static_test PRIVATE
        ${SEVENZIP_STATIC_LIB}
    )
    
    # Windows 系统库
    if(WIN32)
        target_link_libraries(7zip_static_test PRIVATE
            oleaut32
            ole32
            user32
            advapi32
        )
    endif()
    
    # GCC/MinGW 特定设置
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(7zip_static_test PRIVATE
            -Wno-deprecated-declarations
            -Wno-unused-parameter
            -Wno-sign-compare
        )
        target_link_options(7zip_static_test PRIVATE
            -static-libgcc
            -static-libstdc++
            # 允许多重定义 (zstdmt threading 与 libpthread 冲突)
            -Wl,--allow-multiple-definition
            -Wl,--whole-archive
            ${SEVENZIP_STATIC_LIB}
            -Wl,--no-whole-archive
        )
    endif()
    
    # MSVC 特定设置
    if(MSVC)
        target_compile_options(7zip_static_test PRIVATE
            /wd4996
            /wd4267
            /wd4244
        )
        target_link_options(7zip_static_test PRIVATE
            /WHOLEARCHIVE:${SEVENZIP_STATIC_LIB}
        )
    endif()
endif()

# ============================================================================
# 测试程序 - 动态库测试 (使用 LoadLibrary)
# ============================================================================
if(TEST_SHARED)
    add_executable(7zip_shared_test
        test_shared.cpp
    )
    
    target_compile_definitions(7zip_shared_test PRIVATE
        ${COMMON_DEFINITIONS}
    )
    
    target_include_directories(7zip_shared_test PRIVATE
        ${SEVENZIP_SOURCE_DIR}/C
        ${SEVENZIP_SOURCE_DIR}/CPP
        ${SEVENZIP_SOURCE_DIR}/CPP/Common
        ${SEVENZIP_SOURCE_DIR}/CPP/Windows
        ${SEVENZIP_SOURCE_DIR}/CPP/7zip
    )
    
    if(WIN32)
        target_link_libraries(7zip_shared_test PRIVATE
            oleaut32
            ole32
            user32
            advapi32
        )
    endif()
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(7zip_shared_test PRIVATE
            -Wno-deprecated-declarations
        )
    endif()
endif()

# ============================================================================
# 打印配置
# ============================================================================
message(STATUS "========================================")
message(STATUS "7-Zip Test Configuration:")
message(STATUS "========================================")
message(STATUS "  Source: ${SEVENZIP_SOURCE_DIR}")
message(STATUS "  Static lib: ${SEVENZIP_STATIC_LIB}")
message(STATUS "  Test static: ${TEST_STATIC}")
message(STATUS "  Test shared: ${TEST_SHARED}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "========================================")
