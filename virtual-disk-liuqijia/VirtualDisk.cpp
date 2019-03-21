#include "VirtualDisk.h"

void FVirtualDisk::__Clear(FDirectory * _node)
{
	for (auto _ele : _node->GetSonFiles())
	{
		switch (_ele->GetFileType())
		{
		case EFileType::CustomFile:
		case EFileType::SymbolLink:
			delete _ele;
			break;
		case EFileType::Directory:
			__Clear(dynamic_cast<FDirectory*>(_ele));
		}
	}
}

void FVirtualDisk::Init()
{
	Format();
}

void FVirtualDisk::Clear()
{
	__Clear(mRoot);
	delete mRoot;
}

void FVirtualDisk::InitCommandTool(FCommandTool * _commandTool, FDirectory *& _currentDirectory)
{
	_currentDirectory = mRoot;
}
