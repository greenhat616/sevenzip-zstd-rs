#ifndef SEVENZIP_STATIC_INIT_H
#define SEVENZIP_STATIC_INIT_H

/**
 * 7-Zip 静态库初始化
 *
 * 问题背景：
 * 7-Zip 使用全局静态对象的构造函数来注册 codecs 和 archive handlers。
 * 当编译为静态库时，链接器可能会优化掉这些未被直接引用的对象，
 * 导致 g_NumCodecs 和 g_NumArcs 为 0，压缩/解压功能失效。
 *
 * 解决方案：
 * 在使用 7-Zip 功能前调用 SevenZip_StaticInit()，
 * 该函数会强制引用所有需要的注册对象。
 */

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * 初始化 7-Zip 静态库
     * 必须在使用任何 7-Zip 功能前调用一次
     *
     * @return 0 表示成功
     */
    int SevenZip_StaticInit(void);

    /**
     * 获取已注册的 codec 数量
     * 用于验证初始化是否成功
     */
    int SevenZip_GetNumCodecs(void);

    /**
     * 获取已注册的 archive handler 数量
     * 用于验证初始化是否成功
     */
    int SevenZip_GetNumArcs(void);

#ifdef __cplusplus
}
#endif

#endif // SEVENZIP_STATIC_INIT_H