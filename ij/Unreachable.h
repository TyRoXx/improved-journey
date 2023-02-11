#pragma once

#ifdef _MSC_VER
#define IJ_UNREACHABLE() __assume(false)
#else
#define IJ_UNREACHABLE() __builtin_unreachable()
#endif
