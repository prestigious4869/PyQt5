#include "file.h"

bool FCB::create_new_file(string filename, string path) {
	/*1.将路径文件夹从path中提取出来*/
	vector<string> folder_path;
	split_path(path, folder_path);
	/*2.递归查找父文件夹(folder_path的最后一项)所在磁盘号*/
	int disk = search_parent_folder(folder_path, 0, 1, 0, true);
	/*3.判断重名*/
	if (name_available(disk, folder_path[folder_path.size() - 1], filename, 0) == 0) {
		/*4.将新文件夹的fcb信息存入磁盘块中，同create_new_folder*/
		write_fcb(disk, folder_path[folder_path.size() - 1], filename, 0);
		/*5.更新root文件夹的update_time*/
		Value r = read_json("disk\\1.json");
		r["fcb0"]["update_time"] = Value(modify_time());
		write_json("disk\\1.json", r);
	}
	else {
		return false;
	}
	return true;
}