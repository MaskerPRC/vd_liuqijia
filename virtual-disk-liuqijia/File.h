#pragma once
#include <string>
#include <vector>
#include "Util.h"
#include <algorithm>

enum class EFileType
{
	Unknow = 0,
	CustomFile,
	Directory,
	SymbolLink,
	Num
};

#define SpecificFileType(_fileType)\
	public:\
		static EFileType GetStaticFileType() { return _fileType; }\
		EFileType GetFileType()const override {return GetStaticFileType(); }\
	private:

class FPath
{
private:


public:
	FPath() { mPath.push_back("."); };
	FPath(const std::string & _path);

	template<typename _strIteratorType>
	FPath(bool _isEndWithSprit, bool _isInVirtualDisk, _strIteratorType _begin, _strIteratorType _end)
		:mIsEndWithSprit(_isEndWithSprit), mIsInVirtualDisk(_isInVirtualDisk), mPath(_begin, _end)
	{
		if (mPath.size() == 0) mPath.push_back(".");
		for (const auto & _ele : mPath)
		{
			if (std::find_if(mPath.begin(), mPath.end(), [](char _char) {return _char == '?' || _char == '*'; }) != mPath.end())
			{
				mContainsWildcards = true;
				break;
			}
		}
	}


	std::string ToString()const;
	const std::vector<std::string> & GetPath()const { return mPath; }
	bool IsContainsWildcards()const { return mContainsWildcards; }
	bool IsAbsolutePath()const { return mPath.front() == "C:"; }
	bool IsEndWithSprit()const { return mIsEndWithSprit; }
	bool IsInVirtualDisk()const { return mIsInVirtualDisk; }

private:
	bool mContainsWildcards = false;
	bool mIsEndWithSprit = false;
	bool mIsInVirtualDisk = true;
	std::vector<std::string> mPath;
};

class FBlob
{
public:
	FBlob() = default;
	FBlob(uint64_t _size)
	{
		mData = malloc(_size);
		mDataSize = _size;
	}
	FBlob(void * _data, uint64_t _size)
	{
		mData = malloc(_size);
		mDataSize = _size;
		memcpy(mData, _data, mDataSize);
	}
	FBlob(const FBlob & _val)
	{
		new(this) FBlob(_val.mData, _val.mDataSize);
	}
	FBlob(FBlob && _val)
	{
		mData = _val.mData;
		mDataSize = _val.mDataSize;
		_val.mData = nullptr;
		_val.mDataSize = 0;
	}
	~FBlob()
	{
		free(mData);
		mDataSize = 0;
	}

	const FBlob & operator= (const FBlob & _right)
	{
		this->~FBlob();
		new(this) FBlob(_right);
	}

	const FBlob & operator= (FBlob && _right)
	{
		this->~FBlob();
		new(this) FBlob(_right);
	}

	__forceinline void * GetBufferPointer() { return mData; }
	__forceinline const void * GetBufferPointer()const { return mData; }
	__forceinline uint64_t GetBufferSize()const { return mDataSize; }

private:
	void * mData = nullptr;
	uint64_t mDataSize = 0;
};

class FDirectory;
class FVirtualDisk;

class FFile : FNoncopyable
{
public:
	FFile(FVirtualDisk * _vierualDisk, FDirectory * _parentDirectory, const std::string & _fileName)
		:mVirtualDisk(_vierualDisk), mParentDirectory(_parentDirectory), mFileName(_fileName) {}
	virtual ~FFile() = 0 {};

	std::string GetFileName()const { return mFileName; }
	virtual EFileType GetFileType()const = 0;
	virtual uint64_t GetFileSize()const = 0;
	const FDirectory * GetParentDirectory()const { return mParentDirectory; }
	FDirectory * GetParentDirectory() { return mParentDirectory; }

protected:
	FDirectory * mParentDirectory = nullptr;
	FVirtualDisk * mVirtualDisk = nullptr;
	std::string mFileName;
};

class FCustomFile : public FFile
{
	SpecificFileType(EFileType::CustomFile)

public:
	FCustomFile(FVirtualDisk * _vierualDisk, FDirectory * _parentDirectory, const std::string & _fileName, const FBlob & _blob)
		:FFile(_vierualDisk, _parentDirectory, _fileName), mBlob(_blob) {}
	FCustomFile(FVirtualDisk * _vierualDisk, FDirectory * _parentDirectory, const std::string & _fileName, FBlob && _blob)
		:FFile(_vierualDisk, _parentDirectory, _fileName), mBlob(_blob) {}
	~FCustomFile() {}

	uint64_t GetFileSize()const override { return mBlob.GetBufferSize(); };

	FBlob * GetBlob() { return &mBlob; }
	const FBlob * GetBlob()const { return &mBlob; }

	void ResetBlob(const FBlob & _newBlob) { mBlob = _newBlob; }
	void ResetBlob(FBlob && _newBlob) { mBlob = _newBlob; }

private:
	FBlob mBlob;
};

class FDirectory : public FFile
{
	SpecificFileType(EFileType::Directory)

public:
	FDirectory(FVirtualDisk * _vierualDisk, FDirectory * _parentDirectory, const std::string & _fileName)
		:FFile(_vierualDisk, _parentDirectory, _fileName) {}
	~FDirectory() {}

	std::vector<FFile*> & GetSubFiles() { return mSubFiles; }
	const std::vector<FFile*> & GetSubFiles()const { return mSubFiles; }

	std::vector<FFile *> SearchSubFile(const std::string & _fileName)
	{
		std::vector<FFile *>returned;

		if (_fileName == ".")
			returned.push_back(this);
		if (_fileName == "..")
			returned.push_back(mParentDirectory);

		for (auto _ele : mSubFiles)
		{
			if (IsMatch(_ele->GetFileName(), _fileName))
				returned.push_back(_ele);
		}
		return  returned;
	}

	std::vector<const FFile *> SearchSubFile(const std::string & _fileName)const
	{
		std::vector<const FFile *>returned;

		if (_fileName == ".")
			returned.push_back(this);
		if (_fileName == "..")
			returned.push_back(mParentDirectory);

		for (auto _ele : mSubFiles)
		{
			if (IsMatch(_ele->GetFileName(), _fileName))
				returned.push_back(_ele);
		}
		return  returned;
	}

	uint64_t GetFileSize()const override
	{
		uint64_t returned = 0;
		for (auto _ele : mSubFiles)
			returned += _ele->GetFileSize();
		return returned;
	}

	bool AddSubFile(FFile * _file);
	bool EraseSubFile(const std::string & _fileName);

private:
	std::vector<FFile*> mSubFiles;
};

class FSymbolLink : public FFile
{
	SpecificFileType(EFileType::SymbolLink)

public:
	FSymbolLink(FVirtualDisk * _virtualDisk, FDirectory * _parentDirectory, const std::string & _fileName, const FPath & _linkedPath)
		:FFile(_virtualDisk, _parentDirectory, _fileName), mLinkedPath(_linkedPath) {}
	FSymbolLink(FVirtualDisk * _virtualDisk, FDirectory * _parentDirectory, const std::string & _fileName, const FFile * _linkedPath);
	~FSymbolLink() {}

	const FPath & GetLinkedPath()const { return mLinkedPath; };
	bool ResetLinkedPath(const FPath & _path);

private:
	FPath mLinkedPath;
};
