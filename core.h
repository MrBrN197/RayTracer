#pragma once
#include <stdint.h>

#define ASSERT(X) if(!(X)) { __debugbreak();}

typedef uint32_t uint32;
typedef int32_t int32;