#include "file.h"

void FCB::remove_file(string filename, string path) {
	/*1.分割路径*/
	string oldtext = read_file(filename, path);
	vector<string> folder_path;
	split_path(path, folder_path);
	/*2.寻找文件fcb所在磁盘号及fcb编号*/
	int parentFolderDisk = search_parent_folder(folder_path, 0, 1, oldtext.length()*(-1), true);
	int fileDisk = name_available(parentFolderDisk, folder_path[folder_path.size() - 1], filename, 0);
	int index = find_fcb_index(fileDisk, filename, 0);
	/*3.更新root文件夹*/
	Value r = read_json("disk\\1.json");
	r["fcb0"]["update_time"] = Value(modify_time());
	r["fcb0"]["filesize"] = Value(r["fcb0"]["filesize"].asInt() + oldtext.length()*(-1));
	write_json("disk\\1.json", r);
	/*4.删除fcb中保存的全部内容,首先取出所有磁盘块*/
	vector<int> nowFileHoldDisk;
	all_disk_belong_to_folder(fileDisk, filename, nowFileHoldDisk, 0);
	Value root = read_json("disk\\" + to_string(fileDisk) + ".json");
	for (int i = 0; i < BLOCK_SIZE; i++) {
		if (root["fcb" + to_string(index)]["indirect_block"][i].asInt() != 0) {
			nowFileHoldDisk.push_back(root["fcb" + to_string(index)]["indirect_block"][i].asInt());
		}
	}
	for (int i = 0; i < nowFileHoldDisk.size(); i++) {
		/*新建==删除*/
		remove_json(nowFileHoldDisk[i]);
	}
	/*5.文件下的磁盘块已经删除，现在需要删除文件fcb以及修改文件夹fcb*/
	/*如果文件fcb所在磁盘块只有这一个fcb，则需要删除磁盘块内容，并将父文件夹fcb中此磁盘块号置0*/
	if (root["occupy"].asInt() == 1) {
		/*新建==删除*/
		remove_json(fileDisk);
		/*在父文件夹直接索引中查找磁盘号*/
		Value folder = read_json("disk\\" + to_string(parentFolderDisk) + ".json");
		int folder_index = find_fcb_index(parentFolderDisk, folder_path[folder_path.size() - 1], 1);
		for (int i = 0; i < BLOCK_SIZE; i++) {
			if (folder["fcb" + to_string(folder_index)]["direct_block"][i].asInt() == fileDisk) {
				folder["fcb" + to_string(folder_index)]["direct_block"][i] = Value(0);
				write_json("disk\\" + to_string(parentFolderDisk) + ".json", folder);
				return;
			}
		}
		/*在间接索引中查找磁盘块号*/
		for (int i = 0; i < BLOCK_SIZE; i++) {
			Value indirect = read_json("disk\\" + to_string(folder["fcb" + to_string(folder_index)]["indirect_block"][i].asInt()) + ".json");
			for (int j = 0; j < indirect["disk"].size(); j++) {
				/*如果磁盘块号为间接索引磁盘块中唯一的disk*/
				if (indirect["disk"][j].asInt() == fileDisk && indirect["disk"].size() == 1) {
					/*新建==删除*/
					remove_json(folder["fcb" + to_string(folder_index)]["indirect_block"][i].asInt());
					folder["fcb" + to_string(folder_index)]["indirect_block"][i] = Value(0);
					write_json("disk\\" + to_string(parentFolderDisk) + ".json", folder);
					return;
				}
				else if (indirect["disk"][j].asInt() == fileDisk) {
					indirect["disk"].removeIndex(j, NULL);
					write_json("disk\\" + to_string(folder["fcb" + to_string(folder_index)]["indirect_block"][i].asInt()) + ".json", indirect);
					return;
				}
			}
		}
	}
	/*如果文件fcb所在磁盘不止一个fcb*/
	else {
		root["occupy"] = Value(root["occupy"].asInt() - 1);
		root.removeMember("fcb" + to_string(index));
		write_json("disk\\" + to_string(fileDisk) + ".json", root);
		return;
	}
}

void FCB::remove_json(int disk) {
	ofstream fout;
	fout.open("disk\\" + to_string(disk) + ".json");
	fout.close();
	disk_occupancy[disk] = false;
}