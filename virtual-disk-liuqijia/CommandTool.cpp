#include "CommandTool.h"

void FCommandTool::Init(FVirtualDisk * _virtualDisk)
{
	Assert(_virtualDisk != nullptr);

	static std::pair<std::string, uint64_t(FCommandTool::*)(const std::vector<std::string> &)> map[] =
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
		{"cd.", &FCommandTool::Cd},
		{"cd..", &FCommandTool::Cd},
	};

	for (const auto & _ele : map)
		mFuncMap.insert(_ele);

	mVirtualDisk = _virtualDisk;

	mVirtualDisk->InitCommandTool(this, mCurrentDirectory);
}

void FCommandTool::Clear()
{
	mVirtualDisk = nullptr;
	mCurrentDirectory = nullptr;
}

void FCommandTool::Exec(const std::string & _cmdLine)
{
	std::vector<std::string> cmdLines = SplitCmdLine(_cmdLine);

	uint64_t errorCode = E_OK;
	if (cmdLines.size() == 0)
		errorCode = E_PARAMETER_ERROR;
	else
	{
		if (mFuncMap.count(cmdLines[0]) == 0)
			errorCode = E_COMMAND_ERROR;
		else
			errorCode = (this->*mFuncMap[cmdLines[0]])(cmdLines);
	}

	if (errorCode != 0)
	{
		ErrorProcess(errorCode, cmdLines);
	}
}

uint64_t FCommandTool::Dir(const std::vector<std::string> & _params)
{
	bool with_s = false;
	bool with_ad = false;
	std::vector<FPath> paths;
	for (auto it = _params.begin() + 1; it != _params.end(); ++it)
	{
		if (*it == "/s")
		{
			with_s = true;
			continue;
		}
		else if (*it == "/ad")
		{
			with_ad = true;
			continue;
		}

		paths.push_back(FPath(*it));
	}
	if (paths.size() == 0) paths.push_back(FPath("."));


	for (auto _ele : paths)
	{
		std::vector<std::string> names;
		uint64_t errorCode = mVirtualDisk->Dir(mCurrentDirectory, with_s, with_ad, _ele, names);

		if (errorCode != E_OK)
			ErrorProcess(errorCode, _params);
		else
		{
			for (auto & _ele : names)
			{
				printf_s("%s\n", _ele.c_str());
			}
		}
	}

	return E_OK;
}

uint64_t FCommandTool::Md(const std::vector<std::string> & _params)
{
	if (_params.size() != 2) return E_PARAMETER_ERROR;

	return mVirtualDisk->Md(mCurrentDirectory, FPath(_params[1]));
}

uint64_t FCommandTool::Cd(const std::vector<std::string> & _params)
{
	if (_params.size() > 2) return E_PARAMETER_ERROR;

	if (_params.size() == 1)
	{
		if (_params[0] == "cd.")
			return mVirtualDisk->Cd(mCurrentDirectory, FPath("."));
		else if (_params[0] == "cd..")
			return mVirtualDisk->Cd(mCurrentDirectory, FPath(".."));
		else
			printf_s(mVirtualDisk->GetFilePath(mCurrentDirectory).ToString().c_str());
	}
	else
	{
		return mVirtualDisk->Cd(mCurrentDirectory, FPath(_params[1]));
	}

	return E_OK;
}

uint64_t FCommandTool::Copy(const std::vector<std::string> & _params)
{
	if (_params.size() < 3) return E_PARAMETER_ERROR;
	uint64_t pathCount = 0;
	uint64_t yCount = 0;
	bool with_y = false;
	FPath paths[2];

	for (auto it = _params.begin() + 1; it != _params.end(); ++it)
	{
		if (*it == "/y")
		{
			if (yCount != 0) return E_PARAMETER_ERROR;
			with_y = true;
			yCount++;
			continue;
		}
		if (pathCount != 2)
		{
			paths[pathCount] = FPath(*it);
			pathCount++;
		}
		else
		{
			return E_PARAMETER_ERROR;
		}
	}
	if (pathCount != 2) return E_PARAMETER_ERROR;

	return mVirtualDisk->Copy(mCurrentDirectory, with_y, paths[0], paths[1]);
}

uint64_t FCommandTool::Del(const std::vector<std::string> & _params)
{
	std::vector<FPath> paths;
	bool with_s = false;
	for (auto it = _params.begin() + 1; it != _params.end(); ++it)
	{
		if (*it == "/s")
		{
			with_s = true;
			continue;
		}

		paths.push_back(FPath(*it));
	}

	if (paths.size() == 0) return E_PARAMETER_ERROR;

	return mVirtualDisk->Del(mCurrentDirectory, with_s, paths);
}

uint64_t FCommandTool::Rd(const std::vector<std::string> & _params)
{
	std::vector<FPath> paths;
	bool with_s = false;
	for (auto it = _params.begin() + 1; it != _params.end(); ++it)
	{
		if (*it == "/s")
		{
			with_s = true;
			continue;
		}

		paths.push_back(FPath(*it));
	}

	if (paths.size() == 0) return E_PARAMETER_ERROR;

	return mVirtualDisk->Rd(mCurrentDirectory, with_s, paths);
}

uint64_t FCommandTool::Ren(const std::vector<std::string> & _params)
{
	if (_params.size() != 3) return E_PARAMETER_ERROR;
	return mVirtualDisk->Ren(mCurrentDirectory, FPath(_params[1]), _params[2]);
}

uint64_t FCommandTool::Move(const std::vector<std::string> & _params)
{
	if (_params.size() < 3) return E_PARAMETER_ERROR;
	uint64_t pathCount = 0;
	uint64_t yCount = 0;
	bool with_y = false;
	FPath paths[2];

	for (auto it = _params.begin() + 1; it != _params.end(); ++it)
	{
		if (*it == "/y")
		{
			if (yCount != 0) return E_PARAMETER_ERROR;
			with_y = true;
			yCount++;
			continue;
		}
		if (pathCount != 2)
		{
			paths[pathCount] = FPath(*it);
			pathCount++;
		}
		else
		{
			return E_PARAMETER_ERROR;
		}
	}
	if (pathCount != 2) return E_PARAMETER_ERROR;

	return mVirtualDisk->Move(mCurrentDirectory, with_y, paths[0], paths[1]);
}

uint64_t FCommandTool::Mklink(const std::vector<std::string> & _params)
{
	if (_params.size() < 3) return E_PARAMETER_ERROR;
	uint64_t pathCount = 0;
	bool with_d = false;
	FPath paths[2];

	for (auto it = _params.begin() + 1; it != _params.end(); ++it)
	{
		if (*it == "/d")
		{
			with_d = true;
			continue;
		}
		if (pathCount != 2)
		{
			paths[pathCount] = FPath(*it);
			pathCount++;
		}
		else
		{
			return E_PARAMETER_ERROR;
		}
	}

	if (pathCount != 2) return E_PARAMETER_ERROR;

	return mVirtualDisk->Mklink(mCurrentDirectory, with_d, paths[0], paths[1]);
}

uint64_t FCommandTool::Save(const std::vector<std::string> & _params)
{
	if (_params.size() != 2) return E_PARAMETER_ERROR;

	return mVirtualDisk->Save(mCurrentDirectory, FPath(_params[1]));
}

uint64_t FCommandTool::Load(const std::vector<std::string> & _params)
{
	if (_params.size() != 2) return E_PARAMETER_ERROR;

	return mVirtualDisk->Load(mCurrentDirectory, FPath(_params[1]));
}

uint64_t FCommandTool::Cls(const std::vector<std::string> & _params)
{
	if (_params.size() != 1) return E_PARAMETER_ERROR;
	system("cls");
	return E_OK;
}

void FCommandTool::ErrorProcess(uint64_t _errorCode, const std::vector<std::string>& _cmdLines)
{
	printf_s(GetDetail(_errorCode));
	printf_s("\n");
}
