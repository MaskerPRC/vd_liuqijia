#pragma once
#include <stdint.h>

#define RegisterErrorCode(_codeName, _value, ...) constexpr uint64_t _codeName = _value;
#include "ErrorCode.inl"
#undef RegisterErrorCode

const char * GetDetail(uint64_t _errorCode);