# ZYshxpatch
识别shx字体，返回路径，可以供二次开发代码使用，如：绘制路径，按shx字体显示文字等。
## 路径与数值定义
### pendown 
	落笔
### penup 
	起笔
### moveto:12.0000,22.0000 
	移动到:x坐标,y坐标
### lineto:42.0000,62.0000 
	直线画到:x坐标,y坐标
### anglearc:12.0000,12.0000,25.0000,45.0000,45.0000 
	画圆弧:圆心x坐标,圆心y坐标,圆的半径,弧的起点角度,扇形的角度
### 返回路径字符串例子
	pendown|penup|moveto:12.0000,22.0000|lineto:42.0000,62.0000|anglearc:12.0000,12.0000,25.0000,45.0000,45.0000
## c++ 测试
```c++
int _tmain(int argc, _TCHAR* argv[])
{
	ShapeFont* m_font = new ShapeFont();
	if(m_font->Load("HT.SHX")){
		printf("已打开");
		printf("%i",m_font->Count());
		string patch = m_font->Patch(0x41,0,0,1,1);
		printf("%s",patch.c_str());
	}
	else{
		printf("字库打开错误");
	}
	char* str = "";
	scanf_s("%s",str);
	return 0;
}
```

## c#  调用 dll 
```c#
[DllImport("ShxPatch.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
static extern IntPtr GetPatch(String file, UInt16 code, Single orgX, Single orgY, Single scX, Single scY);

//获得字符编码
byte[] big5 = System.Text.Encoding.GetEncoding("GBK").GetBytes("好");
ushort code = big5[0];
if (big5.Length > 1)
{
	code = (ushort)System.BitConverter.ToInt32(new byte[] { big5[0], big5[1], 0x00, 0x00 }, 0);
}
//获得路径
IntPtr ipt = GetPatch("HT.SHX", code, 0, 0, 1, 1);
```

## GetPatch 路径函数说明	
* file shx 字体文件路径
* code 字符编码
* orgX x坐标偏移量
* orgX y坐标偏移量
* scX x方向缩放大小
* scY y方向缩放大小

## 注意
* code 字符编码根据中文和英文不同，如'A'为 65 0x41,'好'为 50106 0xBAC3
* shx字体文件不包含此字符编码时，返回路径为长度为0字符串
