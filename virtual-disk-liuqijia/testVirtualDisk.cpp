#include "testVirtualDisk.h"
#include "VirtualDisk.h"
#include "CommandTool.h"

VirtualDisk::VirtualDisk()
{
	//FVirtualDisk::Get().Init();
	//FCommandTool::Get().Init(&FVirtualDisk::Get());
}

bool VirtualDisk::formatDisk()
{
	FCommandTool::Get().Clear();
	FVirtualDisk::Get().Clear();
	FVirtualDisk::Get().Init();
	FCommandTool::Get().Init(&FVirtualDisk::Get());
	return true;
}

bool VirtualDisk::executeCmd(const std::string & _cmd)
{
	FCommandTool::Get().Exec(_cmd);
	return true;
}

std::string VirtualDisk::getCurPath()
{
	return FCommandTool::Get().GetCurrentPath().ToString(false);
}

bool VirtualDisk::containNode(std::string _path, int & _size, int & _type)
{
	FPath path(_path);

	if (path.IsAbsolutePath())
	{
		uint64_t size;
		EFileType type;
		if (FVirtualDisk::Get().ContainNode(path, &size, &type))
		{
			_size = size;
			switch (type)
			{
			case EFileType::CustomFile:
				_type = 2;
				break;
			case EFileType::Directory:
				_type = 1;
				break;
			case EFileType::SymbolLink:
				_type = 3;
				break;
			}
			return true;
		}
		else
		{
			_size = -1;
			_type = 0;
			return false;
		}
	}
	else
	{
		FILE * fp = nullptr;
		fopen_s(&fp, path.ToString(false).c_str(), "rb");
		if (fp == nullptr)
		{
			_size = -1;
			_type = 0;
			return false;
		}

		fseek(fp, 0, SEEK_END);
		_size = ftell(fp);
		fclose(fp);
		_type = 2;
		return true;
	}
}

std::string VirtualDisk::getLinkNode(std::string _path)
{
	return FVirtualDisk::Get().getLinkNode(FPath(_path));
}

VirtualDisk::~VirtualDisk()
{
	//FCommandTool::Get().Clear();
	//FVirtualDisk::Get().Clear();
}
