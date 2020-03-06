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
	//获取文件大小
	FILE* fIn = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "打开文件失败" << endl;
		return;
	}

	//获取文件大小
	fseek(fIn, 0, SEEK_END);
	ULL fileSize = ftell(fIn);

	//1.如果源文件的大小小于MIN_MATCH一个匹配长度,则不进行处理
	if (fileSize <= MIN_MATCH)
	{
		cout << "文件太小，不压缩" << endl;
		return;
	}

	//从压缩文件中读取一个缓冲区的数据到窗口中
	fseek(fIn, 0, SEEK_SET);
	size_t lookAhead = fread(_pWin, 1, 2 * WSIZE, fIn);	
	USH hashAddr = 0;
	
	//处理前两个字节...设置hashAddr
	for (USH i = 0; i < MIN_MATCH - 1; ++i)
	{
		_ht.HashFunc(hashAddr, _pWin[i]);
	}

	//压缩
	FILE* fOUT = fopen("2.lzp", "wb");
	assert(fOUT);
	USH start = 0;

	//与查找最长匹配相关的变量
	USH matchHead = 0;
	USH curMatchLength = 0; //当前匹配长度
	USH curMatchDist = 0;

	//与写标记相关的变量
	UCH chFlag = 0;
	USH bitCount = 0;
	bool IsLen = false;

	//写标记的文件
	FILE* fOutF = fopen("3.txt", "wb");
	assert(fOutF);

	//lookAhead表示先行缓冲区中剩余字节的个数
	while (lookAhead)
	{
		//1.将当前三个字符(start,start+1,start+2)插入到哈希表中，获取匹配头
		_ht.Insert(matchHead, _pWin[start + 2],start,hashAddr);

		curMatchLength = 0;
		curMatchDist = 0;
		//2. 验证在查找缓冲区中是否找到匹配，如果有匹配，找最长匹配
		if (matchHead)
		{
			//顺着匹配链找最长匹配,最终带出<长度，距离>对
			curMatchLength = LongetMatch(matchHead, curMatchDist,start);
		}
		//3.验证是否找到匹配
		if (curMatchLength < MIN_MATCH)
		{
			//在查找缓冲区中未找到重复字符串
			// 将start 位置的字符写入到压缩文件中
			fputc(_pWin[start], fOUT);

			//写当前字符原字符对应的标记
			WriteFlage(fOutF,chFlag,bitCount,false );
			++start;
			lookAhead--;
		}
		else
		{
			//找到匹配
			//将<长度,距离>对写入到压缩文件中

			//写长度
			UCH chLen = curMatchLength - 3;
			fputc(chLen, fOUT);
			//写距离
			fwrite(&curMatchDist, sizeof(curMatchDist), 1, fOUT);

			//写当前对应的标记
			WriteFlage(fOutF, chFlag, bitCount, true);

			//更新先行缓冲区中剩余的字节数,curMatchLength已经处理过了，就减去
			lookAhead -= curMatchLength;

			//将已经匹配的字符串按照三个一组将其插入到哈希表中
			--curMatchLength;//当前字符串已经插入过了
			while (curMatchLength)
			{
				start++;
				_ht.Insert(matchHead, _pWin[start+2], start, hashAddr);
				curMatchLength--;
			}
			++start; //循环中start少加了一次
		}

		//检测先行缓冲区中剩余字符的个数
		//      1.压缩文件大于64K，将window中字符压缩到小于等于MIN_LOOKAHEAD 正确
		//      2.压缩文件小于64K||现在已经处理到文件末尾，不需要填充
		//       情况1. start>=WSIZE
		//       情况2. start < WSIZE
		if (lookAhead <= MIN_LOOKAHEAD)
			FillWindow(fIn,lookAhead,start);
	}
	//标记位数如果不够8个比特位:
	if (bitCount > 0 && bitCount < 8)
	{
		chFlag <<= (8 - bitCount);
		fputc(chFlag, fOutF);
	}
	fclose(fOutF);

	//合并压缩文件
	MergeFile(fOUT, fileSize);
	fclose(fIn);
	fclose(fOUT);

	//将用来保存标记信息的临时文件删除掉
}
void  LZ77::FillWindow(FILE* fIn, size_t& lookAhead, USH& start)
{
	//压缩已经进行到右窗，先行缓冲区剩余数据不够MIN_LOOKAHEAD
	if (start >= WSIZE)
	{
		//1.将右窗的数据搬移到左窗
		memcpy(_pWin, _pWin + WSIZE, WSIZE);
		memset(_pWin + WSIZE, 0, WSIZE);
		start -= WSIZE;

		//2.更新哈希表
		_ht.Update();

		//3.向右窗中补充一个WSIZE个待压缩数据
		if (!feof(fIn)) // 文件指针没走到末尾
			lookAhead = fread(_pWin + WSIZE, 1, WSIZE, fIn);
	}
}
void LZ77::MergeFile(FILE* fOut,ULL fileSize)
{
	//将压缩数据文件和标记信息合并
	//1.读取标记信息文件中内容，然后将结果写入到压缩文件中
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
	//2. 保存标记信息字节数
	//标记字节数
	fwrite(&flagSize, sizeof(flagSize), 1, fOut);
	//文件大小
	fwrite(&fileSize, sizeof(fileSize), 1, fOut);
	fclose(fInF);
	delete[]pReadbuff;
}

//chFlag:该字节中的每个比特位是用来区分当前字符是原字符还是长度
//0：代表原字符
//1：代表长度

//bitCount:该字节中的多少比特位已经被设置
//isLen:代表该字节是原字符还是长度
//问题---- 标  记文件没内容
//解决---参数要用引用传参，把修改后的值带出去
void LZ77::WriteFlage(FILE* fOut, UCH& chFalg, USH& bitCount, bool isLen)
{
	chFalg <<= 1;
	if (isLen)
		chFalg |= 1;
	bitCount++;

	//当前这个字节中的8个比特位已经用完了，写入，重新置为0
	if (bitCount == 8)
	{
		//将该标记写入到压缩文件中
		fputc(chFalg, fOut);
		chFalg = 0;
		bitCount = 0;
	}
}


//匹配:是在查找缓冲区中进行的，查找缓冲区中找到多个匹配
//输出：需要最长匹配
//注意：可能会遇到环状链----解决：设置最大的匹配次数
		//匹配是在MAX_DIST范围内进行匹配的，太远的距离不进行匹配


//在找的过程中，需要将每次找到的匹配结果进行比对，保存最长匹配
USH LZ77:: LongetMatch(USH matchHead, USH& MatchDist,USH start)
{
	USH curMatchLen = 0; //一次匹配的长度
	USH maxMatchLen = 0; //最大的匹配长度
	UCH maxMatchCount = 255;//最大的匹配次数，解决环状链
	USH curMatchStart = 0; //当前匹配在查找缓冲区的起始位置

	//在先行缓冲区中查找匹配时，不能太远即不能超过MAX_DIST
	USH limit = start > MAX_DIST ? start - MAX_DIST : 0;

	do
	{
		// 匹配范围
		// 先行缓冲区的起始位置
		UCH* pstart = _pWin + start;
		// 先行缓冲区末尾位置
		UCH* pend = pstart + MAX_MATCH;

		//查找缓冲区匹配串的起始位置
		UCH* pMatchStart = _pWin + matchHead;

		//当前匹配长度每回都要重置为0，要不然所有匹配长度都加到一块了
		curMatchLen = 0;

		//可以进行匹配
		//先行缓冲区每到末尾并且字符相等
		while (pstart < pend && *pstart == *pMatchStart)
		{
			curMatchLen++;
			pstart++;
			pMatchStart++;
		}//一次匹配结束

		//匹配长度超过最长匹配长度
		if (curMatchLen>maxMatchLen)
		{
			//更新最长匹配长度
			maxMatchLen = curMatchLen;
			//更新起始位置
			curMatchStart = matchHead;
		}
	} while ((matchHead = _ht.GetNext(matchHead)) > limit && maxMatchCount--); //每设置一次，最大的匹配次数--

	//start-当前匹配的起始位置
	MatchDist = start - curMatchStart;

	return maxMatchLen;
}


void LZ77::UNCompressFile(const std::string& strFilePath)
{
	//打开压缩文件
	FILE* fInD = fopen(strFilePath.c_str(), "rb");
	if (fInD == nullptr)
	{
		cout << "压缩文件打开失败" << endl;
		return;
	}
	
	// 操作标记的文件指针
	FILE* fInF = fopen(strFilePath.c_str(), "rb");
	if (fInF == nullptr)
	{
		cout << "标记打开失败" << endl;
		return;
	}
	//获取源文件的大小
	ULL fileSize = 0;
	fseek(fInF, 0 - sizeof(fileSize), SEEK_END);
	fread(&fileSize, sizeof(fileSize), 1, fInF);

	//获取标记信息大小
	size_t flagSize = 0;
	fseek(fInF,0-sizeof(fileSize)-sizeof(flagSize), SEEK_END);
	fread(&flagSize, sizeof(flagSize), 1, fInF);

	//将读取标记信息的文件指针移动到保存标记数据的起始位置
	//上面已经读取了标记信息的大小，所以得再往前偏移标记信息大小得字节
	//然后再偏移标记信息的大小，移动到标记信息的起始位置
	fseek(fInF, 0 - sizeof(flagSize)-sizeof(fileSize)-flagSize, SEEK_END);

	//开始解压缩
	//写解压缩的数据
	FILE* fOut = fopen("4.txt", "wb");
	assert(fOut);

	//用来匹配前文
	FILE* fR = fopen("4.txt", "rb");

	UCH bitCount = 0;
	UCH chFalg = 0;
	ULL encodeCount = 0;//已经解压缩完的字节
	while (encodeCount < fileSize) 
	{
		if (0 == bitCount)
		{
			//先读取一个字节
			chFalg =  fgetc(fInF);
			bitCount = 8;
		}

		if (chFalg & 0x80)
		{
			//是距离长度对
			//读取长度
			USH matchLen = fgetc(fInD) + 3;
			//读取距离
			USH matchDist = 0;
			fread(&matchDist, sizeof(matchDist), 1, fInD);

			//清空缓冲区，系统把缓冲区中的数据放到文件中
			fflush(fOut);

			//更新已经解码的长度
			encodeCount += matchLen;

			//去前文中找匹配
			//fR:读取匹配串中的内容
			//从末尾往前偏移0-matchDist
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
			//是原字符
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