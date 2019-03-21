#include "CommandTool.h"

void FCommandTool::Init(FVirtualDisk * _virtualDisk)
{
	Assert(_virtualDisk != nullptr);

	static std::pair<std::string, void(FCommandTool::*)(const std::vector<std::string> &)> map[] =
	{
		{"dir",	&FCommandTool::Dir},
		{"md", &FCommandTool::Md},
		{"cd", &FCommandTool::Cd},
		{"copy", &FCommandTool::Copy},
		{"del", &FCommandTool::Del},
		{"rd", &FCommandTool::Rd},
		{"ren", &FCommandTool::Ren},
		{"move", &FCommandTool::Move},
		{"mklink", &FCommandTool::Mklink},
		{"cls", &FCommandTool::Cls},
		{"save", &FCommandTool::Save},
		{"load", &FCommandTool::Load},
	};

	for (const auto & _ele : map)
		mFuncMap.insert(_ele);

	mVirtualDisk = _virtualDisk;

	mVirtualDisk->InitCommandTool(this, mCurrentDirectory);
}

void FCommandTool::Exec(const std::string & _cmdLine)
{
	std::vector<std::string> cmdLines;
	bool isInDoubleQuotes = false;
	bool IsMatching = false;

	for (auto it = _cmdLine.begin(); it != _cmdLine.end(); NULL)
	{
		for (NULL; it != _cmdLine.end() && *it == L' '; ++it);
		if (it == _cmdLine.end()) break;

		auto begin = it;

		if (*begin == L'\"')
		{
			for (it = ++begin; it != _cmdLine.end() && *it != L'\"'; ++it);
			if (begin == it)
				continue;
			else
			{
				cmdLines.push_back(std::string(begin, it));
				if (it == _cmdLine.end()) break;
				if (*it == L'\"') ++it;
			}
		}
		else
		{
			for (NULL; it != _cmdLine.end() && *it != L' '; ++it);
			if (begin == it)
				continue;
			else
				cmdLines.push_back(std::string(begin, it));
		}
	}

	if (mFuncMap.count(cmdLines[0]) == 0)
	{
		printf_s("'%s'不是内部或外部命令，也不是可运行的程序或批处理文件。", cmdLines[0].c_str());
	}
	else
	{
		(this->*mFuncMap[cmdLines[0]])(cmdLines);
	}
}

void FCommandTool::Dir(const std::vector<std::string> & _params)
{
	bool with_s = false;
	bool with_ad = false;
	FPath path;
	for (auto it = _params.begin() + 1; it != _params.end(); ++it)
	{
		bool flag = false;
		if (*it == "/s")
		{
			with_s = true;
			flag = true;
		}

		if (*it == "/ad")
		{
			with_ad = true;
			flag = true;
		}
		if (!flag)
			path = FPath(*it);
	}

	std::vector<std::string> names;
	uint64_t errorCode = mVirtualDisk->Dir(mCurrentDirectory, with_s, with_ad, path, names);

	if (errorCode != 0)
		Assert(false);
	else
	{
		for (auto & _ele : names)
		{
			printf_s("%s\n", _ele.c_str());
		}
	}
}

void FCommandTool::Md(const std::vector<std::string> & _params)
{
	Assert(_params.size() == 2);

	uint64_t errorCode = mVirtualDisk->Md(mCurrentDirectory, FPath(_params[1]));

	if (errorCode != 0)
		Assert(false);
}

void FCommandTool::Cd(const std::vector<std::string> & _params)
{
	Assert(_params.size() <= 2);
	
	if (_params.size() == 1)
	{
	}
}