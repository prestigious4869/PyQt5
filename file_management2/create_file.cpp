#include "file.h"

bool FCB::create_new_file(string filename, string path) {
	/*1.��·���ļ��д�path����ȡ����*/
	vector<string> folder_path;
	split_path(path, folder_path);
	/*2.�ݹ���Ҹ��ļ���(folder_path�����һ��)���ڴ��̺�*/
	int disk = search_parent_folder(folder_path, 0, 1, 0, true);
	/*3.�ж�����*/
	if (name_available(disk, folder_path[folder_path.size() - 1], filename, 0) == 0) {
		/*4.�����ļ��е�fcb��Ϣ������̿��У�ͬcreate_new_folder*/
		write_fcb(disk, folder_path[folder_path.size() - 1], filename, 0);
		/*5.����root�ļ��е�update_time*/
		Value r = read_json("disk\\1.json");
		r["fcb0"]["update_time"] = Value(modify_time());
		write_json("disk\\1.json", r);
	}
	else {
		return false;
	}
	return true;
}