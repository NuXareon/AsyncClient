#pragma once

#if _DEBUG
#include <cstdio>
#include <cassert>
#define DebugLog(...) printf(__VA_ARGS__); 
#define ValidateLog(a, ...) if (!a) {printf(__VA_ARGS__);}
#define AssertLog(a, ...) if (!a) {printf(__VA_ARGS__); assert(false);}
#else
#define DebugLog(...) ((void)0)
#define ValidateLog(a, ...) ((void)0)
#define AssertLog(a, ...) ((void)0)
#endif // _DEBUG