#pragma once

#if _DEBUG
#include <cstdio>
#define DebugLog(...) printf(__VA_ARGS__); 
#define ValidateLog(a, ...) if (!a) {printf(__VA_ARGS__);}
#else
#define DebugLog(...) ((void)0)
#define ValidateLog(a, ...) ((void)0)
#endif // _DEBUG