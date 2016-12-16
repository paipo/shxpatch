// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FONTLIB_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FONTLIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once

#include "Array.h"
typedef xArray<float> vex2Ds;
class  CharData
{
public:
	CharData(){}
	GLYPHMETRICS m_charInfo;//字体信息
	xArray<vex2Ds> m_parts;
};

enum FontType {SHAPE_1_0, SHAPE_1_1, UNIFONT, BIGFONT, TRUETYPE};

class IFontBase {
public:	
	
	//功能：自我删除，释放空间
	virtual void Release()=0;
	virtual const char* GetName()=0;//名字
	//功能：从字体文件加载数据到内存
	//@fileName:字体文件完整路径
	//@return:true-加载成功，false-加载失败
	virtual bool LoadFromFile(const char* fileName,long styleNum=0x8602,HDC dc=NULL)=0;

	//功能：根据字体编码提取字对于的矢量信息
	//@charCode:字编码（支持单字节和双字节文字）
	//@pOut:输出二维坐标列表(XY,XY，...)
	//@reurn:true-解析成功，false-不支持的文字或解析失败
	virtual CharData* GetCharData(unsigned short charCode)=0;

	//属性：文字类型
	virtual FontType get_FontType()=0;

	//属性：文字行高
	virtual float Height()=0;
};


////Shape字体
//class DISPLAYLIB_API Font_Shape:public IFontBase
//{
//
//};
//
