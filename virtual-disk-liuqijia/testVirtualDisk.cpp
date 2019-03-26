#include "testVirtualDisk.h"
#include "VirtualDisk.h"
#include "CommandTool.h"

VirtualDisk::VirtualDisk()
{
	FVirtualDisk::Get().Init();
	FCommandTool::Get().Init(&FVirtualDisk::Get());
}

bool VirtualDisk::formatDisk()
{
	FVirtualDisk::Get().Format();
	return true;
}

bool VirtualDisk::executeCmd(const std::string & _cmd)
{
	FCommandTool::Get().Exec(_cmd);
	return true;
}

std::string VirtualDisk::getCurPath()
{
	return FVirtualDisk::Get().GetFilePath(FCommandTool::Get().GetCurrentDirectory()).ToString(false);
}

bool VirtualDisk::containNode(std::string _path, int & _size, int & _type)
{
	uint64_t size;
	EFileType type;
	if (FVirtualDisk::Get().ContainNode(FPath(_path), &size, &type))
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

std::string VirtualDisk::getLinkNode(std::string _path)
{
	return FVirtualDisk::Get().getLinkNode(FPath(_path));
}

VirtualDisk::~VirtualDisk()
{
	FCommandTool::Get().Clear();
	FVirtualDisk::Get().Clear();
}
