#pragma once

#if _DEBUG
#include <cstdio>
#define DebugLog(...) printf(__VA_ARGS__); 
#else
#define DebugLog(...) ((void)0)
#endif // _DEBUG