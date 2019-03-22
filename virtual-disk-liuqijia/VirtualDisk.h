#pragma once
#include "File.h"
#include <type_traits>

template<typename _t>
struct TIsConstFDirectory {};

template<>
struct TIsConstFDirectory<const FDirectory *> : std::true_type {};

template<>
struct TIsConstFDirectory<FDirectory *> : std::false_type {};

template<>
struct TIsConstFDirectory<std::nullptr_t> : std::false_type {};

template<bool _val, typename _t1, typename _t2>
struct TSwitchType {};

template<typename _t1, typename _t2>
struct TSwitchType<true, _t1, _t2> { using type = _t1; };

template<typename _t1, typename _t2>
struct TSwitchType<false, _t1, _t2> { using type = _t2; };

class FCommandTool;

class FVirtualDisk
{
	FVirtualDisk() = default;
	void __ClearHelper(FDirectory * _node);
	void __DirHelper(const FDirectory * _node, bool _s, bool _ad, std::vector<std::string> & __outFileNames);

	template<typename _directoryRefType>
	std::vector<typename TSwitchType<TIsConstFDirectory<_directoryRefType>::value, const FFile*, FFile*>::type>
		GetFile(_In_opt_ _directoryRefType _directory, const FPath & _path);

	template<typename _directoryRefType>
	typename TSwitchType<TIsConstFDirectory<_directoryRefType>::value, const FDirectory *, FDirectory *>::type
		GetFileParentDirectory(_In_opt_ _directoryRefType _directory, const FPath & _path);

public:
	static FVirtualDisk& Get()
	{
		static FVirtualDisk single;
		return single;
	}

	void Init();
	void Clear();

	void InitCommandTool(FCommandTool * _commandTool, FDirectory *& _currentDirectory);

	void Format();
	bool ContainNode(const FPath & _path, _Out_opt_ uint64_t * _size, _Out_opt_ EFileType * _fileType);
	FPath GetFilePath(const FFile * _file);

#define RegisterFunc(funcName, ...) uint64_t funcName(FDirectory *& _currentPath, __VA_ARGS__)
	RegisterFunc(Dir, bool _s, bool _ad, const FPath & _paths, _Out_ std::vector<std::string> & _outFileNames);
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
	FDirectory * mRoot = nullptr;

};

template<typename _directoryRefType>
std::vector<typename TSwitchType<TIsConstFDirectory<_directoryRefType>::value, const FFile*, FFile*>::type>
FVirtualDisk::GetFile(_In_opt_ _directoryRefType _directory, const FPath & _path)
{
	using FileRefType = typename TSwitchType<TIsConstFDirectory<_directoryRefType>::value, const FFile*, FFile*>::type;
	using DirectoryRefType = typename TSwitchType<TIsConstFDirectory<_directoryRefType>::value, const FDirectory*, FDirectory*>::type;

	DirectoryRefType nowDirectory = GetFileParentDirectory(_directory, _path);

	if (nowDirectory == nullptr) return std::vector<FileRefType>();

	return nowDirectory->SearchSubFile(_path.GetPath().back());
}

template<typename _directoryRefType>
typename TSwitchType<TIsConstFDirectory<_directoryRefType>::value, const FDirectory*, FDirectory*>::type FVirtualDisk::GetFileParentDirectory(_directoryRefType _directory, const FPath & _path)
{
	using FileRefType = typename TSwitchType<TIsConstFDirectory<_directoryRefType>::value, const FFile*, FFile*>::type;
	using DirectoryRefType = typename TSwitchType<TIsConstFDirectory<_directoryRefType>::value, const FDirectory*, FDirectory*>::type;

	DirectoryRefType nowDirectory = _directory;

	if (!_path.IsInVirtualDisk()) return nullptr;
	if (_path.IsAbsolutePath()) nowDirectory = mRoot;
	Assert(nowDirectory);

	for (uint64_t i = 0; i != _path.GetPath().size() - 1; ++i)
	{
		auto files = nowDirectory->SearchSubFile(_path.GetPath()[i]);

		if (files.size() == 0)
			return nullptr;

		auto it = std::find_if(files.begin(), files.end(), [](FileRefType _file) {return _file->GetFileType() == EFileType::Directory; });
		if (it == files.end()) return nullptr;

		nowDirectory = dynamic_cast<DirectoryRefType>(*it);
	}

	return nowDirectory;
}
