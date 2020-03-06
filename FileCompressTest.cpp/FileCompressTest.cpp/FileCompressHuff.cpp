#define _CRT_SECURE_NO_WARNINGS

#include"FileCompressHuff.h"
#include<assert.h>
#include"HuffmanTree.hpp"

//���캯��
FileCompressHuff::FileCompressHuff()
{
	_fileInfo.resize(256);
	for (int i = 0; i < 256; ++i)
	{
		_fileInfo[i]._ch = i;
		_fileInfo[i]._count = 0;
	}
}
//ѹ���ļ�
void FileCompressHuff::CompressFile(const string& path)
{
	//1.ͳ��Դ�ļ���ÿ���ַ����ֵĴ���
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
		//���ļ�
		rdSize = fread(pReadBuff, 1, 1024, fIn);	
		if (0 == rdSize)
			break;
		//ͳ��
		for (int i = 0; i < rdSize; ++i)
		{
			_fileInfo[pReadBuff[i]]._count++;
		}
	}
	
	//2.����Щ���ֵĴ���ΪȨֵ������������
	HuffManTree<CharInfo> t(_fileInfo,CharInfo());

	//3.��ȡÿ���ַ��ı���
	GenerateHuffManCode(t.GetRoot());
	//4.��ȡ�����ַ��������¸�д
	FILE* fOut = fopen("2.txt", "wb");  //��һ���ļ�����ѹ����Ľ��
	if (nullptr == fOut)
	{
		assert(false);
		return;
	}

	WriteHead(fOut, path);

	fseek(fIn,0,SEEK_SET); //���ļ�ָ���ٴηŵ��ļ�����ʼλ��
	char ch = 0;	//����ַ�����
	int bitcount = 0; //������˶��ٸ�����λ
	while (true)
	{
		rdSize = fread(pReadBuff, 1, 1024,fIn);
		if (0 == rdSize)  //�ļ���ȡ����
			break;

		//�����ֽڵı���Զ�ȡ�������ݽ�����д
		for (size_t i = 0; i < rdSize; ++i)
		{
			//�õ�����
			string strCode = _fileInfo [pReadBuff[i]]._strCode;
			//A "100"
			for (size_t j = 0; j < strCode.size(); ++j)
			{
				ch <<= 1; //ÿ��һ������һλ������һ�����������
				//��ŵ�ʱ��һ��һ������λ����
				if ('1' == strCode[j])
					ch |= 1;

				bitcount++;
				if (8 == bitcount) //˵��ch���ˣ�������ֽ�д���ļ���ȥ
				{
					//fputc д�����ֽڵĺ���
					fputc(ch, fOut);
					//д��󣬶�����
					bitcount = 0;
					ch = 0;
				}

			}
		}
	}
	//���һ��ch�п��ܲ���8������λ
	if (bitcount < 8)
	{
		ch <<= 8 - bitcount; //����ʣ���λ��
		//������λ�϶�Ҫд���ļ���
		fputc(ch, fOut);
	}
	delete[]pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

//��ѹ���ļ�
void FileCompressHuff::UNComPressFile(const std::string& path)
{
	FILE* fIn = fopen(path.c_str(), "rb");
	if (nullptr == fIn)
	{
		assert(false);
		return;
	}

	//��ȡ�ļ��ĺ�׺
	string strFilePostFix;
	ReadLine(fIn,strFilePostFix);
	//��ȡ�ַ���Ϣ��������
	string strCount;
	ReadLine(fIn, strCount);
	int lineCount = atoi(strCount.c_str()); //�ܵ�����

	//��ȡ�ַ�����Ϣ
	for (int i = 0; i < lineCount; ++i)
	{
		string strchCount;
		ReadLine(fIn, strchCount); //��ÿһ�е��ַ���Ϣ
		//�����ȡ������\n
		if (strchCount.empty())
		{
			strchCount += '\n'; //��\nд���ȡ
			ReadLine(fIn, strchCount); //���һ�У���\n�Ĵ�����ð�Ŷ�ȡ
		}
		//A:100
		_fileInfo[(unsigned char)strchCount[0]]._count = atoi(strchCount.c_str() + 2);//����ǰ�����ַ�����Ϊǰ������A:
	}
	
	//��ԭHuffman��
	HuffManTree<CharInfo> t;  //����Huffamn���Ķ���
	t.CreatHuffManTree(_fileInfo, CharInfo(0)); //��ԭHuffman��


	FILE* fOut = fopen("3.txt","wb");
	assert(fOut);
	//��ѹ��
	char* pReadBuff = new char[1024];	//����������
	char ch = 0;
	HuffManTreeNode<CharInfo>* pCur = t.GetRoot();
	size_t fileSize = pCur->_weight._count; //�ļ��ܵĴ�С���Ǹ�����Ȩֵ�Ĵ���
	size_t uncount = 0;//��ʾ��ѹ���˶��ٸ�
	while (true)
	{
		size_t rdSize = fread(pReadBuff,1,1024,fIn); //������
		if (0 == rdSize)
			break;
		//һ�����ֽڽ��н�ѹ��
		for (size_t i = 0; i < rdSize; ++i)
		{
			//ֻ��Ҫ��һ���ֽ��е�8������λ��������
			ch = pReadBuff[i];
			for (int pos = 0; pos < 8; ++pos)
			{
				//����һ���ж�
				if (nullptr == pCur->_pLeft && nullptr == pCur->_pRight)
				{
					//uncount++; //ÿ�ν�ѹ��һ������++һ��
					fputc(pCur->_weight._ch, fOut);
					if (uncount == fileSize)
						break;
					//Ҷ�ӽ��,��ѹ��һ���ַ���д���ļ�			
					pCur = t.GetRoot(); //��pCur�ŵ�������λ���ϼ���
				}
				if (ch & 0x80)
					//�����λ�ϵ�����Ϊ1
					pCur = pCur->_pRight;
				else
					pCur = pCur->_pLeft;

				ch <<= 1; //����������ƶ�һλ
				if (nullptr == pCur->_pLeft && nullptr == pCur->_pRight)
				{
					uncount++; //ÿ�ν�ѹ��һ������++һ��
					fputc(pCur->_weight._ch, fOut);
					if (uncount == fileSize)
						break;
					//Ҷ�ӽ��,��ѹ��һ���ַ���д���ļ�			
					pCur = t.GetRoot(); //��pCur�ŵ�������λ���ϼ���
				}
			}//forѭ������ٶ�ȡ��һ���ֽ�
		}
	}

	delete[]pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

//��ȡѹ���ļ�����Ϣ
void FileCompressHuff::ReadLine(FILE* fIn, string& strInfo)
{
	assert(fIn);

	//��ȡһ�е��ַ�
	while (!feof(fIn))  //ֻҪ�ļ�ָ��û�е��ļ�ĩβ�Ͷ�ȡ
	{
		//��ȡһ���ַ�
		char ch = fgetc(fIn);
		if (ch == '\n')
			break;
		//��Ч���ַ���ƴ����ȥ
		strInfo += ch;
	}
}

//��ȡ�ļ��ĺ�׺
//2.txt
//F��\123\2.txt
string FileCompressHuff:: GetFilePostFix(const string& filename)
{
	//substr��ȡ�ļ����ڶ�������û�и��Ļ�Ĭ�Ͻ�ȡ��ĩβ
	return  filename.substr(filename.rfind('.'));
}

//ѹ���ļ���ʽ
void FileCompressHuff::WriteHead(FILE* fOut, const string& filename)
{
	assert(fOut);

	//д�ļ��ĺ�׺
	string strHead;
	strHead += GetFilePostFix(filename);
	strHead += '\n'; //��׺����������֮����\n����
	
	//д����
	size_t lineCount = 0;
	string strChCount;
	char szValue[32] = { 0 };	//�����������ַ�����
	for (int i = 0; i < 256; ++i)
	{
		CharInfo& charInfo = _fileInfo[i];
		if (charInfo._count)
		{
			
			lineCount++;	//����
			strChCount += charInfo._ch; //�ַ�
			strChCount += ':';	//�ַ����ַ�����֮����ð�Ÿ���
			_itoa(charInfo._count, szValue, 10);
			strChCount += szValue;		//�ַ�����
			strChCount += '\n';	//ĩβ��\n
		}
	}

	_itoa(lineCount, szValue, 10);//�����ַ�������
	strHead += szValue;	//�ַ�������
	strHead += '\n';	//���и�����Ϣ
	strHead += strChCount; //�ַ����������
	//д��Ϣ
	fwrite(strHead.c_str(),1,strHead.size(),fOut);
}

//�����ַ�����
void FileCompressHuff::GenerateHuffManCode(HuffManTreeNode<CharInfo>* pRoot)
{
	if (nullptr == pRoot)
		return;
	GenerateHuffManCode(pRoot->_pLeft);
	GenerateHuffManCode(pRoot->_pRight);

	//�ҵ�Ҷ�ӽ��
	if (nullptr == pRoot->_pLeft&&pRoot->_pRight == nullptr)
	{
		string& strCode = _fileInfo[pRoot->_weight._ch]._strCode;
		HuffManTreeNode<CharInfo>*  pCur = pRoot;
		HuffManTreeNode<CharInfo>*  pParent = pCur->_pParent;

		while (pParent)
		{
			if (pCur == pParent->_pLeft)	//��Ϊ0
			{
				strCode += '0';
			}
			else							//��Ϊ1
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