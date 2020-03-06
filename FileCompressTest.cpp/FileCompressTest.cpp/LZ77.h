#pragma once

#include"LZHashTable.h"
#include<string>
class LZ77
{
public:
	LZ77();
	~LZ77();
	void CompressFile(const std::string& strFilePath);
	void UNCompressFile(const std::string& strFilePath);
	USH LongetMatch(USH matchHead, USH& MatchDist,USH start);
	void WriteFlage(FILE* fOut, UCH& chFalg, USH& bitCount, bool isLen);
	void MergeFile(FILE* fOut,ULL fileSize);
	void FillWindow(FILE* fIn,size_t& lookAhead,USH& start);
private:
	UCH* _pWin; //用来保存待压缩数据的缓冲区
	LZHashTable _ht;
};