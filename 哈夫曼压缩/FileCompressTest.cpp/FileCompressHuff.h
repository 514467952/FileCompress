#pragma once
#include"HuffmanTree.hpp"
#include<string>
#include<vector>

struct CharInfo
{
	unsigned char _ch;		//具体的字符
	size_t _count; //字符出现的次数
	std::string _strCode;//字符编码

	CharInfo(size_t count = 0)
		:_count(count)
	{}
	CharInfo operator+(const CharInfo& c)
	{
		//返回对象，无名对象
		return CharInfo(_count + c._count);
	}
	bool operator>(const CharInfo& c)
	{
		return _count > c._count;
	}
	//比较次数
	bool operator==(const CharInfo& c)
	{
		return _count == c._count;
	}
};

class FileCompressHuff
{
public:
	FileCompressHuff();
	void CompressFile(const std::string& path);		//压缩
	void UNComPressFile(const std::string& path); //解压缩
	
private:
	//生成哈夫曼编码
	void GenerateHuffManCode(HuffManTreeNode<CharInfo>* pRoot);
	void WriteHead(FILE* fOut,const string& filename);//传文件指针，和文件后缀
	string GetFilePostFix(const string& filename);
	void ReadLine(FILE* fIn, string& strInfo);
private:
	std::vector<CharInfo> _fileInfo;
};