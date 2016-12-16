#include "StdAfx.h"
#include "fontchrlink.h"
#include ".\shapefont.h"
#include"math.h"
#define PI 3.1415926
#define strnsame(a,b,c) (!strncmp((a),(b),(c)))
#define strnisame(a,b,c) (!strnicmp((a),(b),(c)))
short getfontval(char **lpp) ;
#define FONTHEADER 40

short ic_bulge2arc(double* p0, double* p1, double bulge,
				   double* cc, double *rr, double *sa, double *ea);

int strnicmp(const char *s1, const char *s2, int len)  
{  
	unsigned char c1, c2;  
	if(!len)  
		return 0;  
	do{  
		c1 = *s1++;  
		c2 = *s2++;  
		if (!c1 || !c2)  
			break;  
		if (c1 == c2)  
			continue;  
		c1 = tolower(c1);  
		c2 = tolower(c2);  
		if (c1 != c2)  
			break;  
	}while(--len);  
	return (int)c1 - (int)c2;  
}  

ShapeFont::ShapeFont(void)
{
	m_height=1024;
	m_desc=NULL;
	esc=NULL;
}

ShapeFont::~ShapeFont(void)
{	
	for(int i=0;i< m_data.Count();i++)
	{
		fontchrlink* link=m_data[i];
		if(link!=NULL)delete link;
	}
	m_data.Clear();
	if(m_desc!=NULL)
		delete[] m_desc;
	if(esc!=NULL)delete[] esc;
	m_desc=NULL;
	esc=NULL;
}

bool ShapeFont::Load(const char* fileName)
{			
	CFile pFile;
	if(!pFile.Open(fileName,CFile::modeRead))
	{
		//pFile.Close();
		return false;
	}

	if(pFile.GetLength()<FONTHEADER)
	{
		pFile.Close();
		return false;
	}

	int fileType=0;
	char head[FONTHEADER];	
	pFile.Read(head,FONTHEADER);
	if (strnsame(head,"AutoCAD-86 shapes 1.",20))
	{//SHAPE 1.0/1.1
		fileType=2;
		ReadShapes(pFile,head);

		m_type =head[20]=='1'? SHAPE_1_1:SHAPE_1_0;
	}
	else if (strnsame(head,"AutoCAD-86 unifont 1.0\r\n\x1A",25))
	{//unifont
		fileType=3;		

		ReadUnifont(pFile,head);//把已经读出的头信息传递,减少重复读取
		m_type =UNIFONT;

	}
	else if (strnsame(head,"AutoCAD-86 bigfont 1.0\r\n\x1A"/*DNT*/,25))
	{//bigfont
		fileType=4;
		m_type =BIGFONT;
		ReadBigfont(pFile,head);
	}
	else 
	{//SHP file
		ReadSHP(pFile,head,fileType);


	}

	pFile.Close();
	fontchrlink* link= GetFromCode('A');
	if(link==NULL)return true;
	CharData* pData= new CharData();
	link->ShapeCreateVec(pData);
	m_height= pData->m_charInfo.gmCellIncY;
	link->m_dataPtr=pData;
	return true;
}

fontchrlink* ShapeFont::GetFromName(const char* symbolName)
{	
	CString key(symbolName);
	for(int i=0;i<m_data.Count();i++)
	{
		fontchrlink* link= m_data[i];
		if(link==NULL|| link->symbolName==NULL)continue;
		if(key.CompareNoCase(link->symbolName)==0)
			return link;
	}

	return NULL;
}

fontchrlink* ShapeFont::GetFromCode(unsigned short code)
{
	//if(code=='?')code=0xB9F0;
	//if(code>=m_data.Count())
	//	return NULL;
	if(code>0&&code<256)return m_data[code];
	else
	{
		for(int i=256;i<m_data.Count();i++)
		{				
			if((m_data[i]->code)==(code))
				return m_data[i];
		}
	}

	return NULL;
}

int ShapeFont::FindCodeByName(const char* name)
{
	for(int i = 0; i <m_data.Count(); ++i)
	{
		fontchrlink* data=m_data[i];
		if (data)
		{
			if(data->symbolName && strnicmp(data->symbolName, name, 2049) == 0)
				return data->code;
		}

	}
	return 0;
}

bool ShapeFont::ReadUnifont(CFile& pFile,char* head)
{

	//1.文件头解析
	long nchrsinfile=0;//字体的数量
	short fi1;//数据偏移量
	memcpy(&nchrsinfile,&head[25],4);
	memcpy(&fi1,&head[25 + 4],sizeof(fi1));
	int def1seek=25+ 6 + fi1;//数据开始位置 

	fi1-=6;//描述信息的长度
	char* desc= new char[fi1];
	pFile.Seek(25+6,SEEK_SET);
	pFile.Read(desc,fi1);
	int fi2=0;while (fi2<fi1&&desc[fi2]) fi2++;
	m_desc =new char[fi2+1];
	memcpy(m_desc,desc,fi2);
	m_desc[fi2]='\0';//字体文件描述信息
	delete[] desc;

	/* 读取文件头参数 Get the header bytes: */
	char ln[6];
	pFile.Read(ln,6);	
	above   =ln[0];
	below   =ln[1];
	modes   =ln[2];
	encoding=ln[3];
	m_type    =ln[4];

	//////////////////////////////////////////////////////////////////////////
	//2.读取字体信息数据

	m_data.resize(256);
	for(int i=0;i<m_data.Count();i++)m_data[i]=NULL;	
	pFile.Seek(def1seek,SEEK_SET);
	long fileLenght= pFile.GetLength();
	char temp[2048];
	while (pFile.GetPosition()< (fileLenght-4))
	{
		int g=pFile.GetPosition();
		char ln[4];
		pFile.Read(ln,4);
		short code,descLen;//字体编号，描述信息长度
		memcpy(&code,ln,2);
		memcpy(&descLen,&ln[2],2);

		if(pFile.GetPosition()+ descLen>fileLenght) break;
		fontchrlink* link= new fontchrlink();
		link->code=code;		
		//char* temp= new char[descLen];
		pFile.Read(temp,descLen);

		//提取symbolName，线型样式用到
		int k=0; while(k<descLen&&temp[k]) k++;
		k++;
		if(k<descLen)
		{//
			if(k>1)
			{
				link->symbolName= new char[k];
				memcpy(link->symbolName,temp,k);	
			}
		}
		else
			k=0;

		//提取字体矢量数据
		link->defsz=descLen-k;
		link->def=new char[descLen-k];
		memcpy(link->def,&temp[k],descLen-k);	
		if(code>0&&code<256) 
		{
			if(m_data[code]!=NULL)delete m_data[code];
			m_data[code]=link;
		}
		else
			m_data.Add(link);
	}

	return true;
}

bool ShapeFont::ReadShapes(CFile& pFile,char* head)
{
	//1.文件头解析
	long nchrsinfile=0;//字体的数量
	short fi1;//数据偏移量
	memcpy(&fi1,&head[28],sizeof(fi1));
	nchrsinfile=fi1;
	int def1seek=30L+nchrsinfile*4;

	struct lendatalink { short code,nbytes; } *lendata;
	lendata= new struct lendatalink [nchrsinfile];
	memset(lendata,0,sizeof(struct lendatalink)*nchrsinfile);
	short shapedef=1;
	char ln[512];
	pFile.Seek(30,SEEK_SET);
	for (fi1=0; fi1<nchrsinfile; fi1++)
	{
		pFile.Read(ln,4);

		memcpy(&lendata[fi1].code  ,ln  ,sizeof(short));
		memcpy(&lendata[fi1].nbytes,ln+2,sizeof(short));
		if (!fi1 && !lendata[fi1].code) shapedef=0;  /* Font */

	}

	if (shapedef) {
		m_desc=new char[17];
		strcpy_s(m_desc,17,"Shape Definition");
		above=1;  
	}
	else 
	{
		pFile.Seek(def1seek,SEEK_SET);
		fi1=lendata[0].nbytes-4;  /* Len of desc (counting the 0). */
		pFile.Read(ln,fi1);

		fi1--;  /* Expected strlen of desc. */
		short fi2=0; while (fi2<fi1 && ln[fi2]) fi2++;
		/* fi2 is actual strlen of desc. */
		ln[fi2]=0;
		m_desc=new char[fi2+1];
		strcpy_s(m_desc,fi2+1,ln);

		/* Get the header bytes: */
		pFile.Read(ln,4);

		above=ln[0];
		below=ln[1];
		modes=ln[2];
	}

	//////////////////////////////////////////////////////////////////////////
	//2.读取字体信息数据

	pFile.Seek(def1seek,SEEK_SET);
	m_data.resize(256);
	for(int i=0;i <m_data.Count();i++)
		m_data[i]=NULL;
	for (fi1=0; fi1<nchrsinfile; fi1++) {
		/* 跳过头数据 */
		if (!lendata[fi1].code) 
		{
			int	newPosition = pFile.GetPosition() +lendata[fi1].nbytes;
			pFile.Seek(newPosition,SEEK_SET);
			continue;
		}

		char symbolName[255];//为合成线型样式	
		int symbolNameLen = 0;

		short togo=lendata[fi1].nbytes;
		char fint1;
		while (togo>0) {
			pFile.Read(&fint1,sizeof(fint1));
			symbolName[symbolNameLen] = (char)fint1;		
			++symbolNameLen;
			togo--;
			if (fint1==0 || fint1==EOF) break;
		}

		symbolName[symbolNameLen] = 0;
		++symbolNameLen;

		if (togo>0) {

			fontchrlink *flink=new fontchrlink();

			flink->code=lendata[fi1].code;
			flink->defsz=togo;
			flink->symbolName = new char[symbolNameLen];
			strcpy_s(flink->symbolName,symbolNameLen, symbolName);

			flink->def = new char[flink->defsz];
			for (short didx=0; didx<flink->defsz &&
				pFile.Read(&fint1,sizeof(fint1))&&fint1!=EOF; didx++)
				flink->def[didx]=(char)fint1;

			if(flink->code>0&&flink->code<256) m_data[flink->code]=flink;
			else
				m_data.Add(flink);

		}
	}

	if (lendata!=NULL)
		delete []lendata;
	return true;
}

bool ShapeFont::ReadBigfont(CFile& pFile,char* head)
{	
	//1.文件头解析
	long nchrsinfile=0;//字体的数量
	short fi1;//数据偏移量
	memcpy(&fi1,&head[27],sizeof(fi1));
	nchrsinfile=fi1;
	memcpy(&fi1,&head[27+sizeof(short)],sizeof(fi1));
	nesc = (char)fi1;

	fi1=2*nesc;
	esc = new char[fi1];
	fi1*=2;
	int def1seek = 27+sizeof(short)+sizeof(short);
	char ln[512];
	pFile.Seek(def1seek,SEEK_SET);
	pFile.Read(ln,fi1);

	def1seek = def1seek+fi1;//数据开始位置

	short fi2,fi3,fi4;
	for (fi1=fi2=fi3=0; fi1<nesc; fi1++,fi2+=2,fi3+=4) {
		memcpy(&fi4,&ln[fi3],sizeof(fi4));
		esc[fi2]=(char)fi4;
		memcpy(&fi4,&ln[fi3+2],sizeof(fi4));
		esc[fi2+1]=(char)fi4;
	}

	fi1=8;
	pFile.Read(ln,fi1);
	memcpy(&fi1,&ln[sizeof(short)],sizeof(fi1));
	long thisseek;
	memcpy(&thisseek,&ln[4],sizeof(thisseek));
	pFile.Seek(thisseek,SEEK_SET);
	pFile.Read(ln,fi1);

	fi2=0; while (fi2<fi1 && ln[fi2]) fi2++;
	fi2++; 
	fi3=fi1-fi2;  
	m_desc=new char[fi2];
	strcpy_s(m_desc,fi2, ln);

	/* 读取头文件参数 Get the header bytes: */
	above   =ln[fi2];
	below   =ln[fi2+1];
	modes   =ln[fi2+2];
	if (fi3>4) width=ln[fi2+3];

	//////////////////////////////////////////////////////////////////////////
	//2.读取字体信息数据

	m_data.resize(256);
	for(int i=0;i<m_data.Count();i++)m_data[i]=NULL;
	pFile.Seek(def1seek,SEEK_SET);
	long fileLenght= pFile.GetLength();
	char temp[1024];
	for (long locidx=0; locidx<nchrsinfile; locidx++)
	{
		unsigned char ch[8];
		pFile.Read(ch,8);
		unsigned short code,descLen;//字体编号，描述信息长度
		code= ((unsigned short)ch[0]<<8)|ch[1];
		//memcpy(&code,ch,2);
		/*unsigned short s=0;
		memcpy(&s,ch,sizeof(short));*/

		memcpy(&descLen,&ch[2],2);
		memcpy(&thisseek,&ch[4],sizeof(thisseek));

		if (code ==0)
		{				

			continue;
		}

		if(pFile.GetPosition()+ descLen>fileLenght) break;

		int g = pFile.GetPosition();
		pFile.Seek(thisseek,SEEK_SET);
		fontchrlink* link= new fontchrlink();
		link->code=code;		
		char* temp= new char[descLen];
		pFile.Read(temp,descLen);
		int k=0; while(k<descLen&&!temp[k]) k++;
		link->defsz=descLen-k;
		link->def=new char[descLen-k];
		memcpy(link->def,&temp[k],descLen-k);		
		if(code>0&&code<256) m_data[code]=link;
		else
		{			
			CString mx;mx.Format("%X",code);
			//AfxMessageBox(mx);
			m_data.Add(link);
		}
		pFile.Seek(g,SEEK_SET);
		delete []temp;
	}



	return true;
}

bool ShapeFont::ReadSHP(CFile& pFile,char* head,int &fileType)
{
	//1.文件头解析
	pFile.Seek(0,SEEK_SET);
	long def1seek;
	while(!fileType)
	{
		/* 读取第一行 */
		short lidx=0;
		char fint1;
		char ln[512];
		while (lidx<511 && pFile.Read(&fint1,sizeof(fint1)) &&
			fint1 && fint1!='\n')
			if (fint1!='\r') ln[lidx++]=(char)fint1;
		ln[lidx]=0;

		/*去除说明 */
		char* fcp1=NULL;
		fcp1=ln; while (*fcp1 && *fcp1!=';') fcp1++;
		*fcp1=0;

		//获取括号内数据
		short fi1,fi2;
		for (fi1=fi2=0; ln[fi2]; fi2++) {
			if (ln[fi2]=='(' || ln[fi2]==')') continue;
			if (fi1!=fi2) ln[fi1]=ln[fi2];
			fi1++;
		}
		ln[fi1]=0;

		/* Start lp at 1st non-space char: */
		char* lp=NULL;			;
		lp=ln; while (*lp && isspace((unsigned char) *lp)) lp++;
		if (!*lp) continue;  

		if (m_desc==NULL) {  /* 获取描述 */
			while (*lp && *lp!=',') lp++;
			if (*lp) {
				lp++;
				while (*lp && *lp!=',') lp++;
				if (*lp) {
					lp++;
					m_desc=new char[strlen(lp)+1];
					strcpy_s(m_desc,strlen(lp)+1, lp);
				}
			}
		} else {  /* 获取文件头数据. */

			for (fcp1=lp,fi1=0; *fcp1 && fi1<2; fcp1++)
				if (*fcp1==','/*DNT*/) fi1++;

			for (fi1=0; *lp && fi1<5; fi1++) {
				char fc1=(char)((unsigned char)getfontval(&lp));
				switch (fi1) {
				case 0: above   =fc1; break;
				case 1: below   =fc1; break;
				case 2: modes   =fc1; break;
				case 3: encoding=fc1; break;
				case 4: m_type    =fc1; break;
				}
			}

			/* Seek pos is ok for reading defs: */
			def1seek=pFile.GetPosition();

			fileType=1;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//2.读取字体信息数据
	pFile.Seek(def1seek,SEEK_SET);

	short thiscode=0;  /* Current chr code being processed. */
	short nditems=0;   /* # chr of definition items. */
	short cidx=0;      /* flink->chr[] idx. */
	short didx=0;      /* flink->chr[cidx].def[] idx */
	for (;;) {  /* Read lines. */


		/* Read 1 text line: */
		short lidx=0;
		char fint1;
		char ln[512];
		while (lidx<511 && pFile.Read(&fint1,sizeof(fint1)) &&
			fint1 && fint1!='\n'/*DNT*/)
			if (fint1!='\r'/*DNT*/) ln[lidx++]=(char)fint1;
		ln[lidx]=0;
		if (fint1==EOF) break;

		/* Chop comment: */
		char* fcp1;
		fcp1=ln; while (*fcp1 && *fcp1!=';'/*DNT*/) fcp1++;
		*fcp1=0;

		/* Compress out all parentheses: */
		short fi1,fi2;
		for (fi1=fi2=0; ln[fi2]; fi2++) {
			if (ln[fi2]=='('/*DNT*/ || ln[fi2]==')'/*DNT*/) continue;
			if (fi1!=fi2) ln[fi1]=ln[fi2];
			fi1++;
		}
		ln[fi1]=0;

		/* Start lp at 1st non-space char: */
		char *lp;
		lp=ln; while (*lp && isspace((unsigned char) *lp)) lp++;
		if (!*lp) continue;  /* Blank line. */

		if (*lp=='*'/*DNT*/) {  /* Start a new chr. */

			lp++; while (*lp && isspace((unsigned char) *lp)) lp++;
			if (*lp>='0'/*DNT*/ && *lp<='9'/*DNT*/) {  /* Numeric */
				thiscode=getfontval(&lp);
			} else {  /* Non-numeric */
				/* Try for one of the non-UNICODE */
				/* symbolic identifiers: */
				thiscode=0;  /* Ignore this chr if not found. */
				if      (strnisame(lp,"DEGREE_SIGN"/*DNT*/,11))
					thiscode=0x00B0;
				else if (strnisame(lp,"PLUS_OR_MINUS_SIGN"/*DNT*/,18))
					thiscode=0x00B1;
				else if (strnisame(lp,"DIAMETER_SYMBOL"/*DNT*/,15))
					thiscode=0x2205;
				/* Step past next comma: */
				while (*lp && *lp!=',') lp++;
				if (*lp) lp++;
			}
			if (!thiscode) continue;  /* 0 reserved for header line */
			if (!*lp) { fileType=0; return false; }
			nditems=getfontval(&lp);  /* # of data items. */
			if (nditems<1) { fileType=0; return false; }
			didx=0;  /* Init data item idx */

			/* Set the code and alloc the def array: */
			fontchrlink *flink=new fontchrlink();
			flink->code=thiscode;
			flink->defsz=nditems;
			flink->def=
				new char[flink->defsz];
			memset(flink->def,0,
				flink->defsz);
			if(flink->code>0&&flink->code<256)
			{
				m_data[flink->code]=flink;
				cidx = flink->code;
			}
			else
			{
				m_data.Add(flink);
				cidx=m_data.Count()-1;
			}

		} else {  /* Gather data for thiscode. */

			if (!thiscode || cidx<1 )
			{ thiscode=0; continue; }
			while (*lp && didx<m_data[cidx]->defsz)
				m_data[cidx]->def[didx++]=
				(char)((unsigned char)getfontval(&lp));
			if (didx>=m_data[cidx]->defsz) thiscode=0;

		}

	}
	return true;

}

string ShapeFont::Patch(unsigned short charCode,float orgX,float orgY,float scX,float scY)
{
	string res = "";
	fontchrlink* link= GetFromCode(charCode);
	if(link==NULL)
		return "";//找不到字体
	bool penDown=false;
	bool vertonly=false;//垂直字体
	CPoint curPt(0,0);//当前坐标点
	CPoint stack[102];//堆栈
	int sIndex=-1;	
	float dx=0;
	float dy=0;
	int i;
	for ( i=0; i<link->defsz;i++)
		if(link->def[i]==1||link->def[i]==2)
			break;

	for (; i<link->defsz;i++)
	{
		unsigned char opt= (unsigned char)link->def[i];
		dx=0;dy=0;
		if(opt==0)
		{//结束符号			
			continue;
		}
		switch(opt)
		{
		case 1:
			penDown=true;
			dx=0;dy=0;
			res.append("pendown|");
			break;
		case 2:
			penDown=false;
			dx=0;dy=0;
			res.append("penup|");
			break;
		case 3:
			{
				unsigned char sc= (unsigned char)link->def[++i];
				scX/=sc;scY/=sc;				
			}
			break;
		case 4:
			{
				unsigned char sc= (unsigned char)link->def[++i];
				scX*=sc;scY*=sc;				
			}
			break;
		case 5:
			stack[++sIndex]= curPt;
			break;
		case 6:
			curPt=stack[sIndex--];
			break;
		case 7:
			{
				char basex=0;
				char basey=0;
				char wd=0;
				char ht=0;
				unsigned short subCode=0;
				if(this->m_type==UNIFONT)
				{
					i+=2;
					subCode=(unsigned char)link->def[i-1]<<8
						|(unsigned char)link->def[i];
				}
				else
				{// SHAPE or BIGFONT 
					subCode=(unsigned char)link->def[++i];                                  
					if (!subCode) {
						i+=6;
						subCode=(unsigned char)link->def[i-5]<<8
							|(unsigned char)link->def[i-4];	
						basex= link->def[i-3];	
						basey= link->def[i-2];	
						wd= link->def[i-1];	
						ht= link->def[i];	
					}						
				}

				string _res = Patch(subCode,orgX+ basex,orgY+basey,scX,scY);
				res.append(_res);
			}
			break;
		case 8:
			{
				dx= link->def[++i]; 
				dy= link->def[++i]; 
			}
			break;
		case 9:
			{
				dx= link->def[++i]; 
				dy= link->def[++i]; 
				while(link->def[i-1]||link->def[i])
				{
					curPt.x+=dx;curPt.y-=dy;
					if(penDown){
						float _x = orgX+curPt.x*scX;
						float _y = orgY+curPt.y*scY;

						CString sx;
						sx.Format("%f",_x);
						CString sy;
						sy.Format("%f",_y);

						res.append("lineto:");
						res.append(sx + ",");
						res.append(sy + "|");
						////pDC->LineTo(orgX+curPt.x*scX,orgY+curPt.y*scY);
					}
					dx= link->def[++i]; 
					dy= link->def[++i]; 
				}
			}
			break;
		case 10:
			{//八分圆弧
				i+=2;
				bool cw= (link->def[i]&0x80)!=0;//true-逆时针，false-顺时针
				unsigned char r= link->def[i-1];
				if(r==0)r=1;
				//开始角位置(八分位置)
				unsigned char sa=(((unsigned char)(link->def[i]&'\x70'))>>4);

				//划过的角度(8:圆)
				unsigned char deta=(unsigned char)(link->def[i]&'\x07');
				if(cw)
				{
					sa-=deta;
					if(sa<0)sa+=8;
				}
				if(!deta)deta=8;

				unsigned char ea= sa+deta;
				/* dx and dy to endpt: */
				dx=r*(cos(ea*PI/4)-cos(sa*PI/4));
				dy=r*(sin(ea*PI/4)-sin(sa*PI/4));
				if(cw)
				{
					dx=-dx;dy=-dy;
				}
				//绘制圆(弧)
				////pDC->AngleArc(orgX+curPt.x - r * cos(sa*PI/4)*scX,orgY+ curPt.y+ r * sin(sa*PI/4)*scX,r*scX,sa*45,deta*45);
				res.append("anglearc|");
				curPt.x+=dx;curPt.y-=dy;
				dx=0;dy=0;
			}
			break;
		case 11:
			{
				i+=5;
				bool cw= (link->def[i]&0x80)!=0;//true-逆时针，false-顺时针
				float r=256.0*((float)((unsigned char)link->def[i-2]))+
					(float)((unsigned char)link->def[i-1]);
				if (r<1.0) r=1.0;
				//开始角位置(八分位置)
				float sa=(((unsigned char)(link->def[i]&'\x70'))>>4);

				//划过的角度(8:圆)
				float deta=(unsigned char)(link->def[i]&'\x07');

				//开始偏移，把45°角等分256份
				sa+=(unsigned char)link->def[i-4]/256.0f;
				deta += (unsigned char)link->def[i-3]/256.0f;//终止角偏移
				if(cw)
				{
					sa-=deta;
					if(sa<0)sa+=8;
				}
				if(!deta)deta=8;

				unsigned char ea= sa+deta;
				/* dx and dy to endpt: */
				dx=r*(cos(ea*PI/4)-cos(sa*PI/4));
				dy=r*(sin(ea*PI/4)-sin(sa*PI/4));
				if(cw)
				{
					dx=-dx;dy=-dy;
				}
				//绘制圆(弧)
				////pDC->AngleArc(orgX+curPt.x*scX - r * cos(sa*PI/4),orgY+ curPt.y*scY+ r * sin(sa*PI/4),r*scX,sa*45,deta*45);
				res.append("anglearc|");
				curPt.x+=dx;curPt.y-=dy;
				dx=0;dy=0;
				//绘制圆弧
			}
			break;
		case 12:
			{//弧
				dx= link->def[++i]; 
				dy= link->def[++i]; 
				float bug= link->def[++i]; //凸度
				if(bug>127)bug=127;if(bug<-127)bug=-127;
				bug/=127.0f;
				// i+=100;
				// pDC->LineTo(orgX+curPt.x*scX-100,orgY+curPt.y*scY);
				// pDC->LineTo(orgX+curPt.x*scX,orgY+curPt.y*scY);
				//绘制圆弧
				if(penDown)
				{
					double p1[2],p2[2],cc[2];
					double r,sa,ea;
					p1[0]=orgX+ curPt.x;
					p1[1]=orgY+ curPt.y;
					p2[0]=p1[0]+dx*scX;
					p2[1]=p1[1]-dy*scY;
					ic_bulge2arc(p1,p2, bug, cc, &r, &sa, &ea);
					float step=(ea-sa);						
					if(bug<0)
					{
						step=-step;
						sa=ea;
					}
					if(step>3.1415) 										
						step= IC_TWOPI-step;

					if(step<-3.1415)					
						step=-IC_TWOPI-step;

					step/=8;	
					for(int i=1;i<8;i++)
					{
						double x= cc[0] + r* cos(sa+step*i);						
						double y= cc[1] + r* sin(sa+step*i);

						CString sx;
						sx.Format("%f",x);
						CString sy;
						sy.Format("%f",y);

						res.append("lineto:");
						res.append(sx + ",");
						res.append(sy + "|");

						////pDC->LineTo(x,y);
					}
				}

			}break;
		case 13:
			{//多段圆弧
				unsigned char code1= dx= (unsigned char)link->def[++i]; 
				unsigned char code2= (unsigned char)link->def[++i]; 
				while(code1| code2)
				{//TODO:八分圆弧 case 10
					code1= dx= (unsigned char)link->def[++i]; 
					code2= (unsigned char)link->def[++i]; 
				}
			}
			break;
		case 14:
			vertonly=true;
			break;
		default:
			{

				unsigned char vlen=(unsigned char)link->def[i]; ;
				unsigned char vdir=vlen&'\x0F';
				if (!(vlen>>=4)) break;
				switch (vdir) {
				case '\x00': dx= 1.0; dy= 0.0; break;
				case '\x01': dx= 1.0; dy= 0.5; break;
				case '\x02': dx= 1.0; dy= 1.0; break;
				case '\x03': dx= 0.5; dy= 1.0; break;
				case '\x04': dx= 0.0; dy= 1.0; break;
				case '\x05': dx=-0.5; dy= 1.0; break;
				case '\x06': dx=-1.0; dy= 1.0; break;
				case '\x07': dx=-1.0; dy= 0.5; break;
				case '\x08': dx=-1.0; dy= 0.0; break;
				case '\x09': dx=-1.0; dy=-0.5; break;
				case '\x0A': dx=-1.0; dy=-1.0; break;
				case '\x0B': dx=-0.5; dy=-1.0; break;
				case '\x0C': dx= 0.0; dy=-1.0; break;
				case '\x0D': dx= 0.5; dy=-1.0; break;
				case '\x0E': dx= 1.0; dy=-1.0; break;
				case '\x0F': dx= 1.0; dy=-0.5; break;
				}
				dx*=vlen; dy*=vlen; 
			}
			break;
		}
		if(opt==1){
			////pDC->MoveTo(orgX+curPt.x,orgY+curPt.y);

			float _x = orgX+curPt.x;
			float _y = orgY+curPt.y;

			CString sx;
			sx.Format("%f",_x);
			CString sy;
			sy.Format("%f",_y);

			res.append("moveto:");
			res.append(sx + ",");
			res.append(sy + "|");
		}
		else
		{
			curPt.x+=dx*scY;curPt.y-=dy*scY;
			if(penDown){
				////pDC->LineTo(orgX+curPt.x,orgY+curPt.y);

				float _x = orgX+curPt.x;
				float _y = orgY+curPt.y;

				CString sx;
				sx.Format("%f",_x);
				CString sy;
				sy.Format("%f",_y);

				res.append("lineto:");
				res.append(sx + ",");
				res.append(sy + "|");
			}
		}

	}
	return res;
}


void ShapeFont::Display(CDC* pDC,unsigned short charCode,int orgX,int orgY)
{
	fontchrlink* link= GetFromCode(charCode);
	if(link==NULL)
		return;//找不到字体
	short vertonly,gotdxdy,circ,genpc,arcmode;
	//开始解析字体
	Point2D org(orgX,orgY);
	short pendown=1; 
	short skip=0;
	bool done=false;
	bool cw=false;
	short cmdcode=0;
	double dx=0;
	double dy=0;
	short repmode=0;  /* 0, 9, or 13 (the repeating commands) */
	short fi1;
	double bulge=0;
	short forcependown=1;
	double vfactx=1;
	double vfacty=1;
	double rad=0;	
	int psidx=0;
	int pssz=100;
	Point2D pts[102];//数据
	Point2D curpt(300,300);
	Point2D ap1;
	Point2D endpt;
	int npt=0;
	for (int didx=0;!done;didx++)
	{
		if(didx>=link->defsz) done=true;
		vertonly=gotdxdy=circ=genpc=0; arcmode=-1;

		if(!done)
		{
			if (repmode) {
				if (++didx<link->defsz) {
					if (!link->def[didx-1] && !link->def[didx]) repmode=0;
					if (repmode==9) {
						if (!skip) {
							dx=link->def[didx-1];
							dy=link->def[didx];
							gotdxdy=1;
						}
					} 
					else if (repmode==13) {
						if (++didx<link->defsz && !skip) {
							arcmode=1;  
							dx=link->def[didx-2];
							dy=link->def[didx-1];
							fi1=link->def[didx];
							if(fi1>127)fi1=127;
							else if(fi1<-127)fi1=-127;						
							bulge=fi1/127.0;
						}
					}
				}

			}
			else
			{
				cmdcode=(forcependown) ? 1 : link->def[didx];
				switch (cmdcode) {
				case  0:  /* End 结束*/
					if (skip) break;
					didx=link->defsz;  /* Trigger a "done" */
					break;
				case  1:  /* Pen down 下笔 */
					/* If we're doing a forced pendown, decrement */
					/* ccs->didx so that we don't eat a def byte: */
					if (forcependown) {
						didx--;
						forcependown=0;
					}
					if (skip) break;
					npt=1;
					pendown=1; dx=dy=0.0; gotdxdy=1;
					break;

				case  2:  /* Pen up 起笔*/
					if (skip || !pendown) break;
					pendown=0;
					genpc=1;
					npt=0;
					break;

				case  3:  /* Divide vector lengths by next byte 缩小 */
					if (++didx>=link->defsz) break;
					if (skip) break;
					if (!link->def[didx]) break;							
					vfactx/=((unsigned char)link->def[didx]);
					vfacty/=((unsigned char)link->def[didx]);
					// ]- EBATECH(CNBR)
					break;
				case  4:  /* Multiply vector lengths by next byte 放大*/
					if (++didx>=link->defsz) break;
					if (skip) break;
					if (!link->def[didx]) break;
					// EBATECH(CNBR) ]- for extended subshapes
					//vfact*=((unsigned char)ccs->thisfontchr->def[ccs->didx]);
					vfactx*=((unsigned char)link->def[didx]);
					vfacty*=((unsigned char)link->def[didx]);
					// ]- EBATECH(CNBR)
					break;
				case  5:  /* Push position 压入堆栈点*/
					if (skip || psidx>pssz) break;
					psidx++;
					pts[psidx]=curpt;						
					break;
				case  6:  /* Pop position 出堆栈*/
					/*
					**  Okay.  The code's getting a little bizarre
					**  as I keep patching problems.
					**
					**  Pop needs to lift the pen, move, and
					**  then restore the pen to its original
					**  status.  We can do all of this by
					**  popping the position, setting
					**  ccs->forcependown if the pen is
					**  currently down, and doing the guts of
					**  the penup command (case 2),.  (See the
					**  processing code below the end of this
					**  "else" and the forcependown code above.)
					*/

					if (skip || psidx<0) break;
					/* Pop curpt: */
					curpt=pts[psidx];							
					dx=dy=0.0; psidx--; gotdxdy=1;
					/* Set css->forcependown if it's currently down: 如果是下笔状态，给予向前动力*/
					forcependown=(pendown!=0);
					/* Do a penup command: 起笔*/
					pendown=0; genpc=1;
					break;

				case 7:/* Subshape 嵌套子对象 */
					{//TODO:

					}
					break;
				case  8:  /* dx,dy in next 2 bytes 坐标偏移*/
					if ((didx+=2)>=link->defsz) break;
					if (skip) break;
					dx=(double)link->def[didx-1];
					dy=(double)link->def[didx];
					gotdxdy=1;
					break;
				case  9: 
				case 13:  /* Repeat until (0,0) 重复 */
					repmode=link->def[didx];
					break;

				case 10:  /* Octant arc (next 2 bytes) 2字节圆弧*/
					{

					}
					break;
				case 11:  /* Fractional arc (next 5 bytes) y 5字节圆弧*/

					/* See documentation in learned.doc for this one; */
					/* ACAD's documentation is incorrect and */
					/* insufficient. */
					{

					}
					break;

				case 12:  /* Arc by bulge (next 3 bytes) 3字节圆弧*/
					{

					}
					break;
				case 14:  /* Process next command only for vertical text 垂直字体*/
					vertonly=1;
					break;
				case 15:  /* Not used 保留*/
					break;

				default:  /* Vector/direction 矢量*/
					if (skip) break;
					unsigned char vlen=(unsigned char)link->def[didx];
					char vdir=vlen&'\x0F';
					if (!(vlen>>=4)) break;
					switch (vdir) {
					case '\x00': dx= 1.0; dy= 0.0; break;
					case '\x01': dx= 1.0; dy= 0.5; break;
					case '\x02': dx= 1.0; dy= 1.0; break;
					case '\x03': dx= 0.5; dy= 1.0; break;
					case '\x04': dx= 0.0; dy= 1.0; break;
					case '\x05': dx=-0.5; dy= 1.0; break;
					case '\x06': dx=-1.0; dy= 1.0; break;
					case '\x07': dx=-1.0; dy= 0.5; break;
					case '\x08': dx=-1.0; dy= 0.0; break;
					case '\x09': dx=-1.0; dy=-0.5; break;
					case '\x0A': dx=-1.0; dy=-1.0; break;
					case '\x0B': dx=-0.5; dy=-1.0; break;
					case '\x0C': dx= 0.0; dy=-1.0; break;
					case '\x0D': dx= 0.5; dy=-1.0; break;
					case '\x0E': dx= 1.0; dy=-1.0; break;
					case '\x0F': dx= 1.0; dy=-0.5; break;
					}
					dx*=vlen; dy*=vlen; gotdxdy=1;
					break;
				}

			}		

			if (gotdxdy || arcmode>-1) {  /* Process vector or arc cmd 开始处理 矢量或圆弧*/

				endpt.X=curpt.X+ dx*vfactx;
				endpt.Y= curpt.Y- dy*vfacty;
				if(pendown)
				{
					if(npt==1)
						pDC->MoveTo(endpt.X+org.X,endpt.Y+org.Y);
					else if(npt>1)
						pDC->LineTo(endpt.X+org.X,endpt.Y+org.Y);
					npt++;
				}
				curpt=endpt;
			}
		}


	}

}


short getfontval(char **lpp) {

	char *lp = NULL;
	short rc,neg,fi1,fi2;


	rc=neg=0;


	if (lpp==NULL) goto out;

	lp=*lpp;  /* Convenience */

	while (*lp && isspace((unsigned char) *lp)) lp++;
	if (*lp=='+') lp++; else if (*lp=='-') { neg=1; lp++; }
	if (!*lp) goto out;
	if (*lp=='0'/*DNT*/) {  /* Hex */
		lp++;
		for (fi1=0; fi1<4; fi1++,lp++) {
			*lp=toupper(*lp);
			if (*lp>='A'/*DNT*/ && *lp<='F'/*DNT*/) {
				fi2=*lp-'A'/*DNT*/+10;
			} else if (*lp>='0'/*DNT*/ && *lp<='9'/*DNT*/) {
				fi2=*lp-'0'/*DNT*/;
			} else break;  /* Not a hex digit */
			rc<<=4; rc|=fi2;
		}
		if (neg) rc|=((fi1<3) ? 0x0080 : 0x8000);
	} else {  /* Decimal */
		rc=atoi(lp); if (neg) rc=-rc;
	}

	while (*lp && *lp!=',') lp++;
	if (*lp) { lp++; while (*lp && isspace((unsigned char) *lp)) lp++; }


out:
	if (lpp!=NULL) *lpp=lp;
	return rc;
}
