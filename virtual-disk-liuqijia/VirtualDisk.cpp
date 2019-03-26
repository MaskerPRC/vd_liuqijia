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

void FVirtualDisk::__DelHelper(FDirectory * _node, bool _s, bool _withWildcard, const std::string * _wildcard)
{
	std::vector<std::string> fileNames;
	for (auto it = _node->GetSubFiles().begin(); it != _node->GetSubFiles().end(); ++it)
	{
		switch ((*it)->GetFileType())
		{
		case EFileType::CustomFile:
		case EFileType::SymbolLink:
			if (!_withWildcard || IsMatch((*it)->GetFileName(), *_wildcard))
				fileNames.push_back((*it)->GetFileName());
			break;
		case EFileType::Directory:
			if (_s)
				__DelHelper(dynamic_cast<FDirectory*>(*it), _s, _withWildcard, _wildcard);
		}
	}

	for (const auto & _ele : fileNames)
		_node->EraseSubFile(_ele);

}

void FVirtualDisk::__RdHelper(FDirectory * _node)
{
	std::vector<std::string> fileNames;
	for (auto it = _node->GetSubFiles().begin(); it != _node->GetSubFiles().end(); ++it)
	{
		if ((*it)->GetFileType() == EFileType::Directory)
			__RdHelper(dynamic_cast<FDirectory*>(*it));

		fileNames.push_back((*it)->GetFileName());
	}

	for (const auto & _ele : fileNames)
		_node->EraseSubFile(_ele);
}

void FVirtualDisk::__BuildTopology(FDirectory * _node, uint64_t _parrentIndex, std::vector<uint64_t>& _treeArray)
{
	for (auto _ele : _node->GetSubFiles())
	{
		if (_ele->GetFileType() == EFileType::Directory)
		{
			_treeArray.push_back(_parrentIndex);
			__BuildTopology(dynamic_cast<FDirectory*>(_ele), _treeArray.size() - 1, _treeArray);
		}
		else
		{
			_treeArray.push_back(_parrentIndex);
		}
	}
}

void FVirtualDisk::__SaveHelper(FDirectory * _node, std::ofstream & _ofs)
{
	EFileType fileType = EFileType::Unknow;

	for (auto _ele : _node->GetSubFiles())
	{
		fileType = _ele->GetFileType();
		_ofs.write(reinterpret_cast<const char *>(&fileType), sizeof(EFileType));
		_ele->SaveToFile(_ofs);
		if (_ele->GetFileType() == EFileType::Directory)
			__SaveHelper(dynamic_cast<FDirectory*>(_ele), _ofs);
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
		mRoot = nullptr;
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

std::string FVirtualDisk::getLinkNode(const FPath & _path)
{
	auto files = GetFile(nullptr, _path);
	if (files.size() == 0 || files[0]->GetFileType() != EFileType::SymbolLink)
		return std::string();
	FSymbolLink * symbolFile = dynamic_cast<FSymbolLink*>(files[0]);
	if (ContainNode(symbolFile->GetLinkedPath(), nullptr, nullptr) == false)
		return std::string();
	else return symbolFile->GetLinkedPath().ToString(false);
}

uint64_t FVirtualDisk::Dir(FDirectory *& _currentDir, bool _s, bool _ad, const FPath & _path, _Out_ std::vector<std::string> & _outFileNames)
{
	if (!_path.IsInVirtualDisk()) return E_NOT_IN_VIRTUAL_DISK_ERROR;

	FDirectory * directory = GetFileParentDirectory(_currentDir, _path);
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

uint64_t FVirtualDisk::Md(FDirectory *& _currentDir, const FPath & _path)
{
	if (!_path.IsInVirtualDisk()) return E_NOT_IN_VIRTUAL_DISK_ERROR;
	if (_path.IsContainsWildcards()) return E_INAPPROPRIATE_CONTAIN_WILDCARDS_ERROR;

	FDirectory * nowDirectory = _path.IsAbsolutePath() ? mRoot : _currentDir;
	for (uint64_t i = _path.IsAbsolutePath() ? 1 : 0; i != _path.GetPath().size() - 1; ++i)
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

uint64_t FVirtualDisk::Cd(FDirectory *& _currentDir, FPath & _currentPath, const FPath & _path)
{
	if (!_path.IsInVirtualDisk()) return E_NOT_IN_VIRTUAL_DISK_ERROR;

	auto files = GetFile(_currentDir, _path);
	FFile * sourcefile = files.size() != 0 ? files[0] : nullptr;
	FFile * file = sourcefile;

	if (sourcefile != nullptr && sourcefile->GetFileType() == EFileType::SymbolLink)
	{
		file = GetFinalLinkedFile(dynamic_cast<FSymbolLink*>(sourcefile));
	}

	if (file == nullptr)
		return E_PATH_ERROR;
	else
	{

		if (file->GetFileType() != EFileType::Directory)
			return E_INVALID_DIRECTORY_NAME_ERROR;
		else
		{
			if (_path.IsAbsolutePath())
			{
				auto destPath = _path.GetPath();
				std::vector<std::string> currentPath(destPath.begin(), destPath.end() - 1);
				currentPath.push_back(sourcefile->GetFileName());
				_currentPath = FPath(false, true, currentPath.begin(), currentPath.end());
			}
			else
			{
				auto destPath = _path.GetPath();
				auto currentPath = _currentPath.GetPath();

				for (uint64_t i = 0; i != destPath.size() - (_path.IsContainsWildcards() ? 1 : 0); ++i)
				{
					if (destPath[i] == ".") continue;
					if (destPath[i] == "..")
					{
						currentPath.pop_back();
						continue;
					}
					currentPath.push_back(destPath[i]);
				}
				if (_path.IsContainsWildcards())
					currentPath.push_back(sourcefile->GetFileName());

				_currentPath = FPath(false, true, currentPath.begin(), currentPath.end());
			}
			_currentDir = dynamic_cast<FDirectory*>(file);
			return E_OK;
		}
	}
}

uint64_t FVirtualDisk::Copy(FDirectory *& _currentDir, bool _y, const FPath & _sourcePath, const FPath & _destPath)
{
	std::vector<FBlob> datas;
	std::vector<std::string> fileNames;

	if (_sourcePath.IsInVirtualDisk())
	{
		std::vector<FCustomFile*> sourceFiles;
		std::vector<FFile*> matchFiles = GetFile(_currentDir, _sourcePath);
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
			datas[i] = *sourceFiles[i]->GetBlob();
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
		FBlob newBlob(fileSize);
		fread_s(newBlob.GetBufferPointer(), fileSize, fileSize, 1, fp);
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
			destDirectory = GetFileParentDirectory(_currentDir, _destPath);
			if (destDirectory == nullptr)
			{
				return E_PATH_ERROR;
			}
		}
		else
		{
			FDirectory * parrentDirectory = GetFileParentDirectory(_currentDir, _destPath);
			if (parrentDirectory == nullptr)
			{
				return E_PATH_ERROR;
			}
			auto files = parrentDirectory->SearchSubFile(_destPath.GetPath().back());
			if (_destPath.IsAbsolutePath() && _destPath.GetPath().size() == 1)
			{
				destDirectory = mRoot;
			}
			else if (files.size() == 0)
			{
				destFile = new FCustomFile(this, parrentDirectory, _destPath.GetPath().back(), FBlob());
				parrentDirectory->AddSubFile(destFile);
			}
			else
			{
				FFile * file = files[0];
				if (file->GetFileType() == EFileType::SymbolLink)
				{
					file = GetFinalLinkedFile(dynamic_cast<FSymbolLink*>(file));
					if (file == nullptr)
					{
						return E_PATH_ERROR;
					}
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

				FCustomFile * newFile = new FCustomFile(this, destDirectory, fileNames[i], std::move(datas[i]));
				destDirectory->AddSubFile(newFile);
			}
		}
		else
		{
			uint64_t dataSize = 0;
			for (auto & _ele : datas)
				dataSize += _ele.GetBufferSize();
			FBlob newBlob(dataSize);

			uint8_t * buffer = reinterpret_cast<uint8_t*>(newBlob.GetBufferPointer());

			uint64_t offset = 0;
			for (auto & _ele : datas)
			{
				memcpy(buffer + offset, _ele.GetBufferPointer(), _ele.GetBufferSize());
				offset += _ele.GetBufferSize();
			}

			destFile->ResetBlob(std::move(newBlob));

		}
	}
	else
	{
		FILE * fp = nullptr;
		fopen_s(&fp, _destPath.ToString(false).c_str(), "wb");
		if (fp == nullptr)
		{

			return E_PATH_ERROR;
		}
		for (auto _ele : datas)
		{
			fwrite(_ele.GetBufferPointer(), 1, _ele.GetBufferSize(), fp);
		}
		fclose(fp);
	}

	return E_OK;
}

uint64_t FVirtualDisk::Del(FDirectory *& _currentDir, bool _s, const std::vector<FPath> & _paths)
{
	uint64_t returned = E_OK;
	for (const auto & _path : _paths)
	{
		if (_path.IsContainsWildcards())
		{
			FDirectory * directory = GetFileParentDirectory(_currentDir, _path);
			if (directory == nullptr)
			{
				returned = E_PATH_ERROR;
				continue;
			}
			__DelHelper(directory, _s, true, &_path.GetPath().back());
		}
		else
		{
			auto files = GetFile(_currentDir, _path);
			if (files.size() == 0)
			{
				returned = E_PATH_ERROR;
				continue;
			}

			FFile * file = files[0];

			if (file->GetFileType() == EFileType::SymbolLink)
			{
				FFile * linkedFile = GetFinalLinkedFile(dynamic_cast<FSymbolLink*>(file));
				if (linkedFile != nullptr)
					file = linkedFile->GetFileType() == EFileType::Directory ? linkedFile : file;
			}

			switch (file->GetFileType())
			{
			case EFileType::SymbolLink:
			case EFileType::CustomFile:
			{
				FDirectory * parrentDirectory = file->GetParentDirectory();
				parrentDirectory->EraseSubFile(file->GetFileName());
				break;
			}
			case EFileType::Directory:
				__DelHelper(dynamic_cast<FDirectory*>(file), _s, false, nullptr);
				break;
			}
		}
	}
	return returned;
}

uint64_t FVirtualDisk::Rd(FDirectory *& _currentDir, bool _s, const std::vector<FPath> & _paths)
{
	uint64_t returned = E_OK;
	for (const auto & _path : _paths)
	{
		auto files = GetFile(_currentDir, _path);
		if (files.size() == 0)
		{
			returned = E_PATH_ERROR;
			continue;
		}
		FFile * file = files[0];
		FDirectory * parrentDirectory = file->GetParentDirectory();
		switch (file->GetFileType())
		{
		case EFileType::CustomFile:
			returned = E_NOT_DIRECTORY_ERROR;
			continue;
		case EFileType::SymbolLink:
			parrentDirectory->EraseSubFile(file->GetFileName());
			continue;
		}

		if (file == mRoot)
		{
			returned = E_CANNOT_FIND_SPRCIFIED_FILE_ERROR;
			continue;
		}

		if (!_s && dynamic_cast<FDirectory*>(file)->GetSubFiles().size() != 0)
		{
			returned = E_FOLDER_IS_NOT_EMPTY_ERROR;
			continue;
		}

		__RdHelper(dynamic_cast<FDirectory*>(file));
		parrentDirectory->EraseSubFile(file->GetFileName());
	}

	return returned;
}

uint64_t FVirtualDisk::Ren(FDirectory *& _currentDir, const FPath & _source, const std::string & _dest)
{
	if (!CheckFileName(_dest)) return E_WRONG_FILE_NAME_ERROR;
	FDirectory * destDirectory = GetFileParentDirectory(_currentDir, _source);
	if (destDirectory == nullptr) return E_PATH_ERROR;
	if (destDirectory->SearchSubFile(_dest).size() != 0)  return E_EXIT_FILE_WITH_SAME_NAME_ERROR;

	auto files = destDirectory->SearchSubFile(_source.GetPath().back());
	if (files.size() == 0) return E_PATH_ERROR;

	FFile * file = files[0];

	file->ResetFileName(_dest);

	return E_OK;
}

uint64_t FVirtualDisk::Move(FDirectory *& _currentDir, bool _y, const FPath & _source, const FPath & _dest)
{
	uint64_t errorCode = E_OK;

	std::vector<FFile*> toBeMovedFiles;
	FDirectory * sourceDirectory = nullptr;
	if (_source.IsContainsWildcards())
	{
		sourceDirectory = GetFileParentDirectory(_currentDir, _dest);
		toBeMovedFiles = sourceDirectory->SearchSubFile(_source.GetPath().back());
	}
	else
	{
		auto files = GetFile(_currentDir, _source);
		if (files.size() == 0) return E_PATH_ERROR;
		toBeMovedFiles.push_back(files[0]);
		sourceDirectory = files[0]->GetParentDirectory();
	}

	if (toBeMovedFiles.size() == 0) return E_PATH_ERROR;

	FDirectory * destDirectory = nullptr;
	std::string destFileName;

	if (_dest.IsContainsWildcards())
	{
		destDirectory = GetFileParentDirectory(_currentDir, _dest);
		if (destDirectory = nullptr) return E_PATH_ERROR;
	}
	else
	{
		auto files = GetFile(_currentDir, _dest);
		if (files.size() != 0)
		{
			FFile * file = files[0];
			if (file->GetFileType() == EFileType::SymbolLink)
			{
				auto linkedFile = GetFinalLinkedFile(dynamic_cast<FSymbolLink*>(file));
				if (linkedFile->GetFileType() == EFileType::Directory)
					destDirectory = dynamic_cast<FDirectory*>(linkedFile);
				else
				{
					destFileName = file->GetFileName();
					destDirectory = file->GetParentDirectory();
				}
			}
			else if (file->GetFileType() == EFileType::CustomFile)
			{
				destFileName = file->GetFileName();
				destDirectory = file->GetParentDirectory();
			}
			else if (file->GetFileType() == EFileType::Directory)
			{
				destDirectory = dynamic_cast<FDirectory*>(file);
			}
		}
		else
		{
			FDirectory * directory = GetFileParentDirectory(_currentDir, _dest);
			if (directory == nullptr) return E_PATH_ERROR;
			destFileName = _dest.GetPath().back();
			destDirectory = directory;
		}
	}

	for (auto _ele : toBeMovedFiles)
	{
		auto files = destDirectory->SearchSubFile(_ele->GetFileName());
		if (files.size() != 0)
		{
			FFile * file = files[0];
			if (file->GetFileType() == EFileType::Directory)
			{
				errorCode = E_ACCESS_FAILURE_ERROR;
				continue;
			}
			if (!_y) continue;
			destDirectory->EraseSubFile(file->GetFileName());
		}

		std::string * sp = destFileName.size() == 0 ? nullptr : &destFileName;

		FFile * newFile = nullptr;
		switch (_ele->GetFileType())
		{
		case EFileType::CustomFile:
			newFile = new FCustomFile(this, destDirectory, std::move(*dynamic_cast<FCustomFile*>(_ele)), sp);
			break;
		case EFileType::Directory:
			newFile = new FDirectory(this, destDirectory, std::move(*dynamic_cast<FDirectory*>(_ele)), sp);
			break;
		case EFileType::SymbolLink:
			newFile = new FSymbolLink(this, destDirectory, std::move(*dynamic_cast<FSymbolLink*>(_ele)), sp);
			break;
		}
		_ele->GetParentDirectory()->EraseSubFile(_ele->GetFileName());
		destDirectory->AddSubFile(newFile);
	}

	return errorCode;
}

uint64_t FVirtualDisk::Mklink(FDirectory *& _currentDir, bool _d, const FPath & _source, const FPath & _dest)
{
	if (_source.IsContainsWildcards() || _dest.IsContainsWildcards()) return E_WILDCARDS_APPEAR_IN_THE_PATH_ERROR;

	FDirectory * symbolDestDir = GetFileParentDirectory(_currentDir, _source);
	if (symbolDestDir == nullptr) return E_PATH_ERROR;

	auto files = GetFile(_currentDir, _dest);
	if (files.size() != 1) return E_CANNOT_FIND_SPRCIFIED_FILE_ERROR;

	FSymbolLink * symbolLinkFile = new FSymbolLink(this, symbolDestDir, _source.GetPath().back(), GetFilePath(files[0]));
	symbolDestDir->AddSubFile(symbolLinkFile);
	return E_OK;
}

uint64_t FVirtualDisk::Save(FDirectory *& _currentDir, const FPath & _dest)
{
	std::vector<uint64_t> tree;
	tree.push_back(UINT64_MAX);
	__BuildTopology(mRoot, 0, tree);

	std::ofstream ofs(_dest.ToString(false), std::ios::out | std::ios::binary);
	if (ofs.is_open() == false) return E_ACCESS_FAILURE_ERROR;

	uint64_t magicNum = 1014931172;
	ofs.write(reinterpret_cast<const char*>(&magicNum), sizeof(uint64_t));

	uint64_t fileCount = tree.size();
	ofs.write(reinterpret_cast<const char *>(&fileCount), sizeof(uint64_t));
	ofs.write(reinterpret_cast<const char *>(tree.data()), sizeof(uint64_t) * fileCount);

	EFileType fileType = EFileType::Directory;
	ofs.write(reinterpret_cast<const char *>(&fileType), sizeof(EFileType));
	mRoot->SaveToFile(ofs);
	__SaveHelper(mRoot, ofs);
	ofs.close();

	return E_OK;
}

uint64_t FVirtualDisk::Load(FDirectory *& _currentDir, const FPath & _source)
{
	std::vector<uint64_t> tree;
	std::vector<FFile*> files;

	std::ifstream ifs(_source.ToString(false), std::ios::in | std::ios::binary);
	if (ifs.is_open() == false) return E_ACCESS_FAILURE_ERROR;

	uint64_t magicNum = 0;
	ifs.read(reinterpret_cast<char*>(&magicNum), sizeof(uint64_t));
	if (magicNum != 1014931172) return E_NOT_VALID_VIRTUAL_DISK_FILE_ERROR;
	uint64_t fileCount = 0;
	ifs.read(reinterpret_cast<char*>(&fileCount), sizeof(uint64_t));
	tree.resize(fileCount);
	ifs.read(reinterpret_cast<char*>(&tree[0]), sizeof(uint64_t) * fileCount);

	{
		EFileType fileType = EFileType::Unknow;
		ifs.read(reinterpret_cast<char *>(&fileType), sizeof(EFileType));
		Assert(fileType == EFileType::Directory);
		mRoot = new FDirectory(this, nullptr, std::string());
		mRoot->LoadFromFile(ifs);
	}

	files.resize(fileCount, nullptr);
	files[0] = mRoot;

	for (uint64_t i = 1; i != fileCount; ++i)
	{
		EFileType fileType = EFileType::Unknow;
		ifs.read(reinterpret_cast<char *>(&fileType), sizeof(EFileType));
		switch (fileType)
		{
		case EFileType::CustomFile:
			files[i] = new FCustomFile(this, dynamic_cast<FDirectory*>(files[tree[i]]), std::string(), FBlob());
			break;
		case EFileType::Directory:
			files[i] = new FDirectory(this, dynamic_cast<FDirectory*>(files[tree[i]]), std::string());
			break;
		case EFileType::SymbolLink:
			files[i] = new FSymbolLink(this, dynamic_cast<FDirectory*>(files[tree[i]]), std::string(), FPath());
			break;
		default:
			Assert(false);
		}
		files[i]->LoadFromFile(ifs);
		dynamic_cast<FDirectory*>(files[tree[i]])->AddSubFile(files[i]);
	}
	return E_OK;
}
