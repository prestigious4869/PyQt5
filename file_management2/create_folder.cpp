#include <iostream>
#include <fstream>
#include <atltime.h>
#include "file.h"
using namespace std;

bool FCB::create_new_folder(string filename, string path) {
	/*�������ļ���*/
	if (filename == "root" && path == "") {
		write_fcb_into_new_disk(1, filename, 1);
		disk_occupancy[1] = true;
	}
	/*����һ����ͨ�ļ���*/
	else {
		/*1.��·���ļ��д�path����ȡ����*/
		vector<string> folder_path;
		split_path(path, folder_path);
		/*2.�ݹ���Ҹ��ļ���(folder_path�����һ��)���ڴ��̺�*/
		int disk = search_parent_folder(folder_path, 0, 1, 0, true);
		/*3.�жϴ˸��ļ������Ƿ��������ļ���*/
		if (name_available(disk, folder_path[folder_path.size()-1], filename, 1) == 0) {
			/*
				4.������ļ��еĸ��ļ�����Ѱ�Һ��ʵ�direct��indirect block.
				������direct��indirect block�е�Ŀ����̿�洢��fcb����������Ҫ�ҵ��µĿմ��̿�
				������һ��direct��indirect block��
			*/
			/*5.�����ļ��е�fcb��Ϣ������̿���*/
			write_fcb(disk, folder_path[folder_path.size() - 1], filename, 1);
			/*����root�ļ��е�update_time*/
			Value r = read_json("disk\\1.json");
			r["fcb0"]["update_time"] = Value(modify_time());
			write_json("disk\\1.json", r);
		}
		/*�ļ���������ʧ��*/
		else {
			return false;
		}
	}
	return true;
}

/*�ݹ���Ҵ����ļ�����Ŀ¼��fcb���ڵĴ��̿�ţ��ļ�·��һ��Ҫ��ȷ��������*/
int FCB::search_parent_folder(vector<string> folder_path, int now_folder_index, int now_disk, int size, bool time) {
	/*����ҵ����һ��Ŀ¼���򷵻�Ŀ¼���ڿ�ż�Ϊ���*/
	if (now_folder_index == folder_path.size() - 1) {
		return now_disk;
	}
	else {
		string folderToSearch = folder_path[now_folder_index + 1];
		vector<int> nowFolderHoldDisk;
		/*��ȡֱ��������������������ȫ�����̺�*/
		all_disk_belong_to_folder(now_disk, folder_path[now_folder_index], nowFolderHoldDisk, 1);
		/*�ڻ�õ�ȫ�����̿����Ѱ��·���е���һ���ļ���*/
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

/*����ͬ�ļ������Ƿ��������ļ����У�*/
int FCB::name_available(int disk, string folder, string filename, int isdir) {
	if (folder == "")
		return 1;
	vector<int> nowFolderHoldDisk;
	/*��ȡֱ��������������������ȫ�����̺�*/
	all_disk_belong_to_folder(disk, folder, nowFolderHoldDisk, 1);
	/*�����Ƿ���ͬ���ļ�*/
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

/*����������ص㣬��ΪҪ�ж�fcbд���λ��������Ŀ���ļ����µ�direct��indirect�飬��Ҫ�������ļ����ж�����Ҳ�ܶ�*/
void FCB::write_fcb(int disk, string folder, string filename, int isdir) {
	Value r = read_json("disk\\" + to_string(disk) + ".json");
	int index = find_fcb_index(disk, folder, 1);
	/*�ȿ�direct���Ƿ����㹻�ռ䱣��*/
	for (int i = 0; i < BLOCK_SIZE; i++) {
		int target = r["fcb" + to_string(index)]["direct_block"][i].asInt();
		/*��ȡʧ�ܲ��ᴴ�����ļ�*/
		Value root = read_json("disk\\" + to_string(target) + ".json");
		if (target == 0) {
			/*���пյ�direct������Ҫ������һ�����õĴ��̿飬�������direct��*/
			/*�ܽ�������˵����һ�����̿��Ȼ�Ѿ�ʹ���꣬��Ȼһ������������else if��������֤��д���̵�������*/
			int diskAvailable = next_disk_available();			
			r["fcb" + to_string(index)]["direct_block"][i] = Value(diskAvailable);
			write_json("disk\\" + to_string(disk) + ".json", r);
			/*��fcbд��diskAvailable��*/
			write_fcb_into_new_disk(diskAvailable, filename, isdir);
			disk_occupancy[diskAvailable] = true;
			return;
		}
		else if (root["occupy"].asInt() != MAX_NUMBER_OF_FCBS_IN_BLOCK) {
			/*����directû��ʹ���꣬�����ʹ��*/
			write_fcb_into_old_disk(target, filename, isdir);
			return;
		}
	}
	/*�����е�����˵��direct����������鿴indirect����Ȼһ����Ҳ������ò����������*/
	/*��indirect���ȡ��*/
	vector<int> indirect_block;
	for (int i = 0; i < BLOCK_SIZE; i++) {
		indirect_block.push_back(r["fcb"+to_string(index)]["indirect_block"][i].asInt());
	}
	/*��ÿһ������з���*/
	for (int i = 0; i < indirect_block.size(); i++) {
		/*˼·ͬdirect*/
		if (indirect_block[i] == 0) {
			/*����һ�����ô��̿��д��indirect��*/
			int diskAvailable = next_disk_available();
			r["fcb" + to_string(index)]["indirect_block"][i] = Value(diskAvailable);
			write_json("disk\\" + to_string(disk) + ".json", r);
			disk_occupancy[diskAvailable] = true;
			/*���������������diskavailable��*/
			int fcb_to_write = next_disk_available();
			Value root;
			root["disk"].append(Value(fcb_to_write));
			write_json("disk\\" + to_string(diskAvailable) + ".json", root);
			disk_occupancy[fcb_to_write] = true;
			/*��fcbд��fcb_to_write��*/
			write_fcb_into_new_disk(fcb_to_write, filename, isdir);
			return;
		}
		/*��indirect�Ѿ�ʹ�ã�����Ҫ�鿴ָ��Ĵ��̿�����Ĵ��̿�����*/
		else{
			/*ȡ������������е�����*/
			Value indirect = read_json("disk\\" + to_string(indirect_block[i]) + ".json");
			/*���²鿴�����������disk��ʹ�����*/
			/*�������������д�ŵĴ��̿�Ŵﵽ����*/
			if (indirect["disk"].size() == MAX_NUMBER_OF_DISK_BLOCKS_IN_INDIRECT_BLOCK) {
				/*��Ҫ�鿴���һ�����̿���б����fcb�Ƿ�ﵽ����*/
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
			/*��������Ĵ��̿��δ������*/
			else {
				/*��Ҫ�鿴���һ�����̿���б����fcb�Ƿ�ﵽ����*/
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


/*������һЩ�������ܺ��������ܺܶ�ط��õõ�����ţ�*/


/*�����µĴ��̿�д��fcbʱ*/
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
	/*��fcb1����Ϣ������root��*/
	root1["fcb0"] = Value(fcb0);
	write_json("disk\\" + to_string(disk) + ".json", root1);
}

/*�����Ѿ�ʹ�õ�û��д���Ĵ��̿�д��fcbʱ*/
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

/*����һ��FCB�µ����д��̿飨�������������ָ���ŵĴ��̿飩*/
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

/*����filename�ǵ�ǰ���̿��еĵڼ���fcb*/
int FCB::find_fcb_index(int disk, string filename, int isdir) {
	Value root = read_json("disk\\" + to_string(disk) + ".json");
	for (int i = 0; i < MAX_NUMBER_OF_FCBS_IN_BLOCK; i++) {
		if (root.isMember("fcb" + to_string(i)) && root["fcb" + to_string(i)]["filename"].asString() == filename && root["fcb" + to_string(i)]["isdir"].asInt() == isdir)
			return i;
	}
	return 0;
}

/*������һ�����õĿմ��̿�*/
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

/*�ָ��ļ�·��*/
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

/*������ȡ���̿��е�ȫ������*/
Value FCB::read_json(string path) {
	Value root;
	Reader reader;
	fstream f(path, ios::in);
	reader.parse(f, root);
	f.close();
	return root;
}

/*������ȫ��д����̿��У�һ����read_json����*/
void FCB::write_json(string path, Value root) {
	fstream file(path, ios::out);
	StyledWriter sw;
	file << sw.write(root);
	file.close();
}

/*��ȡϵͳ��ǰʱ��*/
string FCB::modify_time() {
	CTime t = CTime::GetCurrentTime();
	string old;
	old = CT2A(t.Format(_T("%Y-%m-%d %H:%M:%S")));
	return old;
}