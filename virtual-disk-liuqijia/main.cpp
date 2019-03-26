#include "CommandTool.h"
#include "testVirtualDisk.h"

void Rebuild()
{
	VirtualDisk vd;
	vd.formatDisk();
	FCommandTool::Get().Exec("md \"b in\"");
	FCommandTool::Get().Exec("md baa");
	FCommandTool::Get().Exec("mklink sym \"b in\"");
	FCommandTool::Get().Exec("mklink sym1 sym");
	FCommandTool::Get().Exec("copy @test1.txt 1.txt");
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
}

int main()
{
	FVirtualDisk::Get().Init();
	FCommandTool::Get().Init(&FVirtualDisk::Get());
	auto virtualDisk = &FVirtualDisk::Get();
	auto cmdtool = &FCommandTool::Get();

	VirtualDisk vd;

	Rebuild();

	FCommandTool::Get().Exec("cd stl");
	std::string path = vd.getCurPath();
	FCommandTool::Get().Exec("cd.");
	path = vd.getCurPath();
	FCommandTool::Get().Exec("cd");
	path = vd.getCurPath();
	FCommandTool::Get().Exec("cd baa");
	path = vd.getCurPath();
	FCommandTool::Get().Exec("cd..");
	path = vd.getCurPath();
	FCommandTool::Get().Exec("cd sym1");
	path = vd.getCurPath();
	FCommandTool::Get().Exec("cd st");
	path = vd.getCurPath();
	FCommandTool::Get().Exec("cd /");
	path = vd.getCurPath();
	FCommandTool::Get().Exec("cd C:\\sym\\st");
	path = vd.getCurPath();
	FCommandTool::Get().Exec("cd /");
	path = vd.getCurPath();
	FCommandTool::Get().Exec("cd b*");
	path = vd.getCurPath();

	Rebuild();

	FCommandTool::Get().Exec("md stl");
	FCommandTool::Get().Exec("md C:\\\"b in\"\\sg baa\\sf");
	FCommandTool::Get().Exec("md x\\y\\z");
	FCommandTool::Get().Exec("md C:\\sym\\ab");

	Rebuild();

	FCommandTool::Get().Exec("del 2.txt /y");
	FCommandTool::Get().Exec("del 1*");
	FCommandTool::Get().Exec("del baa");
	FCommandTool::Get().Exec("del sym1");
	FCommandTool::Get().Exec("del s.txt y.txt");
	FCommandTool::Get().Exec("del sfs");
	FCommandTool::Get().Exec("del . /s");

	Rebuild();

	FCommandTool::Get().Exec("ren a.txt 3.txt");
	FCommandTool::Get().Exec("ren 1.txt 2.txt");
	FCommandTool::Get().Exec("ren 1.txt a.txt");
	FCommandTool::Get().Exec("ren 1* b.txt");
	FCommandTool::Get().Exec("ren sym\\st sf");
	FCommandTool::Get().Exec("ren c:\\\"b in\" bin");
	FCommandTool::Get().Exec("ren c:\\sym1\\1.txt 2.txt");
	FCommandTool::Get().Exec("ren c:\\bin\\1.txt f.txt");

	Rebuild();

	FCommandTool::Get().Exec("copy stl\1.txt baa");
	FCommandTool::Get().Exec("copy 1.txt bas\\1.txt");
	FCommandTool::Get().Exec("copy 1.txt baa\\3.txt");
	FCommandTool::Get().Exec("copy C:\\baa\\1* 1* /y");
	FCommandTool::Get().Exec("copy baa C:\\sym\\st");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\UML.pdf C:\\");
	FCommandTool::Get().Exec("copy s.txt z.txt");

	Rebuild();

	FCommandTool::Get().Exec("del 1.txt");
	FCommandTool::Get().Exec("md stl");
	FCommandTool::Get().Exec("move 2.txt stl");
	FCommandTool::Get().Exec("rd sym");
	FCommandTool::Get().Exec("copy @VirtualDiskTest\\filetree.png C:\\");
	FCommandTool::Get().Exec("save @VirtualDiskTest\\save.xml");
	FCommandTool::Get().Exec("del 11.txt");
	FCommandTool::Get().Exec("md sf");
	FCommandTool::Get().Exec("load @VirtualDiskTest\\save.xml");

	FCommandTool::Get().Clear();
	FVirtualDisk::Get().Clear();

	system("pause");
}