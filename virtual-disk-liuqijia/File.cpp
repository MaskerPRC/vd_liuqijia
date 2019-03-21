#include "File.h"

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
		if(it == _path.end()) break;

		begin = it;

		for (NULL; it != _path.end() && *it != L'/' && *it != '\\'; ++it);
		mPath.push_back(std::string(begin, it));
	}
	Assert(mPath.size() != 0);
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
