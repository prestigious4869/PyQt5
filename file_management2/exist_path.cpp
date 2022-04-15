#include "file.h"

bool FCB::file_exist(string path) {
	/*�ָ�·��ȡ���ļ���*/
	string file;
	vector<string> folder_path;
	split_path(path, folder_path);
	file = folder_path[folder_path.size() - 1];
	folder_path.pop_back();
	/*�ȶ�λ���ļ������ڴ��̿��*/
	int parentFolderDisk = search_parent_folder(folder_path, 0, 1, 0, false);
	if (name_available(parentFolderDisk, folder_path[folder_path.size() - 1], file, 0) == 0) {
		return false;
	}
	return true;
}

bool FCB::folder_exist(string path) {
	/*�ָ�·��ȡ���ļ���*/
	string folder;
	vector<string> folder_path;
	split_path(path, folder_path);
	/*Ѱ��root�ļ���*/
	if (folder_path.size() == 1) {
		return true;
	}
	else {
		folder = folder_path[folder_path.size() - 1];
		folder_path.pop_back();
	}
	/*�ȶ�λ���ļ������ڴ��̿��*/
	int parentFolderDisk = search_parent_folder(folder_path, 0, 1, 0, false);
	if (name_available(parentFolderDisk, folder_path[folder_path.size() - 1], folder, 1) == 0) {
		return false;
	}
	return true;
}