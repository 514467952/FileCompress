#pragma once

//为了书写方便
typedef unsigned char UCH;
typedef unsigned short USH;
typedef unsigned long long ULL; //文件比较大


//最小匹配的字符串长度，从3个字符开始匹配
const USH MIN_MATCH = 3;
//最大的匹配，0代表3个字符匹配，255可以代表258个字符匹配
const USH MAX_MATCH = 258;
const USH WSIZE = 32 * 1024;
