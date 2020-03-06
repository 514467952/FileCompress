#define _CRT_SECURE_NO_WARNINGS
#include"LZ77.h"
#include<iostream>
#include<assert.h>
using namespace std;

const USH MIN_LOOKAHEAD = MAX_MATCH + MIN_MATCH + 1;
const USH MAX_DIST = WSIZE - MIN_LOOKAHEAD;
LZ77::LZ77()
:_pWin(new UCH[WSIZE*2])
, _ht(WSIZE) 
{}

LZ77::~LZ77()
{
	delete[]_pWin;
	_pWin = nullptr;
}

void  LZ77::CompressFile(const std::string& strFilePath)
{
	//��ȡ�ļ���С
	FILE* fIn = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "���ļ�ʧ��" << endl;
		return;
	}

	//��ȡ�ļ���С
	fseek(fIn, 0, SEEK_END);
	ULL fileSize = ftell(fIn);

	//1.���Դ�ļ��Ĵ�СС��MIN_MATCHһ��ƥ�䳤��,�򲻽��д���
	if (fileSize <= MIN_MATCH)
	{
		cout << "�ļ�̫С����ѹ��" << endl;
		return;
	}

	//��ѹ���ļ��ж�ȡһ�������������ݵ�������
	fseek(fIn, 0, SEEK_SET);
	size_t lookAhead = fread(_pWin, 1, 2 * WSIZE, fIn);	
	USH hashAddr = 0;
	
	//����ǰ�����ֽ�...����hashAddr
	for (USH i = 0; i < MIN_MATCH - 1; ++i)
	{
		_ht.HashFunc(hashAddr, _pWin[i]);
	}

	//ѹ��
	FILE* fOUT = fopen("2.lzp", "wb");
	assert(fOUT);
	USH start = 0;

	//������ƥ����صı���
	USH matchHead = 0;
	USH curMatchLength = 0; //��ǰƥ�䳤��
	USH curMatchDist = 0;

	//��д�����صı���
	UCH chFlag = 0;
	USH bitCount = 0;
	bool IsLen = false;

	//д��ǵ��ļ�
	FILE* fOutF = fopen("3.txt", "wb");
	assert(fOutF);

	//lookAhead��ʾ���л�������ʣ���ֽڵĸ���
	while (lookAhead)
	{
		//1.����ǰ�����ַ�(start,start+1,start+2)���뵽��ϣ���У���ȡƥ��ͷ
		_ht.Insert(matchHead, _pWin[start + 2],start,hashAddr);

		curMatchLength = 0;
		curMatchDist = 0;
		//2. ��֤�ڲ��һ��������Ƿ��ҵ�ƥ�䣬�����ƥ�䣬���ƥ��
		if (matchHead)
		{
			//˳��ƥ�������ƥ��,���մ���<���ȣ�����>��
			curMatchLength = LongetMatch(matchHead, curMatchDist,start);
		}
		//3.��֤�Ƿ��ҵ�ƥ��
		if (curMatchLength < MIN_MATCH)
		{
			//�ڲ��һ�������δ�ҵ��ظ��ַ���
			// ��start λ�õ��ַ�д�뵽ѹ���ļ���
			fputc(_pWin[start], fOUT);

			//д��ǰ�ַ�ԭ�ַ���Ӧ�ı��
			WriteFlage(fOutF,chFlag,bitCount,false );
			++start;
			lookAhead--;
		}
		else
		{
			//�ҵ�ƥ��
			//��<����,����>��д�뵽ѹ���ļ���

			//д����
			UCH chLen = curMatchLength - 3;
			fputc(chLen, fOUT);
			//д����
			fwrite(&curMatchDist, sizeof(curMatchDist), 1, fOUT);

			//д��ǰ��Ӧ�ı��
			WriteFlage(fOutF, chFlag, bitCount, true);

			//�������л�������ʣ����ֽ���,curMatchLength�Ѿ�������ˣ��ͼ�ȥ
			lookAhead -= curMatchLength;

			//���Ѿ�ƥ����ַ�����������һ�齫����뵽��ϣ����
			--curMatchLength;//��ǰ�ַ����Ѿ��������
			while (curMatchLength)
			{
				start++;
				_ht.Insert(matchHead, _pWin[start+2], start, hashAddr);
				curMatchLength--;
			}
			++start; //ѭ����start�ټ���һ��
		}

		//������л�������ʣ���ַ��ĸ���
		//      1.ѹ���ļ�����64K����window���ַ�ѹ����С�ڵ���MIN_LOOKAHEAD ��ȷ
		//      2.ѹ���ļ�С��64K||�����Ѿ������ļ�ĩβ������Ҫ���
		//       ���1. start>=WSIZE
		//       ���2. start < WSIZE
		if (lookAhead <= MIN_LOOKAHEAD)
			FillWindow(fIn,lookAhead,start);
	}
	//���λ���������8������λ:
	if (bitCount > 0 && bitCount < 8)
	{
		chFlag <<= (8 - bitCount);
		fputc(chFlag, fOutF);
	}
	fclose(fOutF);

	//�ϲ�ѹ���ļ�
	MergeFile(fOUT, fileSize);
	fclose(fIn);
	fclose(fOUT);

	//��������������Ϣ����ʱ�ļ�ɾ����
}
void  LZ77::FillWindow(FILE* fIn, size_t& lookAhead, USH& start)
{
	//ѹ���Ѿ����е��Ҵ������л�����ʣ�����ݲ���MIN_LOOKAHEAD
	if (start >= WSIZE)
	{
		//1.���Ҵ������ݰ��Ƶ���
		memcpy(_pWin, _pWin + WSIZE, WSIZE);
		memset(_pWin + WSIZE, 0, WSIZE);
		start -= WSIZE;

		//2.���¹�ϣ��
		_ht.Update();

		//3.���Ҵ��в���һ��WSIZE����ѹ������
		if (!feof(fIn)) // �ļ�ָ��û�ߵ�ĩβ
			lookAhead = fread(_pWin + WSIZE, 1, WSIZE, fIn);
	}
}
void LZ77::MergeFile(FILE* fOut,ULL fileSize)
{
	//��ѹ�������ļ��ͱ����Ϣ�ϲ�
	//1.��ȡ�����Ϣ�ļ������ݣ�Ȼ�󽫽��д�뵽ѹ���ļ���
	FILE* fInF = fopen("3.txt", "rb");
	size_t flagSize = 0;
	UCH* pReadbuff = new UCH[1024];
	while (true)
	{
		size_t rdSize = fread(pReadbuff, 1, 1024, fInF);
		if (0 == rdSize)
			break;

		fwrite(pReadbuff, 1, rdSize,fOut);
		flagSize += rdSize;
	}
	//2. ��������Ϣ�ֽ���
	//����ֽ���
	fwrite(&flagSize, sizeof(flagSize), 1, fOut);
	//�ļ���С
	fwrite(&fileSize, sizeof(fileSize), 1, fOut);
	fclose(fInF);
	delete[]pReadbuff;
}

//chFlag:���ֽ��е�ÿ������λ���������ֵ�ǰ�ַ���ԭ�ַ����ǳ���
//0������ԭ�ַ�
//1��������

//bitCount:���ֽ��еĶ��ٱ���λ�Ѿ�������
//isLen:������ֽ���ԭ�ַ����ǳ���
//����---- ��  ���ļ�û����
//���---����Ҫ�����ô��Σ����޸ĺ��ֵ����ȥ
void LZ77::WriteFlage(FILE* fOut, UCH& chFalg, USH& bitCount, bool isLen)
{
	chFalg <<= 1;
	if (isLen)
		chFalg |= 1;
	bitCount++;

	//��ǰ����ֽ��е�8������λ�Ѿ������ˣ�д�룬������Ϊ0
	if (bitCount == 8)
	{
		//���ñ��д�뵽ѹ���ļ���
		fputc(chFalg, fOut);
		chFalg = 0;
		bitCount = 0;
	}
}


//ƥ��:���ڲ��һ������н��еģ����һ��������ҵ����ƥ��
//�������Ҫ�ƥ��
//ע�⣺���ܻ�������״��----�������������ƥ�����
		//ƥ������MAX_DIST��Χ�ڽ���ƥ��ģ�̫Զ�ľ��벻����ƥ��


//���ҵĹ����У���Ҫ��ÿ���ҵ���ƥ�������бȶԣ������ƥ��
USH LZ77:: LongetMatch(USH matchHead, USH& MatchDist,USH start)
{
	USH curMatchLen = 0; //һ��ƥ��ĳ���
	USH maxMatchLen = 0; //����ƥ�䳤��
	UCH maxMatchCount = 255;//����ƥ������������״��
	USH curMatchStart = 0; //��ǰƥ���ڲ��һ���������ʼλ��

	//�����л������в���ƥ��ʱ������̫Զ�����ܳ���MAX_DIST
	USH limit = start > MAX_DIST ? start - MAX_DIST : 0;

	do
	{
		// ƥ�䷶Χ
		// ���л���������ʼλ��
		UCH* pstart = _pWin + start;
		// ���л�����ĩβλ��
		UCH* pend = pstart + MAX_MATCH;

		//���һ�����ƥ�䴮����ʼλ��
		UCH* pMatchStart = _pWin + matchHead;

		//��ǰƥ�䳤��ÿ�ض�Ҫ����Ϊ0��Ҫ��Ȼ����ƥ�䳤�ȶ��ӵ�һ����
		curMatchLen = 0;

		//���Խ���ƥ��
		//���л�����ÿ��ĩβ�����ַ����
		while (pstart < pend && *pstart == *pMatchStart)
		{
			curMatchLen++;
			pstart++;
			pMatchStart++;
		}//һ��ƥ�����

		//ƥ�䳤�ȳ����ƥ�䳤��
		if (curMatchLen>maxMatchLen)
		{
			//�����ƥ�䳤��
			maxMatchLen = curMatchLen;
			//������ʼλ��
			curMatchStart = matchHead;
		}
	} while ((matchHead = _ht.GetNext(matchHead)) > limit && maxMatchCount--); //ÿ����һ�Σ�����ƥ�����--

	//start-��ǰƥ�����ʼλ��
	MatchDist = start - curMatchStart;

	return maxMatchLen;
}


void LZ77::UNCompressFile(const std::string& strFilePath)
{
	//��ѹ���ļ�
	FILE* fInD = fopen(strFilePath.c_str(), "rb");
	if (fInD == nullptr)
	{
		cout << "ѹ���ļ���ʧ��" << endl;
		return;
	}
	
	// ������ǵ��ļ�ָ��
	FILE* fInF = fopen(strFilePath.c_str(), "rb");
	if (fInF == nullptr)
	{
		cout << "��Ǵ�ʧ��" << endl;
		return;
	}
	//��ȡԴ�ļ��Ĵ�С
	ULL fileSize = 0;
	fseek(fInF, 0 - sizeof(fileSize), SEEK_END);
	fread(&fileSize, sizeof(fileSize), 1, fInF);

	//��ȡ�����Ϣ��С
	size_t flagSize = 0;
	fseek(fInF,0-sizeof(fileSize)-sizeof(flagSize), SEEK_END);
	fread(&flagSize, sizeof(flagSize), 1, fInF);

	//����ȡ�����Ϣ���ļ�ָ���ƶ������������ݵ���ʼλ��
	//�����Ѿ���ȡ�˱����Ϣ�Ĵ�С�����Ե�����ǰƫ�Ʊ����Ϣ��С���ֽ�
	//Ȼ����ƫ�Ʊ����Ϣ�Ĵ�С���ƶ��������Ϣ����ʼλ��
	fseek(fInF, 0 - sizeof(flagSize)-sizeof(fileSize)-flagSize, SEEK_END);

	//��ʼ��ѹ��
	//д��ѹ��������
	FILE* fOut = fopen("4.txt", "wb");
	assert(fOut);

	//����ƥ��ǰ��
	FILE* fR = fopen("4.txt", "rb");

	UCH bitCount = 0;
	UCH chFalg = 0;
	ULL encodeCount = 0;//�Ѿ���ѹ������ֽ�
	while (encodeCount < fileSize) 
	{
		if (0 == bitCount)
		{
			//�ȶ�ȡһ���ֽ�
			chFalg =  fgetc(fInF);
			bitCount = 8;
		}

		if (chFalg & 0x80)
		{
			//�Ǿ��볤�ȶ�
			//��ȡ����
			USH matchLen = fgetc(fInD) + 3;
			//��ȡ����
			USH matchDist = 0;
			fread(&matchDist, sizeof(matchDist), 1, fInD);

			//��ջ�������ϵͳ�ѻ������е����ݷŵ��ļ���
			fflush(fOut);

			//�����Ѿ�����ĳ���
			encodeCount += matchLen;

			//ȥǰ������ƥ��
			//fR:��ȡƥ�䴮�е�����
			//��ĩβ��ǰƫ��0-matchDist
			fseek(fR, 0-matchDist, SEEK_END);
			UCH ch;
			while (matchLen)
			{
				ch = fgetc(fR);
				fputc(ch,fOut);
				matchLen--;
				fflush(fOut);
			}
		}
		else
		{
			//��ԭ�ַ�
			UCH ch = fgetc(fInD);
			fputc(ch, fOut);
			encodeCount += 1;
			
		}

		chFalg <<= 1;
		bitCount--;
	}

	fclose(fInD);
	fclose(fInF);
	fclose(fOut);
	fclose(fR);
}