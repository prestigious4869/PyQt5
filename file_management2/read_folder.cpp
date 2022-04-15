#include "file.h"

void FCB::read_folder(string filename, string path) {
	message.clear();
	/*1.分割路径*/
	vector<string> folder_path;
	split_path(path, folder_path);
	/*2.寻找文件fcb所在磁盘号及fcb编号*/
	int parentFolderDisk = search_parent_folder(folder_path, 0, 1, 0, false);
	int folderDisk = name_available(parentFolderDisk, folder_path[folder_path.size() - 1], filename, 1);
	/*3.取出属于文件fcb的全部磁盘号*/
	vector<int> nowFolderHoldDisk;
	all_disk_belong_to_folder(folderDisk, filename, nowFolderHoldDisk, 1);
	/*4.取出全部磁盘中的全部fcb*/
	for (int i = 0; i < int(nowFolderHoldDisk.size()); i++) {
		Value root = read_json("disk\\" + to_string(nowFolderHoldDisk[i]) + ".json");
		for (int j = 0; j < BLOCK_SIZE; j++) {
			if (root.isMember("fcb" + to_string(j))) {
				res response;
				response.filename = root["fcb" + to_string(j)]["filename"].asString();
				response.filesize = root["fcb" + to_string(j)]["filesize"].asInt();
				response.create_time = root["fcb" + to_string(j)]["create_time"].asString();
				response.update_time = root["fcb" + to_string(j)]["update_time"].asString();
				response.isdir = root["fcb" + to_string(j)]["isdir"].asInt();
				message.push_back(response);
			}
		}
	}
}