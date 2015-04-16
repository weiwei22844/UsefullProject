
/************************ Copyright (c) 2006 ************************

程序作者:

    张鲁夺(zhangluduo) : 为所有爱我的人和我爱的人努力!

联系方式:

    zhangluduo@msn.com
	[oopfans]群:34064264

修改时间:

    2007-03-04

功能描述:

    Base64编码和解码

版权声明:

    许可任何单位,个人随意使用,拷贝,修改,散布及出售这份代码,但是必须保
    留此版权信息,以慰藉作者辛勤的劳动,及表明此代码的来源,如若此份代码
    有任何BUG,请通知作者,以便弥补作者由于水平所限而导致的一些错误和不
    足,谢谢!

	注:
	先前写的代码有Bug,不能对文件进行base64计算,后根据网上例子修改而来!

************************ Copyright (c) 2006 ************************/

#ifndef _ZBASE64
#define _ZBASE64

#pragma warning(disable:4786) 
#include <string>
using namespace std;

class ZBase64
{

public:

	/*编码
	DataByte
		[in]输入的数据长度,以字节为单位
	*/
	string Encode(const unsigned char* Data,int DataByte);

	/*解码
	DataByte
		[in]输入的数据长度,以字节为单位
	OutByte
		[out]输出的数据长度,以字节为单位,请不要通过返回值计算
		输出数据的长度
	*/
	string Decode(const char* Data,int DataByte,int& OutByte);
};
#endif