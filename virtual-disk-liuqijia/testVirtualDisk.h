#pragma once
#include <string>
class VirtualDisk
{
public:
	VirtualDisk();
	bool formatDisk();
	bool executeCmd(const std::string & _cmd);
	std::string getCurPath();
	bool containNode(std::string _path, int & _size, int &_type);
	std::string getLinkNode(std::string _path);
	~VirtualDisk();
};