#include "file.h"

void FCB::write_file(string filename, string path, string newtext) {
	/*1.�ָ�·��*/
	string oldtext = read_file(filename, path);
	vector<string> folder_path;
	split_path(path, folder_path);
	/*2.Ѱ���ļ�fcb���ڴ��̺ż�fcb���*/
	int parentFolderDisk = search_parent_folder(folder_path, 0, 1, newtext.length() - oldtext.length(), true);
	int fileDisk = name_available(parentFolderDisk, folder_path[folder_path.size() - 1], filename, 0);
	int index = find_fcb_index(fileDisk, filename, 0);
	/*3.����root�ļ���*/
	Value r = read_json("disk\\1.json");
	r["fcb0"]["update_time"] = Value(modify_time());
	r["fcb0"]["filesize"] = Value(r["fcb0"]["filesize"].asInt() + newtext.length() - oldtext.length());
	write_json("disk\\1.json", r);
	/*4.ɾ��fcb�б����ȫ������,����ȡ�����д��̿�*/
	vector<int> nowFileHoldDisk;
	all_disk_belong_to_folder(fileDisk, filename, nowFileHoldDisk, 0);
	Value root = read_json("disk\\" + to_string(fileDisk) + ".json");
	for (int i = 0; i < BLOCK_SIZE; i++) {
		if (root["fcb" + to_string(index)]["indirect_block"][i].asInt() != 0) {
			nowFileHoldDisk.push_back(root["fcb" + to_string(index)]["indirect_block"][i].asInt());
		}
	}
	for (int i = 0; i < nowFileHoldDisk.size(); i++) {
		/*�½�==ɾ��*/
		remove_json(nowFileHoldDisk[i]);
	}
	/*5.���ļ�fcb�е�direct��indirectȫ����Ϊ��*/
	for (int i = 0; i < BLOCK_SIZE; i++) {
		root["fcb" + to_string(index)]["direct_block"][i] = Value(0);
		root["fcb" + to_string(index)]["indirect_block"][i] = Value(0);
	}
	write_json("disk\\" + to_string(fileDisk) + ".json",root);
	/*6.����д����*/
	vector<int> disk;
	root = read_json("disk\\" + to_string(fileDisk) + ".json");
	root["fcb" + to_string(index)]["update_time"] = Value(modify_time());
	root["fcb" + to_string(index)]["filesize"] = Value(newtext.length());
	while (newtext.length() != 0) {
		disk.push_back(next_disk_available());
		disk_occupancy[disk[disk.size() - 1]] = true;
		Value r;
		r["data"] = Value(newtext.substr(0, MAX_DISK_BLOCK_CAPACITY));
		write_json("disk\\" + to_string(disk[disk.size() - 1]) + ".json", r);
		newtext.erase(0, MAX_DISK_BLOCK_CAPACITY);
	}
	/*7.�����̺������ļ�fcb��*/
	if (disk.size() <= BLOCK_SIZE) {
		for (int i = 0; i < disk.size(); i++) {
			root["fcb" + to_string(index)]["direct_block"][i] = Value(disk[i]);
		}
		write_json("disk\\" + to_string(fileDisk) + ".json", root);
	}
	/*����õ�indirect��������ܲ��Բ������п��ܳ���*/
	else {
		int i = 0;
		vector<int> indirect;
		for (; i < BLOCK_SIZE; i++) {
			root["fcb" + to_string(index)]["direct_block"][i] = Value(disk[i]);
		}
		for (; i < disk.size(); i++) {
			int nextDisk = next_disk_available();
			disk_occupancy[nextDisk] = true;
			indirect.push_back(nextDisk);
			Value r;
			r["disk"].append(Value(disk[i]));
			i++;
			for (; r["disk"].size() <= MAX_NUMBER_OF_DISK_BLOCKS_IN_INDIRECT_BLOCK && i < disk.size(); i++) {
				r["disk"].append(Value(disk[i]));
			}
		}
		for (int j = 0; j < indirect.size(); j++) {
			root["fcb" + to_string(index)]["indirect_block"][j] = Value(indirect[j]);
		}
		write_json("disk\\" + to_string(fileDisk) + ".json", root);
	}
}

void FCB::append_file(string filename, string path, string text) {
	string oldtext = read_file(filename, path);
	string newtext = oldtext + text;
	write_file(filename, path, newtext);
}