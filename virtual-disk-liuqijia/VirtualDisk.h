#pragma once
#include "File.h"
#include <type_traits>

template<typename _t>
struct TIsConstPointer {};

template<typename _t>
struct TIsConstPointer<const _t *> : std::true_type {};

template<typename _t>
struct TIsConstPointer<_t *> : std::false_type {};

template<>
struct TIsConstPointer<std::nullptr_t> : std::false_type {};

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
	void __DirHelper(const FDirectory * _node, bool _s, bool _ad, std::vector<std::string> & _outFileNames, bool _withWildcard, __in_opt const std::string * _wildcard);
	void __DelHelper(FDirectory * _node, bool _s, bool _withWildcard, __in_opt const std::string * _wildcard);
	void __RdHelper(FDirectory * _node);
	void __BuildTopology(FDirectory * _node, uint64_t _parrentIndex, std::vector<uint64_t>& _treeArray);
	void __SaveHelper(FDirectory * _node, std::ofstream & _ofs);

	template<typename _directoryRefType>
	std::vector<typename TSwitchType<TIsConstPointer<_directoryRefType>::value, const FFile*, FFile*>::type>
		GetFile(_In_opt_ _directoryRefType _directory, const FPath & _path);

	template<typename _directoryRefType>
	typename TSwitchType<TIsConstPointer<_directoryRefType>::value, const FDirectory *, FDirectory *>::type
		GetFileParentDirectory(_In_opt_ _directoryRefType _directory, const FPath & _path);

	template<typename _symbolLinkRefType>
	typename TSwitchType<TIsConstPointer<_symbolLinkRefType>::value, const FFile *, FFile *>::type
		GetFinalLinkedFile(_symbolLinkRefType _symbolLink);

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

	Comment(for test) std::string getLinkNode(const FPath & _path);

#define RegisterFunc(funcName, ...) uint64_t funcName(FDirectory *& _currentDir, __VA_ARGS__)
	RegisterFunc(Dir, bool _s, bool _ad, const FPath & _paths, _Out_ std::vector<std::string> & _outFileNames);
	RegisterFunc(Md, const FPath & _path);
	RegisterFunc(Cd, FPath & _currentPath, const FPath & _path);
	RegisterFunc(Copy, bool _y, const FPath & _sourcePath, const FPath & _destPath);
	RegisterFunc(Del, bool _s, const std::vector<FPath> & _paths);
	RegisterFunc(Rd, bool _s, const std::vector<FPath> & _paths);
	RegisterFunc(Ren, const FPath & _source, const std::string & _dest);
	RegisterFunc(Move, bool _y, const FPath & _source, const FPath & _dest);
	RegisterFunc(Mklink, bool _d, const FPath & _source, const FPath & _dest);
	RegisterFunc(Save, const FPath & _dest);
	RegisterFunc(Load, const FPath & _source);
#undef RegisterFunc

private:
	FDirectory * mRoot = nullptr;

};

template<typename _directoryRefType>
std::vector<typename TSwitchType<TIsConstPointer<_directoryRefType>::value, const FFile*, FFile*>::type>
FVirtualDisk::GetFile(_In_opt_ _directoryRefType _directory, const FPath & _path)
{
	using FileRefType = typename TSwitchType<TIsConstPointer<_directoryRefType>::value, const FFile*, FFile*>::type;
	using DirectoryRefType = typename TSwitchType<TIsConstPointer<_directoryRefType>::value, const FDirectory*, FDirectory*>::type;

	DirectoryRefType nowDirectory = GetFileParentDirectory(_directory, _path);

	if (nowDirectory == nullptr) return std::vector<FileRefType>();
	if (_path.GetPath().size() == 1 && _path.IsAbsolutePath())
		return std::vector<FileRefType>(1, nowDirectory);
	else
		return nowDirectory->SearchSubFile(_path.GetPath().back());
}

template<typename _directoryRefType>
typename TSwitchType<TIsConstPointer<_directoryRefType>::value, const FDirectory*, FDirectory*>::type
FVirtualDisk::GetFileParentDirectory(_directoryRefType _directory, const FPath & _path)
{
	using FileRefType = typename TSwitchType<TIsConstPointer<_directoryRefType>::value, const FFile*, FFile*>::type;
	using DirectoryRefType = typename TSwitchType<TIsConstPointer<_directoryRefType>::value, const FDirectory*, FDirectory*>::type;
	using SymbolLinkRefType = typename TSwitchType<TIsConstPointer<_directoryRefType>::value, const FSymbolLink*, FSymbolLink*>::type;

	DirectoryRefType nowDirectory = _directory;

	if (!_path.IsInVirtualDisk()) return nullptr;
	if (_path.IsAbsolutePath()) nowDirectory = mRoot;
	Assert(nowDirectory);

	for (uint64_t i = _path.IsAbsolutePath() ? 1 : 0; i < _path.GetPath().size() - 1; ++i)
	{
		auto files = nowDirectory->SearchSubFile(_path.GetPath()[i]);

		if (files.size() == 0 || files[0] == nullptr)
			return nullptr;

		auto it = std::find_if(files.begin(), files.end(), [](FileRefType _file) {return _file->GetFileType() == EFileType::Directory || _file->GetFileType() == EFileType::SymbolLink; });
		if (it == files.end()) return nullptr;

		if ((*it)->GetFileType() == EFileType::SymbolLink)
		{
			FileRefType file = GetFinalLinkedFile(dynamic_cast<SymbolLinkRefType>(*it));
			if (file == nullptr) return nullptr;
			if (file->GetFileType() == EFileType::Directory)
			{
				nowDirectory = dynamic_cast<DirectoryRefType>(file);
				continue;
			}
			else
			{
				return nullptr;
			}
		}
		else nowDirectory = dynamic_cast<DirectoryRefType>(*it);
	}

	return nowDirectory;
}

template<typename _symbolLinkRefType>
typename TSwitchType<TIsConstPointer<_symbolLinkRefType>::value, const FFile*, FFile*>::type
FVirtualDisk::GetFinalLinkedFile(_symbolLinkRefType _symbolLink)
{
	using FileRefType = typename TSwitchType<TIsConstPointer<_symbolLinkRefType>::value, const FFile*, FFile*>::type;
	using DirectoryRefType = typename TSwitchType<TIsConstPointer<_symbolLinkRefType>::value, const FDirectory*, FDirectory*>::type;
	using SymbolLinkRefType = typename TSwitchType<TIsConstPointer<_symbolLinkRefType>::value, const FSymbolLink*, FSymbolLink*>::type;

	if (_symbolLink == nullptr) return nullptr;

	auto files = GetFile(nullptr, dynamic_cast<FSymbolLink*>(_symbolLink)->GetLinkedPath());

	FileRefType file = files.size() != 0 ? files[0] : nullptr;

	if (file && file->GetFileType() == EFileType::SymbolLink)
		file = GetFinalLinkedFile(dynamic_cast<SymbolLinkRefType>(file));

	return file;
}
