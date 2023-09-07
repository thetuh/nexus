#include "stl.h"

EXTERN_C
_ACRTIMP void __cdecl _invoke_watson(
	_In_opt_z_ wchar_t const* _Expression,
	_In_opt_z_ wchar_t const* _FunctionName,
	_In_opt_z_ wchar_t const* _FileName,
	_In_       unsigned int _LineNo,
	_In_       uintptr_t _Reserved )
{
}

namespace std { _Prhand _Raise_handler; }