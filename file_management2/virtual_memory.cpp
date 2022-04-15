#include "file.h"

void FCB::create_virtual_memory(string path, int size) {
	/*记录虚拟内存的路径和大小，其默认文件名为virtual_memory*/
	virtual_memory_path = path;
	virtual_memory_size = size;
	create_new_file("virtual_memory", path);
}

void FCB::write_virtual_memory(string text) {
	if (text.length() > virtual_memory_size) {
		cout << "write error!" << endl;
	}
	else {
		write_file("virtual_memory", virtual_memory_path, text);
	}
}

string FCB::read_virtual_memory(int offset) {
	string text = read_file("virtual_memory", virtual_memory_path);
	int page_num = offset / PAGE_SIZE;
	if (page_num * PAGE_SIZE < text.size())
		return text.substr(page_num * PAGE_SIZE, (page_num + 1) * PAGE_SIZE);
	else
		return "";
}