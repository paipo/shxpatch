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

## c++ 测试
```c++
int _tmain(int argc, _TCHAR* argv[])
{
	ShapeFont* m_font = new ShapeFont();
	if(m_font->Load("Simplex.shx")){
		printf("已打开");
		printf("%i",m_font->Count());
		string patch = m_font->Patch(0x41,20,20,1,1);
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
