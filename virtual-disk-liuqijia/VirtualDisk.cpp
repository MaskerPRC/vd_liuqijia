#include "VirtualDisk.h"
#include <algorithm>
#include "ErrorConstant.h"
#include <fstream>

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

void FVirtualDisk::__DirHelper(const FDirectory * _node, bool _s, bool _ad, std::vector<std::string> & _outFileNames, bool _withWildcard, __in_opt const std::string * _wildcard)
{
	if (!_withWildcard || IsMatch(_node->GetFileName(), *_wildcard))
		_outFileNames.push_back(_node->GetFileName());
	for (auto _ele : _node->GetSubFiles())
	{
		if (_ele->GetFileType() == EFileType::Directory)
		{
			if (_s) __DirHelper(_node, _s, _ad, _outFileNames, _withWildcard, _wildcard);
		}
		else
		{
			if (!_ad && (!_withWildcard || IsMatch(_node->GetFileName(), *_wildcard)))
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

	FDirectory * directory = GetFileParentDirectory(_currentPath, _path);
	if (directory == nullptr) return E_PATH_ERROR;

	for (auto _ele : directory->GetSubFiles())
	{
		if (_ele->GetFileType() == EFileType::Directory)
			__DirHelper(dynamic_cast<FDirectory*>(_ele), _s, _ad, _outFileNames, _path.IsContainsWildcards(), &_path.GetPath().back());
		else if (!_path.IsContainsWildcards() || IsMatch(_ele->GetFileName(), _path.GetPath().back()))
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
			switch (file->GetFileType())
			{
			case EFileType::CustomFile:
				return E_PATH_ERROR;
			case EFileType::Directory:
				nowDirectory = dynamic_cast<FDirectory*>(file);
				break;
			case EFileType::SymbolLink:
			{
				file = GetFinalLinkedFile(dynamic_cast<FSymbolLink*>(file));
				if (file->GetFileType() == EFileType::Directory)
					nowDirectory = dynamic_cast<FDirectory*>(file);
				else
					return E_PATH_ERROR;
			}
			}
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

	if (file != nullptr && file->GetFileType() == EFileType::SymbolLink)
	{
		file = GetFinalLinkedFile(dynamic_cast<FSymbolLink*>(file));
	}

	if (file == nullptr)
		return E_PATH_ERROR;
	else
	{

		if (file->GetFileType() != EFileType::Directory)
			return E_INVALID_DIRECTORY_NAME_ERROR;
		else
		{
			_currentPath = dynamic_cast<FDirectory*>(file);
			return E_OK;
		}
	}
}

uint64_t FVirtualDisk::Copy(FDirectory *& _currentPath, bool _y, const FPath & _sourcePath, const FPath & _destPath)
{
	std::vector<FBlob*> datas;
	std::vector<std::string> fileNames;

	if (_sourcePath.IsInVirtualDisk())
	{
		std::vector<FCustomFile*> sourceFiles;
		std::vector<FFile*> matchFiles = GetFile(_currentPath, _sourcePath);
		if (matchFiles.size() == 0) return E_CANNOT_FIND_SPRCIFIED_FILE_ERROR;

		if (!_sourcePath.IsContainsWildcards())
		{
			FFile * file = matchFiles[0];
			if (file->GetFileType() == EFileType::SymbolLink)
			{
				file = GetFinalLinkedFile(dynamic_cast<FSymbolLink*>(file));
			}

			switch (file->GetFileType())
			{
			case EFileType::CustomFile:
				sourceFiles.push_back(dynamic_cast<FCustomFile*>(file));
				matchFiles.clear();
				break;
			case EFileType::Directory:
				matchFiles = dynamic_cast<FDirectory*>(file)->GetSubFiles();
				break;
			}
		}

		CopyIf(sourceFiles, matchFiles.begin(), matchFiles.end(),
			[&](FFile * _file)
		{
			switch (_file->GetFileType())
			{
			case EFileType::CustomFile:
				return true;
			case EFileType::Directory:
				return false;
			case EFileType::SymbolLink:
			{
				FFile * file = GetFinalLinkedFile(dynamic_cast<FSymbolLink*>(_file));
				if (file == nullptr) return false;
				if (file->GetFileType() != EFileType::CustomFile) return false;
				return true;
			}
			default:
				return false;
			}
		},
			[&](FFile * _file)
		{
			FFile * file = _file;
			if (_file->GetFileType() == EFileType::SymbolLink)
				file = GetFinalLinkedFile(dynamic_cast<FSymbolLink*>(_file));

			return dynamic_cast<FCustomFile*>(_file);
		}
		);

		if (sourceFiles.size() == 0)
			return E_CANNOT_FIND_SPRCIFIED_FILE_ERROR;
		datas.resize(sourceFiles.size());
		fileNames.resize(sourceFiles.size());

		for (uint64_t i = 0; i != datas.size(); ++i)
		{
			datas[i] = sourceFiles[i]->GetBlob();
			fileNames[i] = sourceFiles[i]->GetFileName();
		}
	}
	else
	{
		FILE * fp = nullptr;
		fopen_s(&fp, _sourcePath.ToString(false).c_str(), "rb");
		if (fp == nullptr) return E_CANNOT_FIND_SPRCIFIED_FILE_ERROR;
		fseek(fp, 0, SEEK_END);
		uint64_t fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		FBlob  * newBlob = new FBlob(fileSize);
		fread_s(newBlob->GetBufferPointer(), fileSize, fileSize, 1, fp);
		fclose(fp);
		datas.push_back(newBlob);
		fileNames.push_back(_sourcePath.GetPath().back());
	}

	FDirectory * destDirectory = nullptr;
	FCustomFile * destFile = nullptr;
	if (_destPath.IsInVirtualDisk())
	{
		if (_destPath.IsContainsWildcards())
		{
			destDirectory = GetFileParentDirectory(_currentPath, _destPath);
		}
		else
		{
			auto files = GetFile(_currentPath, _destPath);
			if (files.size() == 0)
			{
				delete datas[0];
				return E_PATH_ERROR;
			}
			FFile * file = files[0];
			if (file->GetFileType() == EFileType::SymbolLink)
			{
				file = GetFinalLinkedFile(file);
			}

			switch (file->GetFileType())
			{
			case EFileType::CustomFile:
				destFile = dynamic_cast<FCustomFile*>(file);
				break;
			case EFileType::Directory:
				destDirectory = dynamic_cast<FDirectory*>(file);
				break;
			default:
				Assert(false);
			}
		}

		if (destDirectory)
		{
			for (uint64_t i = 0; i != datas.size(); ++i)
			{
				if (destDirectory->SearchSubFile(fileNames[i]).size() != 0)
				{
					if (_y) destDirectory->EraseSubFile(fileNames[i]);
					else continue;
				}

				FCustomFile * newFile = new FCustomFile(this, destDirectory, fileNames[i], *datas[i]);
				destDirectory->AddSubFile(newFile);
			}
		}
		else
		{
			uint64_t dataSize = 0;
			for (auto _ele : datas)
				dataSize += _ele->GetBufferSize();
			FBlob newBlob(dataSize);

			uint8_t * buffer = reinterpret_cast<uint8_t*>(newBlob.GetBufferPointer());

			uint64_t offset = 0;
			for (auto _ele : datas)
			{
				memcpy(buffer + offset, _ele->GetBufferPointer(), _ele->GetBufferSize());
				offset += _ele->GetBufferSize();
			}
		}
	}
	else
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
