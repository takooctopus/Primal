#pragma once

#pragma warning(disable: 4530) //Ω˚”√“Ï≥£æØ∏Ê warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// C/C++
#include <stdint.h>
#include <assert.h>
#include <typeinfo>
#include <memory>


#if defined(_WIN64)
#include <DirectXMath.h>
#endif

#ifndef DISABLE_COPY
#define DISABLE_COPY(T)								\
	explicit T(const T&) = delete;					\
	T& operator=(const T&) = delete;
#endif // !DISABLE_COPY

#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T)								\
	explicit T(T&&) = delete;						\
	T& operator=(T&&) = delete;
#endif // !DISABLE_COPY

#ifndef DISABLE_COPY_AND_MOVE
#define DISABLE_COPY_AND_MOVE(T) DISABLE_COPY(T) DISABLE_MOVE(T)
#endif // !DISABLE_COPY_AND_MOVE


// Common headers
#include "PrimitiveTypes.h"	
#include "Id.h"
#include "..\Utilities\Utilities.h"
#include "..\Utilities\MathTypes.h"
#include "..\Utilities\Math.h"


// macros

#ifdef _DEBUG
#define DEBUG_OP(x) x
#else 
#define DEBUG_OP(x) (void(0))
#endif // _DEBUG
