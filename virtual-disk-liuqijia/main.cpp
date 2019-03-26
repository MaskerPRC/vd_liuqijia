#include "CommandTool.h"

int main()
{
	FVirtualDisk::Get().Init();
	FCommandTool::Get().Init(&FVirtualDisk::Get());
	auto virtualDisk = &FVirtualDisk::Get();
	auto cmdtool = &FCommandTool::Get();
	FCommandTool::Get().Exec("md \"b in\"");
	FCommandTool::Get().Exec("md baa");
	FCommandTool::Get().Exec("mklink sym \"b in\"");
	FCommandTool::Get().Exec("mklink sym1 sym");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\test1.txt 1.txt");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\test1.txt C:\\11.txt");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\test1.txt C:\\2.txt");
	FCommandTool::Get().Exec("cd \"b in\"");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\test2.txt 1.txt");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\test2.txt C:\\\"b in\"\\11.txt");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\test2.txt C:\\\"b in\"\\2.txt");
	FCommandTool::Get().Exec("md st");
	FCommandTool::Get().Exec("cd..");
	FCommandTool::Get().Exec("cd baa");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\test3.txt 1.txt");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\test3.txt C:\\baa\\11.txt");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\test3.txt C:\\baa\\2.txt");
	FCommandTool::Get().Exec("cd ..");
	FCommandTool::Get().Exec("mklink s.txt 1.txt");
	FCommandTool::Get().Exec("mklink y.txt s.txt");


	
	FCommandTool::Get().Clear();
	FVirtualDisk::Get().Clear();

	system("pause");
}