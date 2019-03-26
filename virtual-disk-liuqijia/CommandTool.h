#pragma once
#include <string>
#include <map>
#include <vector>
#include "File.h"
#include "VirtualDisk.h"
#include "ErrorConstant.h"

class FCommandTool
{
	FCommandTool() = default;

public:
	static FCommandTool & Get()
	{
		static FCommandTool single;
		return single;
	}

	void Init(FVirtualDisk * _virtualDisk);
	void Clear();

	void Exec(const std::string & _cmdLine);

	Comment(for test) const FPath & GetCurrentPath()const { return mCurrentPath; }

private:
#define RegisterFunc(ProcessFunc) uint64_t ProcessFunc(const std::vector<std::string> & _params)
	RegisterFunc(Dir);
	RegisterFunc(Md);
	RegisterFunc(Cd);
	RegisterFunc(Copy);
	RegisterFunc(Del);
	RegisterFunc(Rd);
	RegisterFunc(Ren);
	RegisterFunc(Move);
	RegisterFunc(Mklink);
	RegisterFunc(Cls);
	RegisterFunc(Save);
	RegisterFunc(Load);
#undef RegisterFunc

	void ErrorProcess(uint64_t _errorCode, const std::vector<std::string> & _cmdLines);

private:
	FVirtualDisk * mVirtualDisk;
	FDirectory * mCurrentDirectory;
	std::map<std::string, uint64_t(FCommandTool::*)(const std::vector<std::string> &)> mFuncMap;
	FPath mCurrentPath;
};
