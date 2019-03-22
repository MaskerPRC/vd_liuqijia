#include "ErrorConstant.h"

const char * GetDetail(uint64_t _errorCode)
{
	switch (_errorCode)
	{
#define RegisterErrorCode(_codeName, _value, _detail) case _value : return _detail;
#include "ErrorCode.inl"
#undef RegisterErrorCode
	default:
		return "Unknow ErrorCode";
	}
}
