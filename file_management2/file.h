#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <fstream>
#include "include/json/json.h" 
#pragma comment(lib, "lib_json.lib") 
using namespace std;
using namespace Json;

#define MAX_DISK_BLOCK_CAPACITY 512
#define MAX_NUMBER_OF_DISK_BLOCKS_IN_INDIRECT_BLOCK 4
#define MAX_NUMBER_OF_FCBS_IN_BLOCK 4
#define BLOCK_SIZE 4
#define DISK_BLOCK_SIZE 100
#define PAGE_SIZE 256

typedef struct respect {
	string filename;
	int filesize;
	bool isdir;
	string create_time;
	string update_time;
	//int direct_block[BLOCK_SIZE] = { 0 };
	//int indirect_block[BLOCK_SIZE] = { 0 };
}res;

class FCB {
public:
	string modify_time();//获取时间
	bool create_new_file(string, string);//创建新文件(参数：文件名，路径（不加文件名）)
	bool create_new_folder(string, string);//创建新文件夹(参数：文件名，路径（不加文件夹名）)
	void write_file(string, string, string);//写文件(参数：文件名，路径（不加文件名），内容)
	string read_file(string, string);//读文件(参数：文件名，路径（不加文件名）)
	void read_folder(string, string);//读文件夹(参数：文件名，路径（不加文件夹名）)
	void remove_file(string, string);//删除文件(参数：文件名，路径（不加文件名）)
	bool file_exist(string);//判断文件是否存在（参数：路径（含文件））
	bool folder_exist(string);//判断文件夹是否存在（参数：路径（含文件夹））
	void append_file(string, string, string);//向文件末尾添加内容（参数：文件名，路径（不含文件夹名），内容）
	void create_virtual_memory(string, int);//创建虚拟内存（参数：路径(不包括文件名)，容量）
	void write_virtual_memory(string);//覆盖写虚拟内存（参数：内容）
	string read_virtual_memory(int);//读虚拟内存（参数：偏移量）

	//bool remove_folder(string, string);//删除文件夹

	vector<res> message;//向外暴露读文件夹返回的文件夹下所有文件基本信息
	string virtual_memory_path;//虚拟内存文件路径
	int virtual_memory_size;//虚拟内存文件容量

private:
	bool disk_occupancy[101] = { false };//标记磁盘块的使用
	void write_fcb(int, string, string, int);
	void write_fcb_into_new_disk(int, string, int);//将fcb写入一个新的磁盘块中
	void write_fcb_into_old_disk(int, string, int);//将fcb写入一个内部已经存在数据的磁盘块中
	int search_parent_folder(vector<string>, int, int, int, bool);//查找创建文件所在文件夹的磁盘号
	int name_available(int, string, string, int);//文件名是否重复，有重复返回磁盘块号，无重复返回0
	int find_fcb_index(int, string, int);//查找filename是当前磁盘块中的第几个fcb
	void all_disk_belong_to_folder(int, string, vector<int>&, int);
	void split_path(string, vector<string>&);
	Value read_json(string);
	void write_json(string, Value);
	void remove_json(int);
	int next_disk_available();
};