#pragma once
#include"HuffmanTree.hpp"
#include<string>
#include<vector>

struct CharInfo
{
	unsigned char _ch;		//������ַ�
	size_t _count; //�ַ����ֵĴ���
	std::string _strCode;//�ַ�����

	CharInfo(size_t count = 0)
		:_count(count)
	{}
	CharInfo operator+(const CharInfo& c)
	{
		//���ض�����������
		return CharInfo(_count + c._count);
	}
	bool operator>(const CharInfo& c)
	{
		return _count > c._count;
	}
	//�Ƚϴ���
	bool operator==(const CharInfo& c)
	{
		return _count == c._count;
	}
};

class FileCompressHuff
{
public:
	FileCompressHuff();
	void CompressFile(const std::string& path);		//ѹ��
	void UNComPressFile(const std::string& path); //��ѹ��
	
private:
	//���ɹ���������
	void GenerateHuffManCode(HuffManTreeNode<CharInfo>* pRoot);
	void WriteHead(FILE* fOut,const string& filename);//���ļ�ָ�룬���ļ���׺
	string GetFilePostFix(const string& filename);
	void ReadLine(FILE* fIn, string& strInfo);
private:
	std::vector<CharInfo> _fileInfo;
};