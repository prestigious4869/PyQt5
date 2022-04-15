#include "file.h"

bool FCB::file_exist(string path) {
	/*分割路径取出文件名*/
	string file;
	vector<string> folder_path;
	split_path(path, folder_path);
	file = folder_path[folder_path.size() - 1];
	folder_path.pop_back();
	/*先定位父文件夹所在磁盘块号*/
	int parentFolderDisk = search_parent_folder(folder_path, 0, 1, 0, false);
	if (name_available(parentFolderDisk, folder_path[folder_path.size() - 1], file, 0) == 0) {
		return false;
	}
	return true;
}

bool FCB::folder_exist(string path) {
	/*分割路径取出文件名*/
	string folder;
	vector<string> folder_path;
	split_path(path, folder_path);
	/*寻找root文件夹*/
	if (folder_path.size() == 1) {
		return true;
	}
	else {
		folder = folder_path[folder_path.size() - 1];
		folder_path.pop_back();
	}
	/*先定位父文件夹所在磁盘块号*/
	int parentFolderDisk = search_parent_folder(folder_path, 0, 1, 0, false);
	if (name_available(parentFolderDisk, folder_path[folder_path.size() - 1], folder, 1) == 0) {
		return false;
	}
	return true;
}