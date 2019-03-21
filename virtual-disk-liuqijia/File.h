#pragma once
#include <string>
#include <vector>
#include "Util.h"

enum class EFileType
{
	CustomFile = 0,
	Directory,
	SymbolLink,
	Num
};

#define SpecificFileType(_fileType)\
	public:\
		static EFileType GetStaticFileType() { return _fileType; }\
		EFileType GetFileType()const override {return GetStaticFileType(); }\
	private:

struct FPath 
{
	FPath() = default;
	FPath(const std::string & _path);
	std::string ToString()const;

	bool mIsEndWithSprit = false;
	bool mIsInVirtualDisk = true;
	std::vector<std::string> mPath;
};

class FDirectory;

class FFile : FNoncopyable
{
private:
	FDirectory * mParentDirectory = nullptr;
	std::string mFileName;

public:
	FFile(FDirectory * _ParentDirectory, const std::string & _fileName);
	virtual EFileType GetFileType()const = 0;
	virtual ~FFile();
};

class FCustomFile : FFile
{
	SpecificFileType(EFileType::CustomFile)

public:
	FCustomFile(FDirectory * _ParentDirectory, const std::string & _fileName, void * _data, uint64_t _dataSize);
	~FCustomFile();
private:
	void * mData = nullptr;
	uint64_t mDataSize = 0;
};

class FDirectory : FFile
{
	SpecificFileType(EFileType::Directory)

public:
	FDirectory(FDirectory * _ParentDirectory, const std::string & _fileName);
	~FDirectory();

	std::vector<FFile*> & GetSonFiles() { return mSonFiles; }
	const std::vector<FFile*> & GetSonFiles()const { return mSonFiles; }

private:
	uint64_t mSonFileDataSize = 0;
	std::vector<FFile*> mSonFiles;
};

class FSymbolLink : FFile
{
	SpecificFileType(EFileType::SymbolLink)

public:
	FSymbolLink(FDirectory * _ParentDirectory, const std::string & _fileName, const FPath & _linkedPath);
	~FSymbolLink();
private:
	FPath mLinkedPath;
};
