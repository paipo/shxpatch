// test1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ShapeFont.h"
#include "fontchrlink.h"
int _tmain(int argc, _TCHAR* argv[])
{
	ShapeFont* m_font = new ShapeFont();
	if(m_font->Load("HT.SHX")){
		printf("已打开");
		printf("%i",m_font->Count());//字库包含多少字符
		//开始获得路径 例 '好' 字
		CString str("好");
		//获得 '好' 字编码
		char* kk=str.GetBuffer(0);
		wchar_t buf1[100];
		char key[8];
		MultiByteToWideChar(CP_OEMCP,0,str.GetBuffer(0),6,buf1,100);
		WideCharToMultiByte(932,0,buf1,3,key,6,NULL,NULL);
		unsigned short k1;
		memcpy(&k1,&kk[0],2);
		//获得shx字体路径
		string patch = m_font->Patch(k1,20,20,1,1);
		printf("%s",patch.c_str());
	}
	else{
		printf("字库打开错误");
	}

	return 0;
}

