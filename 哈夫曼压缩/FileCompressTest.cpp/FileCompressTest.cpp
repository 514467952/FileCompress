
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