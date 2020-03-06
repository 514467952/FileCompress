#define _CRT_SECURE_NO_WARNINGS

#include"FileCompressHuff.h"
#include<assert.h>
#include"HuffmanTree.hpp"

//构造函数
FileCompressHuff::FileCompressHuff()
{
	_fileInfo.resize(256);
	for (int i = 0; i < 256; ++i)
	{
		_fileInfo[i]._ch = i;
		_fileInfo[i]._count = 0;
	}
}
//压缩文件
void FileCompressHuff::CompressFile(const string& path)
{
	//1.统计源文件中每个字符出现的次数
	FILE* fIn = fopen(path.c_str(),"rb");
	if (nullptr == fIn)
	{
		assert(false);
		return;
	}

	unsigned char * pReadBuff = new unsigned char[1024];
	int rdSize = 0;
	while (true)
	{
		//读文件
		rdSize = fread(pReadBuff, 1, 1024, fIn);	
		if (0 == rdSize)
			break;
		//统计
		for (int i = 0; i < rdSize; ++i)
		{
			_fileInfo[pReadBuff[i]]._count++;
		}
	}
	
	//2.以这些出现的次数为权值创建哈夫曼树
	HuffManTree<CharInfo> t(_fileInfo,CharInfo());

	//3.获取每个字符的编码
	GenerateHuffManCode(t.GetRoot());
	//4.获取到的字符编码重新改写
	FILE* fOut = fopen("2.txt", "wb");  //打开一个文件保存压缩后的结果
	if (nullptr == fOut)
	{
		assert(false);
		return;
	}

	WriteHead(fOut, path);

	fseek(fIn,0,SEEK_SET); //把文件指针再次放到文件的起始位置
	char ch = 0;	//存放字符编码
	int bitcount = 0; //计算放了多少个比特位
	while (true)
	{
		rdSize = fread(pReadBuff, 1, 1024,fIn);
		if (0 == rdSize)  //文件读取结束
			break;

		//根据字节的编码对读取到的内容进行重写
		for (size_t i = 0; i < rdSize; ++i)
		{
			//拿到编码
			string strCode = _fileInfo [pReadBuff[i]]._strCode;
			//A "100"
			for (size_t j = 0; j < strCode.size(); ++j)
			{
				ch <<= 1; //每放一个往左一位，把下一个编码往里放
				//存放的时候，一个一个比特位来放
				if ('1' == strCode[j])
					ch |= 1;

				bitcount++;
				if (8 == bitcount) //说明ch满了，把这个字节写到文件中去
				{
					//fputc 写单个字节的函数
					fputc(ch, fOut);
					//写完后，都清零
					bitcount = 0;
					ch = 0;
				}

			}
		}
	}
	//最后一次ch中可能不够8个比特位
	if (bitcount < 8)
	{
		ch <<= 8 - bitcount; //左移剩余的位数
		//不够的位肯定要写到文件中
		fputc(ch, fOut);
	}
	delete[]pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

//解压缩文件
void FileCompressHuff::UNComPressFile(const std::string& path)
{
	FILE* fIn = fopen(path.c_str(), "rb");
	if (nullptr == fIn)
	{
		assert(false);
		return;
	}

	//读取文件的后缀
	string strFilePostFix;
	ReadLine(fIn,strFilePostFix);
	//读取字符信息的总行数
	string strCount;
	ReadLine(fIn, strCount);
	int lineCount = atoi(strCount.c_str()); //总的行数

	//读取字符的信息
	for (int i = 0; i < lineCount; ++i)
	{
		string strchCount;
		ReadLine(fIn, strchCount); //读每一行的字符信息
		//如果读取到的是\n
		if (strchCount.empty())
		{
			strchCount += '\n'; //将\n写入读取
			ReadLine(fIn, strchCount); //多读一行，将\n的次数和冒号读取
		}
		//A:100
		_fileInfo[(unsigned char)strchCount[0]]._count = atoi(strchCount.c_str() + 2);//跳过前两个字符，因为前两个是A:
	}
	
	//还原Huffman树
	HuffManTree<CharInfo> t;  //创建Huffamn树的对象
	t.CreatHuffManTree(_fileInfo, CharInfo(0)); //还原Huffman树


	FILE* fOut = fopen("3.txt","wb");
	assert(fOut);
	//解压缩
	char* pReadBuff = new char[1024];	//创建缓冲区
	char ch = 0;
	HuffManTreeNode<CharInfo>* pCur = t.GetRoot();
	size_t fileSize = pCur->_weight._count; //文件总的大小就是根结点的权值的次数
	size_t uncount = 0;//表示解压缩了多少个
	while (true)
	{
		size_t rdSize = fread(pReadBuff,1,1024,fIn); //读数据
		if (0 == rdSize)
			break;
		//一个个字节进行解压缩
		for (size_t i = 0; i < rdSize; ++i)
		{
			//只需要将一个字节中的8个比特位单独处理
			ch = pReadBuff[i];
			for (int pos = 0; pos < 8; ++pos)
			{
				//增加一次判断
				if (nullptr == pCur->_pLeft && nullptr == pCur->_pRight)
				{
					//uncount++; //每次解压缩一个，就++一下
					fputc(pCur->_weight._ch, fOut);
					if (uncount == fileSize)
						break;
					//叶子结点,解压出一个字符，写入文件			
					pCur = t.GetRoot(); //把pCur放到树根的位置上继续
				}
				if (ch & 0x80)
					//如果该位上的数字为1
					pCur = pCur->_pRight;
				else
					pCur = pCur->_pLeft;

				ch <<= 1; //与完后往左移动一位
				if (nullptr == pCur->_pLeft && nullptr == pCur->_pRight)
				{
					uncount++; //每次解压缩一个，就++一下
					fputc(pCur->_weight._ch, fOut);
					if (uncount == fileSize)
						break;
					//叶子结点,解压出一个字符，写入文件			
					pCur = t.GetRoot(); //把pCur放到树根的位置上继续
				}
			}//for循环完后再读取下一个字节
		}
	}

	delete[]pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

//读取压缩文件的信息
void FileCompressHuff::ReadLine(FILE* fIn, string& strInfo)
{
	assert(fIn);

	//读取一行的字符
	while (!feof(fIn))  //只要文件指针没有到文件末尾就读取
	{
		//读取一个字符
		char ch = fgetc(fIn);
		if (ch == '\n')
			break;
		//有效的字符就拼接上去
		strInfo += ch;
	}
}

//获取文件的后缀
//2.txt
//F：\123\2.txt
string FileCompressHuff:: GetFilePostFix(const string& filename)
{
	//substr截取文件，第二个参数没有给的话默认截取到末尾
	return  filename.substr(filename.rfind('.'));
}

//压缩文件格式
void FileCompressHuff::WriteHead(FILE* fOut, const string& filename)
{
	assert(fOut);

	//写文件的后缀
	string strHead;
	strHead += GetFilePostFix(filename);
	strHead += '\n'; //后缀与后面的内容之间用\n隔开
	
	//写行数
	size_t lineCount = 0;
	string strChCount;
	char szValue[32] = { 0 };	//缓冲区放入字符次数
	for (int i = 0; i < 256; ++i)
	{
		CharInfo& charInfo = _fileInfo[i];
		if (charInfo._count)
		{
			
			lineCount++;	//行数
			strChCount += charInfo._ch; //字符
			strChCount += ':';	//字符与字符次数之间用冒号隔开
			_itoa(charInfo._count, szValue, 10);
			strChCount += szValue;		//字符次数
			strChCount += '\n';	//末尾加\n
		}
	}

	_itoa(lineCount, szValue, 10);//接受字符的行数
	strHead += szValue;	//字符的行树
	strHead += '\n';	//换行隔开信息
	strHead += strChCount; //字符的种类个数
	//写信息
	fwrite(strHead.c_str(),1,strHead.size(),fOut);
}

//计算字符编码
void FileCompressHuff::GenerateHuffManCode(HuffManTreeNode<CharInfo>* pRoot)
{
	if (nullptr == pRoot)
		return;
	GenerateHuffManCode(pRoot->_pLeft);
	GenerateHuffManCode(pRoot->_pRight);

	//找到叶子结点
	if (nullptr == pRoot->_pLeft&&pRoot->_pRight == nullptr)
	{
		string& strCode = _fileInfo[pRoot->_weight._ch]._strCode;
		HuffManTreeNode<CharInfo>*  pCur = pRoot;
		HuffManTreeNode<CharInfo>*  pParent = pCur->_pParent;

		while (pParent)
		{
			if (pCur == pParent->_pLeft)	//左为0
			{
				strCode += '0';
			}
			else							//右为1
			{
				strCode += '1';
			}
			pCur = pParent;
			pParent = pCur->_pParent;
		}
		reverse(strCode.begin(), strCode.end());
		//_fileInfo[pRoot->_weight._ch]._strCode = strCode;
	}
}