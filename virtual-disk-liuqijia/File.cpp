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

std::string FPath::ToString() const
{
	std::string returned;
	uint64_t len = 0;
	for (auto _ele : mPath)
		len += _ele.size() + 1;
	len += !mIsInVirtualDisk - !mIsEndWithSprit;
	returned.reserve(len);

	if (!mIsInVirtualDisk)
		returned += '@';
	for (uint64_t i = 0; i != mPath.size() - 1; ++i)
	{
		returned += mPath[i];
		returned += '\\';
	}
	returned += mPath.back();
	if (!mIsEndWithSprit)
		returned += '\\';

	return std::move(returned);
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
	mSubFiles.erase(it);

	return true;
}

FSymbolLink::FSymbolLink(FVirtualDisk * _virtualDisk, FDirectory * _parentDirectory, const std::string & _fileName, const FFile * _linkedPath)
	:FFile(_virtualDisk, _parentDirectory, _fileName), mLinkedPath(_virtualDisk->GetFilePath(_linkedPath))
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
