#pragma once
#include <string>
#include <map>
#include <vector>
#include "File.h"
#include "VirtualDisk.h"

class FCommandTool
{
	FCommandTool() = default;

public:
	static FCommandTool & Get()
	{
		FCommandTool single;
		return single;
	}

	void Init(FVirtualDisk * _virtualDisk);
	void Clear();

	void Exec(const std::string & _cmdLine);

private:
#define RegisterFunc(ProcessFunc) void ProcessFunc(const std::vector<std::string> & _params)
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

private:
	FVirtualDisk * mVirtualDisk;
	FDirectory * mCurrentDirectory;
	std::map<std::string, void(FCommandTool::*)(const std::vector<std::string> &)> mFuncMap;
};
