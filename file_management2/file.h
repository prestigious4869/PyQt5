#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <fstream>
#include "include/json/json.h" 
#pragma comment(lib, "lib_json.lib") 
using namespace std;
using namespace Json;

#define MAX_DISK_BLOCK_CAPACITY 512
#define MAX_NUMBER_OF_DISK_BLOCKS_IN_INDIRECT_BLOCK 4
#define MAX_NUMBER_OF_FCBS_IN_BLOCK 4
#define BLOCK_SIZE 4
#define DISK_BLOCK_SIZE 100
#define PAGE_SIZE 256

typedef struct respect {
	string filename;
	int filesize;
	bool isdir;
	string create_time;
	string update_time;
	//int direct_block[BLOCK_SIZE] = { 0 };
	//int indirect_block[BLOCK_SIZE] = { 0 };
}res;

class FCB {
public:
	string modify_time();//��ȡʱ��
	bool create_new_file(string, string);//�������ļ�(�������ļ�����·���������ļ�����)
	bool create_new_folder(string, string);//�������ļ���(�������ļ�����·���������ļ�������)
	void write_file(string, string, string);//д�ļ�(�������ļ�����·���������ļ�����������)
	string read_file(string, string);//���ļ�(�������ļ�����·���������ļ�����)
	void read_folder(string, string);//���ļ���(�������ļ�����·���������ļ�������)
	void remove_file(string, string);//ɾ���ļ�(�������ļ�����·���������ļ�����)
	bool file_exist(string);//�ж��ļ��Ƿ���ڣ�������·�������ļ�����
	bool folder_exist(string);//�ж��ļ����Ƿ���ڣ�������·�������ļ��У���
	void append_file(string, string, string);//���ļ�ĩβ������ݣ��������ļ�����·���������ļ������������ݣ�
	void create_virtual_memory(string, int);//���������ڴ棨������·��(�������ļ���)��������
	void write_virtual_memory(string);//����д�����ڴ棨���������ݣ�
	string read_virtual_memory(int);//�������ڴ棨������ƫ������

	//bool remove_folder(string, string);//ɾ���ļ���

	vector<res> message;//���Ⱪ¶���ļ��з��ص��ļ����������ļ�������Ϣ
	string virtual_memory_path;//�����ڴ��ļ�·��
	int virtual_memory_size;//�����ڴ��ļ�����

private:
	bool disk_occupancy[101] = { false };//��Ǵ��̿��ʹ��
	void write_fcb(int, string, string, int);
	void write_fcb_into_new_disk(int, string, int);//��fcbд��һ���µĴ��̿���
	void write_fcb_into_old_disk(int, string, int);//��fcbд��һ���ڲ��Ѿ��������ݵĴ��̿���
	int search_parent_folder(vector<string>, int, int, int, bool);//���Ҵ����ļ������ļ��еĴ��̺�
	int name_available(int, string, string, int);//�ļ����Ƿ��ظ������ظ����ش��̿�ţ����ظ�����0
	int find_fcb_index(int, string, int);//����filename�ǵ�ǰ���̿��еĵڼ���fcb
	void all_disk_belong_to_folder(int, string, vector<int>&, int);
	void split_path(string, vector<string>&);
	Value read_json(string);
	void write_json(string, Value);
	void remove_json(int);
	int next_disk_available();
};