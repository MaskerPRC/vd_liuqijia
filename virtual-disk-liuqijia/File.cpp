#include "File.h"
#include "VirtualDisk.h"
#include <algorithm>

FPath::FPath(const std::string & _path)
{
	auto it = _path.begin();
	if (*it == '@')
	{
		mIsInVirtualDisk = false;
		++it;
	}

	if (_path.back() == '\\' || _path.back() == '/')
		mIsEndWithSprit = true;

	for (auto begin = it; it != _path.end(); NULL)
	{
		for (NULL; it != _path.end() && (*it == L'/' || *it == '\\'); ++it);
		if (it == _path.end()) break;

		begin = it;

		for (NULL; it != _path.end() && *it != L'/' && *it != '\\'; ++it);
		mPath.push_back(std::string(begin, it));
	}

	mContainsWildcards = std::find_if(_path.begin(), _path.end(), [](char _char) {return _char == '?' || _char == '*'; }) != _path.end();

	if (mPath.size() == 0)
		mPath.push_back(".");
}

std::string FPath::ToString(bool _couldWithAt) const
{
	std::string returned;
	uint64_t len = 0;
	for (auto _ele : mPath)
		len += _ele.size() + 1;
	len += !mIsInVirtualDisk - !mIsEndWithSprit;
	returned.reserve(len);

	if (_couldWithAt && !mIsInVirtualDisk)
		returned += '@';
	for (uint64_t i = 0; i != mPath.size() - 1; ++i)
	{
		returned += mPath[i];
		returned += '\\';
	}
	returned += mPath.back();
	if (mIsEndWithSprit || (IsAbsolutePath() && mPath.size() == 1) )
		returned += '\\';

	return std::move(returned);
}

FCustomFile::FCustomFile(FVirtualDisk * _virtualDisk, FDirectory * _parentDirectory, FCustomFile && _file, _In_opt_  const std::string * _str)
	:FFile(_virtualDisk, _parentDirectory, _str ? *_str : _file.GetFileName()), mBlob(std::move(_file.mBlob))
{
}

void FCustomFile::SaveToFile(std::ofstream & _ofs) const
{
	SaveStringToFile(_ofs, mFileName);
	uint64_t blobSize = mBlob.GetBufferSize();
	_ofs.write(reinterpret_cast<const char *>(&blobSize), sizeof(uint64_t));
	if (blobSize != 0)
		_ofs.write(reinterpret_cast<const char *>(mBlob.GetBufferPointer()), blobSize);
}

void FCustomFile::LoadFromFile(std::ifstream & _ifs)
{
	LoadStringFromFile(_ifs, mFileName);
	uint64_t blobSize = 0;
	_ifs.read(reinterpret_cast<char *>(&blobSize), sizeof(uint64_t));
	FBlob blob(blobSize);
	if (blobSize != 0)
		_ifs.read(reinterpret_cast<char *>(blob.GetBufferPointer()), blobSize);
	mBlob = std::move(blobSize);
}

FDirectory::FDirectory(FVirtualDisk * _virtualDisk, FDirectory * _parentDirectory, FDirectory && _directory, _In_opt_  const std::string * _str)
	: FFile(_virtualDisk, _parentDirectory, _str ? *_str : _directory.GetFileName()), mSubFiles(std::move(_directory.mSubFiles))
{
}

bool FDirectory::AddSubFile(FFile * _file)
{
	auto it = std::find_if(mSubFiles.begin(), mSubFiles.end(), [&](FFile * _val) {return _val->GetFileName() == _file->GetFileName(); });
	if (it != mSubFiles.end()) return false;

	mSubFiles.push_back(_file);
	return true;
}

bool FDirectory::EraseSubFile(const std::string & _fileName)
{
	auto it = std::find_if(mSubFiles.begin(), mSubFiles.end(), [&](FFile * _file) {return _file->GetFileName() == _fileName; });

	if (it == mSubFiles.end()) return false;
	delete *it;
	mSubFiles.erase(it);

	return true;
}

void FDirectory::SaveToFile(std::ofstream & _ofs) const
{
	SaveStringToFile(_ofs, mFileName);
}

void FDirectory::LoadFromFile(std::ifstream & _ifs)
{
	LoadStringFromFile(_ifs, mFileName);
}

FSymbolLink::FSymbolLink(FVirtualDisk * _virtualDisk, FDirectory * _parentDirectory, const std::string & _fileName, const FFile * _linkedPath)
	:FFile(_virtualDisk, _parentDirectory, _fileName), mLinkedPath(_virtualDisk->GetFilePath(_linkedPath))
{
}

FSymbolLink::FSymbolLink(FVirtualDisk * _virtualDisk, FDirectory * _parentDirectory, FSymbolLink && _symbolLink, _In_opt_  const std::string * _str)
	: FFile(_virtualDisk, _parentDirectory, _str ? *_str : _symbolLink.GetFileName()), mLinkedPath(std::move(_symbolLink.mLinkedPath))
{
}

bool FSymbolLink::ResetLinkedPath(const FPath & _path)
{
	if (mVirtualDisk->ContainNode(_path, nullptr, nullptr))
	{
		mLinkedPath = _path;
		return true;
	}
	return false;
}

void FSymbolLink::SaveToFile(std::ofstream & _ofs) const
{
	SaveStringToFile(_ofs, mFileName);
	SaveStringToFile(_ofs, mLinkedPath.ToString());
}

void FSymbolLink::LoadFromFile(std::ifstream & _ifs)
{
	LoadStringFromFile(_ifs, mFileName);
	std::string linkedPath;
	LoadStringFromFile(_ifs, linkedPath);
	mLinkedPath = FPath(linkedPath);
}
