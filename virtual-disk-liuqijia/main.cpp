#include "CommandTool.h"

int main()
{
	FVirtualDisk::Get().Init();
	FCommandTool::Get().Init(&FVirtualDisk::Get());


}