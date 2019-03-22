#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <assert.h>
#include <stdint.h>
#include <vector>

#if defined(DEBUG) || defined(_DEBUG)
#define DEBUG
constexpr bool IsDebugMode = true;
constexpr bool IsReleaseMode = false;
#else
constexpr bool IsDebugMode = false;
constexpr bool IsReleaseMode = true;
#endif

constexpr float PI = 3.1415928f;
constexpr float PI2 = 2 * PI;

#pragma warning(disable : 4244 4267)


#define LogA(_format, ...)					\
do											\
{											\
	char buffer[1024];						\
	sprintf_s(buffer, _format, __VA_ARGS__);\
	OutputDebugStringA(buffer);				\
} while (0)

#define LogW(_format, ...)						\
do												\
{												\
	wchar_t buffer[1024];						\
	swprintf_s(buffer, _format, __VA_ARGS__);	\
	OutputDebugStringW(buffer);					\
} while (0)

#ifdef UNICODE
#define Log LogA
#else
#define Log LogW
#endif

#ifdef DEBUG
#define DebugMessageBoxA(_caption, _format, ...)									\
do																					\
{																					\
	char buffer[1024];																\
	sprintf_s(buffer, _format, __VA_ARGS__);										\
	MessageBoxA(0, buffer, _caption, 0);											\
	__debugbreak();																	\
} while (0)

#define DebugMessageBoxW(_caption, _format, ...)									\
do																					\
{																					\
	wchar_t buffer[1024];															\
	swprintf_s(buffer, _format, __VA_ARGS__);										\
	MessageBoxW(0, buffer, _caption, 0);											\
	__debugbreak();																	\
} while (0)

#else
#define  DebugMessageBoxA(...) ((void)(0))
#define  DebugMessageBoxW(...) ((void)(0))
#endif // DEBUG

#ifdef UNICODE
#define DebugMessageBox DebugMessageBoxW
#else
#define DebugMessageBox DebugMessageBoxA
#endif

#ifdef DEBUG
#define AssertDebugMessageBoxW(_expression, _caption, _format, ...) \
do {																\
	if (!!(_expression))											\
		DebugMessageBoxW(_caption, _format, __VA_ARGS__);			\
} while (0)

#define AssertDebugMessageBoxA(_expression, _caption, _format, ...) \
do {																\
	if (!!(_expression))											\
		DebugMessageBoxA(_caption, _format, __VA_ARGS__);			\
} while (0)
#else
#define AssertDebugMessageBoxW ((void)0)
#define AssertDebugMessageBoxA ((void)0)
#endif

#ifdef UNICODE
#define AssertDebugMessageBox AssertDebugMessageBoxW
#else
#define AssertDebugMessageBox AssertDebugMessageBoxA
#endif

#ifdef DEBUG
#define Assert(_expression)															\
do																					\
{																					\
	if (!(_expression))																\
		DebugMessageBox(															\
			TEXT("assert failed"),													\
			TEXT("file: %s\n" "func: %s\n" "line: %d\n" "%s"),						\
			TEXT(__FILE__), TEXT(__FUNCTION__),  (int)__LINE__, L###_expression);	\
} while (0)
#else
#define Assert(_expression) ((void)0)
#endif

#define Comment(...)

/**
* utility template for a class that should not be copyable.
* Derive from this class to make your class non-copyable
*/
class FNoncopyable
{
protected:
	// ensure the class cannot be constructed directly
	FNoncopyable() {}
	// the class should not be used polymorphically
	~FNoncopyable() {}
private:
	FNoncopyable(const FNoncopyable&) = delete;
	FNoncopyable& operator=(const FNoncopyable&) = delete;
};

template<typename _Ty>
struct FLess
{
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef _Ty first_argument_type;
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef _Ty second_argument_type;
	_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef bool result_type;

	constexpr bool operator()(const _Ty& _Left, const _Ty& _Right) const
	{
		return memcmp(&_Left, &_Right, sizeof(_Ty)) < 0;
	}
};

template<typename _charType>
void LoadStringFromFile(std::ifstream & _ifs, std::basic_string<_charType> & _dest)
{
	uint64_t stringSize = 0;
	_ifs.read(reinterpret_cast<char*>(&stringSize), sizeof(uint64_t));
	_dest.resize(stringSize);
	_ifs.read(reinterpret_cast<char*>(&_dest[0]), sizeof(std::basic_string<_charType>::value_type) * stringSize);
}

template<typename _charType>
void SaveStringToFile(std::ofstream & _ofs, const std::basic_string<_charType> & _source)
{
	uint64_t stringSize = _source.size();
	_ofs.write(reinterpret_cast<char*>(&stringSize), sizeof(uint64_t));
	_ofs.write(reinterpret_cast<const char*>(_source.data()), sizeof(std::basic_string<_charType>::value_type) * stringSize);
}

bool IsMatch(Comment(without * and ? ) const std::string & _str1, Comment(with * and ? ) const std::string & _str2);

std::vector<std::string> SplitCmdLine(std::string _cmdLine);
