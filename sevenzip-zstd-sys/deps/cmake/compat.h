// compat.h - Compatibility header for GCC/MinGW
// This header must be force-included before any other header
// It ensures min/max macros are available like in MSVC

#ifndef SEVENZIP_COMPAT_H
#define SEVENZIP_COMPAT_H

#ifdef _WIN32
// Include Windows headers first
#include <windows.h>
#endif

// Define min/max macros if not already defined
// These are available by default in MSVC's windows.h
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#endif // SEVENZIP_COMPAT_H
