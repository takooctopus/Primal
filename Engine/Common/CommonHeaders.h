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
