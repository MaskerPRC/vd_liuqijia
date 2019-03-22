#include "VirtualDisk.h"
#include <algorithm>
#include "ErrorConstant.h"

void FVirtualDisk::__ClearHelper(FDirectory * _node)
{
	for (auto _ele : _node->GetSubFiles())
	{
		switch (_ele->GetFileType())
		{
		case EFileType::CustomFile:
		case EFileType::SymbolLink:
			delete _ele;
			break;
		case EFileType::Directory:
			__ClearHelper(dynamic_cast<FDirectory*>(_ele));
			delete _ele;
			break;
		}
	}
}

void FVirtualDisk::__DirHelper(const FDirectory * _node, bool _s, bool _ad, std::vector<std::string> & _outFileNames)
{
	_outFileNames.push_back(_node->GetFileName());
	for (auto _ele : _node->GetSubFiles())
	{
		if (_ele->GetFileType() == EFileType::Directory)
		{
			if (_s) __DirHelper(_node, _s, _ad, _outFileNames);
		}
		else
		{
			if (!_ad)
				_outFileNames.push_back(_ele->GetFileName());
		}
	}
}

void FVirtualDisk::Init()
{
	Format();
}

void FVirtualDisk::Clear()
{
	if (mRoot)
	{
		__ClearHelper(mRoot);
		delete mRoot;
	}
}

void FVirtualDisk::InitCommandTool(FCommandTool * _commandTool, FDirectory *& _currentDirectory)
{
	_currentDirectory = mRoot;
}

void FVirtualDisk::Format()
{
	Clear();
	mRoot = new FDirectory(this, nullptr, "C:");
}

bool FVirtualDisk::ContainNode(const FPath & _path, uint64_t * _size, EFileType * _fileType)
{
	auto files = GetFile(nullptr, _path);
	if (files.size() == 0)
	{
		if (_size) *_size = 0;
		if (_fileType) *_fileType = EFileType::Unknow;
		return false;
	}

	if (_size) *_size = files[0]->GetFileSize();
	if (_fileType) *_fileType = files[0]->GetFileType();
	return true;
}

FPath FVirtualDisk::GetFilePath(const FFile * _file)
{
	const FFile * nowFile = _file;
	std::string path = _file->GetFileName();
	nowFile = nowFile->GetParentDirectory();

	while (nowFile != nullptr)
	{
		path = nowFile->GetFileName() + '\\' + path;
		nowFile = nowFile->GetParentDirectory();
	}

	return FPath(path);
}

uint64_t FVirtualDisk::Dir(FDirectory *& _currentPath, bool _s, bool _ad, const FPath & _path, _Out_ std::vector<std::string> & _outFileNames)
{
	if (!_path.IsInVirtualDisk()) return E_NOT_IN_VIRTUAL_DISK_ERROR;

	auto files = GetFile(_currentPath, _path);
	if (files.size() == 0) return E_PATH_ERROR;
	_outFileNames.resize(0);

	for (auto _ele : files)
	{
		if (_ele->GetFileType() == EFileType::Directory)
			__DirHelper(dynamic_cast<FDirectory*>(_ele), _s, _ad, _outFileNames);
		else
			_outFileNames.push_back(_ele->GetFileName());
	}

	return E_OK;
}

uint64_t FVirtualDisk::Md(FDirectory *& _currentPath, const FPath & _path)
{
	if (!_path.IsInVirtualDisk()) return E_NOT_IN_VIRTUAL_DISK_ERROR;
	if (_path.IsContainsWildcards()) return E_INAPPROPRIATE_CONTAIN_WILDCARDS_ERROR;

	FDirectory * nowDirectory = _path.IsAbsolutePath() ? mRoot : _currentPath;
	for (uint64_t i = 0; i != _path.GetPath().size() - 1; ++i)
	{
		auto files = nowDirectory->SearchSubFile(_path.GetPath()[i]);
		FFile * file = files.size() != 0 ? files[0] : nullptr;
		if (file == nullptr)
		{
			FDirectory * newDir = new FDirectory(this, nowDirectory, _path.GetPath()[i]);
			nowDirectory->AddSubFile(newDir);
			nowDirectory = newDir;
		}
		else
		{
			if (file->GetFileType() != EFileType::Directory) return E_PATH_ERROR;
			else nowDirectory = dynamic_cast<FDirectory*>(file);
		}
	}

	if (nowDirectory->SearchSubFile(_path.GetPath().back()).size() == 0)
	{
		FDirectory * newDir = new FDirectory(this, nowDirectory, _path.GetPath().back());
		nowDirectory->AddSubFile(newDir);
		return E_OK;
	}
	else
	{
		return E_SUBDIRECTORY_OR_FILE_ALREADY_EXIST_ERROR;
	}
}

uint64_t FVirtualDisk::Cd(FDirectory *& _currentPath, const FPath & _path)
{
	if (!_path.IsInVirtualDisk()) return E_NOT_IN_VIRTUAL_DISK_ERROR;

	auto files = GetFile(_currentPath, _path);
	FFile * file = files.size() != 0 ? files[0] : nullptr;

	if (file == nullptr)
		return E_PATH_ERROR;
	else if (file->GetFileType() != EFileType::Directory)
		return E_INVALID_DIRECTORY_NAME_ERROR;
	else
	{
		_currentPath = dynamic_cast<FDirectory*>(file);
		return E_OK;
	}
}

uint64_t FVirtualDisk::Copy(FDirectory *& _currentPath, bool _y, const FPath & _sourcePath, const FPath & _destPath)
{
	FBlob data;
	if (_sourcePath.IsInVirtualDisk())
	{
	}
	return E_PARAMETER_ERROR;
}

uint64_t FVirtualDisk::Del(FDirectory *& _currentPath, bool _s, const std::vector<std::string> & _paths)
{
	return E_PARAMETER_ERROR;
}

uint64_t FVirtualDisk::Rd(FDirectory *& _currentPath, bool _s, const std::vector<std::string> & _paths)
{
	return E_PARAMETER_ERROR;
}

uint64_t FVirtualDisk::Ren(FDirectory *& _currentPath, const FPath & _source, const FPath & _dest)
{
	return E_PARAMETER_ERROR;
}

uint64_t FVirtualDisk::Move(FDirectory *& _currentPath, bool _y, const FPath & _source, const FPath & _dest)
{
	return E_PARAMETER_ERROR;
}

uint64_t FVirtualDisk::Mklink(FDirectory *& _currentPath, bool _d, const FPath & _source, const FPath & _dest)
{
	return E_PARAMETER_ERROR;
}

uint64_t FVirtualDisk::Save(FDirectory *& _currentPath, const FPath & _dest)
{
	return E_PARAMETER_ERROR;
}

uint64_t FVirtualDisk::Load(FDirectory *& _currentPath, const FPath & _source)
{
	return E_PARAMETER_ERROR;
}
