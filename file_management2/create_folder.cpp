#include <iostream>
#include <fstream>
#include <atltime.h>
#include "file.h"
using namespace std;

bool FCB::create_new_folder(string filename, string path) {
	/*创建根文件夹*/
	if (filename == "root" && path == "") {
		write_fcb_into_new_disk(1, filename, 1);
		disk_occupancy[1] = true;
	}
	/*创建一个普通文件夹*/
	else {
		/*1.将路径文件夹从path中提取出来*/
		vector<string> folder_path;
		split_path(path, folder_path);
		/*2.递归查找父文件夹(folder_path的最后一项)所在磁盘号*/
		int disk = search_parent_folder(folder_path, 0, 1, 0, true);
		/*3.判断此父文件夹下是否有重名文件夹*/
		if (name_available(disk, folder_path[folder_path.size()-1], filename, 1) == 0) {
			/*
				4.在这个文件夹的父文件夹中寻找合适的direct或indirect block.
				其中若direct或indirect block中的目标磁盘块存储的fcb已满，则需要找到新的空磁盘块
				放入下一个direct或indirect block中
			*/
			/*5.将新文件夹的fcb信息存入磁盘块中*/
			write_fcb(disk, folder_path[folder_path.size() - 1], filename, 1);
			/*更新root文件夹的update_time*/
			Value r = read_json("disk\\1.json");
			r["fcb0"]["update_time"] = Value(modify_time());
			write_json("disk\\1.json", r);
		}
		/*文件重名创建失败*/
		else {
			return false;
		}
	}
	return true;
}

/*递归查找创建文件所在目录的fcb所在的磁盘块号，文件路径一定要正确！求求了*/
int FCB::search_parent_folder(vector<string> folder_path, int now_folder_index, int now_disk, int size, bool time) {
	/*如果找到最后一级目录，则返回目录所在块号即为结果*/
	if (now_folder_index == folder_path.size() - 1) {
		return now_disk;
	}
	else {
		string folderToSearch = folder_path[now_folder_index + 1];
		vector<int> nowFolderHoldDisk;
		/*获取直接索引，间接索引里面的全部磁盘号*/
		all_disk_belong_to_folder(now_disk, folder_path[now_folder_index], nowFolderHoldDisk, 1);
		/*在获得的全部磁盘块号中寻找路径中的下一个文件夹*/
		for (int i = 0; i < nowFolderHoldDisk.size(); i++) {
			Value root = read_json("disk\\" + to_string(nowFolderHoldDisk[i]) + ".json");
			for (int j = 0; j < MAX_NUMBER_OF_FCBS_IN_BLOCK; j++) {
				if (root.isMember("fcb" + to_string(j)) && root["fcb" + to_string(j)]["filename"].asString() == folderToSearch && root["fcb" + to_string(j)]["isdir"].asInt() == 1) {
					if (time) {
						root["fcb" + to_string(j)]["update_time"] = Value(modify_time());
					}
					root["fcb" + to_string(j)]["filesize"] = Value(root["fcb" + to_string(j)]["filesize"].asInt() + size);
					write_json("disk\\" + to_string(nowFolderHoldDisk[i]) + ".json", root);
					now_disk = nowFolderHoldDisk[i];
					now_folder_index++;
					return search_parent_folder(folder_path, now_folder_index, now_disk, size, time);
				}
			}
		}
	}
}

/*查找同文件夹下是否有重名文件（夹）*/
int FCB::name_available(int disk, string folder, string filename, int isdir) {
	if (folder == "")
		return 1;
	vector<int> nowFolderHoldDisk;
	/*获取直接索引，间接索引里面的全部磁盘号*/
	all_disk_belong_to_folder(disk, folder, nowFolderHoldDisk, 1);
	/*查找是否有同名文件*/
	for (int i = 0; i < nowFolderHoldDisk.size(); i++) {
		Value root = read_json("disk\\" + to_string(nowFolderHoldDisk[i]) + ".json");
		for (int j = 0; j < MAX_NUMBER_OF_FCBS_IN_BLOCK; j++) {
			if (root.isMember("fcb" + to_string(j)) && root["fcb" + to_string(j)]["filename"].asString() == filename && root["fcb" + to_string(j)]["isdir"].asInt() == isdir) {
				return nowFolderHoldDisk[i];
			}
		}
	}
	return 0;
}

/*这个函数是重点，因为要判断fcb写入的位置是属于目标文件夹下的direct或indirect块，需要反复读文件，判断条件也很多*/
void FCB::write_fcb(int disk, string folder, string filename, int isdir) {
	Value r = read_json("disk\\" + to_string(disk) + ".json");
	int index = find_fcb_index(disk, folder, 1);
	/*先看direct中是否有足够空间保存*/
	for (int i = 0; i < BLOCK_SIZE; i++) {
		int target = r["fcb" + to_string(index)]["direct_block"][i].asInt();
		/*读取失败不会创建空文件*/
		Value root = read_json("disk\\" + to_string(target) + ".json");
		if (target == 0) {
			/*若有空的direct，则需要查找下一个可用的磁盘块，将其放入direct中*/
			/*能进入这里说明上一个磁盘块必然已经使用完，不然一定会进入下面的else if，这样保证了写磁盘的连续性*/
			int diskAvailable = next_disk_available();			
			r["fcb" + to_string(index)]["direct_block"][i] = Value(diskAvailable);
			write_json("disk\\" + to_string(disk) + ".json", r);
			/*将fcb写入diskAvailable中*/
			write_fcb_into_new_disk(diskAvailable, filename, isdir);
			disk_occupancy[diskAvailable] = true;
			return;
		}
		else if (root["occupy"].asInt() != MAX_NUMBER_OF_FCBS_IN_BLOCK) {
			/*若有direct没有使用完，则继续使用*/
			write_fcb_into_old_disk(target, filename, isdir);
			return;
		}
	}
	/*能运行到这里说明direct已满，则需查看indirect，虽然一般大概也许可能用不到间接索引*/
	/*将indirect块号取出*/
	vector<int> indirect_block;
	for (int i = 0; i < BLOCK_SIZE; i++) {
		indirect_block.push_back(r["fcb"+to_string(index)]["indirect_block"][i].asInt());
	}
	/*对每一个块进行分析*/
	for (int i = 0; i < indirect_block.size(); i++) {
		/*思路同direct*/
		if (indirect_block[i] == 0) {
			/*将下一个可用磁盘块号写在indirect中*/
			int diskAvailable = next_disk_available();
			r["fcb" + to_string(index)]["indirect_block"][i] = Value(diskAvailable);
			write_json("disk\\" + to_string(disk) + ".json", r);
			disk_occupancy[diskAvailable] = true;
			/*将间接索引保存至diskavailable中*/
			int fcb_to_write = next_disk_available();
			Value root;
			root["disk"].append(Value(fcb_to_write));
			write_json("disk\\" + to_string(diskAvailable) + ".json", root);
			disk_occupancy[fcb_to_write] = true;
			/*将fcb写在fcb_to_write中*/
			write_fcb_into_new_disk(fcb_to_write, filename, isdir);
			return;
		}
		/*若indirect已经使用，则需要查看指向的磁盘块里面的磁盘块内容*/
		else{
			/*取出间接索引块中的内容*/
			Value indirect = read_json("disk\\" + to_string(indirect_block[i]) + ".json");
			/*以下查看间接索引块中disk的使用情况*/
			/*如果间接索引块中存放的磁盘块号达到上限*/
			if (indirect["disk"].size() == MAX_NUMBER_OF_DISK_BLOCKS_IN_INDIRECT_BLOCK) {
				/*需要查看最后一个磁盘块号中保存的fcb是否达到上限*/
				int lastdisk = indirect["disk"][MAX_NUMBER_OF_DISK_BLOCKS_IN_INDIRECT_BLOCK - 1].asInt();
				Value root = read_json("disk\\" + to_string(lastdisk) + ".json");
				if (root["occupy"].asInt() == MAX_NUMBER_OF_FCBS_IN_BLOCK) {
					continue;
				}
				else {
					write_fcb_into_old_disk(lastdisk, filename, isdir);
					return;
				}
			}
			/*间接索引的磁盘块号未到上限*/
			else {
				/*需要查看最后一个磁盘块号中保存的fcb是否达到上限*/
				int lastdisk = indirect["disk"][indirect["disk"].size() - 1].asInt();
				Value root = read_json("disk\\" + to_string(lastdisk) + ".json");
				if (root["occupy"].asInt() == MAX_NUMBER_OF_FCBS_IN_BLOCK) {
					int diskAvailable = next_disk_available();
					indirect["disk"].append(Value(diskAvailable));
					disk_occupancy[diskAvailable] = true;
					write_json("disk\\" + to_string(indirect_block[i]) + ".json", indirect);
					write_fcb_into_new_disk(diskAvailable, filename, isdir);
					return;
				}
				else {
					write_fcb_into_old_disk(lastdisk, filename, isdir);
					return;
				}
			}
		}
	}
}


/*以下是一些辅助功能函数，可能很多地方用得到（大概）*/


/*启用新的磁盘块写入fcb时*/
void FCB::write_fcb_into_new_disk(int disk, string filename, int isdir) {
	Value root1;
	root1["occupy"] = Value(1);
	Value fcb0;
	fcb0["filename"] = Value(filename);
	fcb0["isdir"] = Value(isdir);
	for (int i = 0; i < BLOCK_SIZE; i++) {
		fcb0["direct_block"].append(Value(0));
		fcb0["indirect_block"].append(Value(0));
	}
	fcb0["create_time"] = Value(modify_time());
	fcb0["update_time"] = Value(modify_time());
	fcb0["filesize"] = Value(0);
	/*将fcb1的信息挂载至root中*/
	root1["fcb0"] = Value(fcb0);
	write_json("disk\\" + to_string(disk) + ".json", root1);
}

/*启用已经使用但没有写满的磁盘块写入fcb时*/
void FCB::write_fcb_into_old_disk(int disk, string filename, int isdir) {
	Value root = read_json("disk\\" + to_string(disk) + ".json");
	int index = 0;
	for (int i = 0; i < MAX_NUMBER_OF_FCBS_IN_BLOCK; i++) {
		if (!root.isMember("fcb" + to_string(i))) {
			index = i;
			break;
		}
	}
	root["occupy"] = Value(root["occupy"].asInt() + 1);
	Value fcb;
	fcb["filename"] = Value(filename);
	fcb["isdir"] = Value(isdir);
	for (int i = 0; i < BLOCK_SIZE; i++) {
		fcb["direct_block"].append(Value(0));
		fcb["indirect_block"].append(Value(0));
	}
	fcb["create_time"] = Value(modify_time());
	fcb["update_time"] = Value(modify_time());
	fcb["filesize"] = Value(0);
	root["fcb" + to_string(index)] = Value(fcb);
	write_json("disk\\" + to_string(disk) + ".json", root);
}

/*查找一个FCB下的所有磁盘块（不包括间接索引指针存放的磁盘块）*/
void FCB::all_disk_belong_to_folder(int disk, string folder, vector<int> &nowFolderHoldDisk, int isdir) {
	int index = find_fcb_index(disk, folder, isdir);
	for (int i = 0; i < BLOCK_SIZE; i++) {
		Value r = read_json("disk\\" + to_string(disk) + ".json");
		if (r["fcb" + to_string(index)]["direct_block"][i].asInt() != 0) {
			nowFolderHoldDisk.push_back(r["fcb" + to_string(index)]["direct_block"][i].asInt());
		}
		if (r["fcb" + to_string(index)]["indirect_block"][i].asInt() != 0) {
			Value root = read_json("disk\\" + to_string(r["fcb" + to_string(index)]["indirect_block"][i].asInt()) + ".json");
			for (int j = 0; j < root["disk"].size(); j++) {
				nowFolderHoldDisk.push_back(root["disk"][j].asInt());
			}
		}
	}
}

/*查找filename是当前磁盘块中的第几个fcb*/
int FCB::find_fcb_index(int disk, string filename, int isdir) {
	Value root = read_json("disk\\" + to_string(disk) + ".json");
	for (int i = 0; i < MAX_NUMBER_OF_FCBS_IN_BLOCK; i++) {
		if (root.isMember("fcb" + to_string(i)) && root["fcb" + to_string(i)]["filename"].asString() == filename && root["fcb" + to_string(i)]["isdir"].asInt() == isdir)
			return i;
	}
	return 0;
}

/*查找下一个可用的空磁盘块*/
int FCB::next_disk_available() {
	int diskAvailable = 0;
	for (int j = 1; j < DISK_BLOCK_SIZE + 1; j++) {
		if (disk_occupancy[j] == false) {
			diskAvailable = j;
			break;
		}
	}
	return diskAvailable;
}

/*分割文件路径*/
void FCB::split_path(string path, vector<string> &folder_path) {
	string temp;
	for (int i = 0; i < path.length(); i++) {
		if (path[i] != '\\') {
			temp += path[i];
		}
		else {
			folder_path.push_back(temp);
			temp = "";
		}
	}
	folder_path.push_back(temp);
}

/*用来读取磁盘块中的全部内容*/
Value FCB::read_json(string path) {
	Value root;
	Reader reader;
	fstream f(path, ios::in);
	reader.parse(f, root);
	f.close();
	return root;
}

/*将内容全部写入磁盘块中，一般与read_json连用*/
void FCB::write_json(string path, Value root) {
	fstream file(path, ios::out);
	StyledWriter sw;
	file << sw.write(root);
	file.close();
}

/*获取系统当前时间*/
string FCB::modify_time() {
	CTime t = CTime::GetCurrentTime();
	string old;
	old = CT2A(t.Format(_T("%Y-%m-%d %H:%M:%S")));
	return old;
}