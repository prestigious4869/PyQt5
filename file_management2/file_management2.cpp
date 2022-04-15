#include "file.h"
using namespace std;

int main()
{
	for (int i = 1; i <= 100; i++) {
		ofstream fout;
		fout.open("disk\\"+to_string(i)+".json");
		fout.close();
	}
	FCB root;
	root.create_new_folder("root","");

	root.create_virtual_memory("root", 8000);
	root.write_virtual_memory("123456789");
	cout << root.read_virtual_memory(500);
}


/*以下函数用来获取一个目录下的全部文件路径*/
//#include <iostream>
//#include <string>
//#include <vector>
////注意这个头文件
//#include <io.h>
//using namespace std;
//
////核心函数
//void getFileNames(string path, vector<string>& files);
//
//int main() {
//	vector<string> fileNames;
//	string path("G:\\code\\file_management"); 	//自己选择目录测试
//	getFileNames(path, fileNames);
//	for (const auto& ph : fileNames) {
//		std::cout << ph << "\n";
//	}
//	return 0;
//}
////核心代码
//void getFileNames(string path, vector<string>& files)
//{
//	//文件句柄
//	//注意：我发现有些文章代码此处是long类型，实测运行中会报错访问异常
//	intptr_t hFile = 0;
//	//文件信息
//	struct _finddata_t fileinfo;
//	string p;
//	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
//	{
//		do
//		{
//			//如果是目录,递归查找
//			//如果不是,把文件绝对路径存入vector中
//			if ((fileinfo.attrib & _A_SUBDIR))
//			{
//				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
//					getFileNames(p.assign(path).append("\\").append(fileinfo.name), files);
//			}
//			else
//			{
//				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
//			}
//		} while (_findnext(hFile, &fileinfo) == 0);
//		_findclose(hFile);
//	}
//}