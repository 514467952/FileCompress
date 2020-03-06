# if 0
//#include"HuffmanTree.hpp"
#include"FileCompressHuff.h"
int main()
{
	//TestHuffmanTree();
	FileCompressHuff fc;
	fc.CompressFile("1.txt");
	fc.UNComPressFile("2.txt");
	system("pause");
	return 0;
}
#endif

#include"LZ77.h"
int main()
{
	LZ77 lz;
	lz.CompressFile("1.txt");
	lz.UNCompressFile("2.lzp");
	return 0;
}