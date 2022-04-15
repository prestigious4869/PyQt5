#include "file.h"

string FCB::read_file(string filename, string path) {
	/*1.�ָ�·��*/
	vector<string> folder_path;
	split_path(path, folder_path);
	/*2.Ѱ���ļ�fcb���ڴ��̺ż�fcb���*/
	int parentFolderDisk = search_parent_folder(folder_path, 0, 1, 0, false);
	int fileDisk = name_available(parentFolderDisk, folder_path[folder_path.size() - 1], filename, 0);
	/*3.ȡ�������ļ�fcb��ȫ�����̺�*/
	vector<int> nowFolderHoldDisk;
	all_disk_belong_to_folder(fileDisk, filename, nowFolderHoldDisk, 0);
	/*4.����������ƴ��Ϊ���ַ�������*/
	string res;
	for (int i = 0; i < nowFolderHoldDisk.size(); i++) {
		Value root = read_json("disk\\" + to_string(nowFolderHoldDisk[i]) + ".json");
		res += root["data"].asString();
	}
	return res;
}