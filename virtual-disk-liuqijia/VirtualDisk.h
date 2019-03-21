#pragma once
#include "File.h"

class FCommandTool;

class FVirtualDisk
{
	FVirtualDisk() = default;
	void __Clear(FDirectory * _node);
public:
	static FVirtualDisk& Get()
	{
		FVirtualDisk single;
		return single;
	}

	void Init();
	void Clear();

	void InitCommandTool(FCommandTool * _commandTool, FDirectory *& _currentDirectory);

	void Format();
	bool ContainNode(FPath & _path, _Out_opt_ uint64_t * _size, _Out_opt_ EFileType _fileType);
	FPath GetFilePath(const FFile & _file);

#define RegisterFunc(funcName, ...) uint64_t funcName(FDirectory *& _currentPath, __VA_ARGS__)
	RegisterFunc(Dir, bool _s, bool _ad, const FPath & _path, _Out_ std::vector<std::string> & _outFileNames);
	RegisterFunc(Md, const FPath & _path);
	RegisterFunc(Cd, const FPath & _path);
	RegisterFunc(Copy, bool _y, const FPath & _sourcePath, const FPath & _destPath);
	RegisterFunc(Del, bool _s, const std::vector<std::string> & _paths);
	RegisterFunc(Rd, bool _s, const std::vector<std::string> & _paths);
	RegisterFunc(Ren, const FPath & _source, const FPath & _dest);
	RegisterFunc(Move, bool _y, const FPath & _source, const FPath & _dest);
	RegisterFunc(Mklink, bool _d, const FPath & _source, const FPath & _dest);
	RegisterFunc(Save, const FPath & _dest);
	RegisterFunc(Load, const FPath & _source);
#undef RegisterFunc

private:
	FDirectory * mRoot;

};
