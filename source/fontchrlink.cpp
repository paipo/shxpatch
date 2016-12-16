#include "StdAfx.h"
#include ".\fontchrlink.h"
#include "glyph.h"


short ic_bulge2arc(const Point2F& p0, const Point2F& p1, double bulge,
				   Point2F& cc, double *rr, double *sa, double *ea);

double ic_atan2(double yy, double xx);
void ic_normang(double *a1, double *a2);
Point2Ds ArcGen(Point2F pt1,Point2F pt2,double bulge,double pixsz,short curvdispqual);


fontchrlink::fontchrlink(void)
{
	code=0;
	defsz=0;
	def=NULL;
	symbolName=NULL;
	m_dataPtr=NULL;
}

fontchrlink::~fontchrlink(void)
{
	if(def)delete[] def;
	def=NULL;
	if (symbolName)delete [] symbolName;
	symbolName = NULL;
	if(m_dataPtr)delete m_dataPtr;
	m_dataPtr=NULL;
}


long fontchrlink::gr_parsetext(                       // R:  Unicode of character of control code
			 _TCHAR**                cpp,    // IO: String of text that should be parsed
			 UINT                    dwg_code_page,  // for converting from multibyte to Unicode
			 FontType   type,    // I:  Font type. May be: SHAPE_1_0, SHAPE_1_1, UNIFONT, BIGFONT, TRUETYPE
			 TTF_table* pFontTable
			 )
{
	/*
	This is a parser.  It determines the fist character code (from %%, 
	\U+, or \M+n control sequences) for the string *cpp and assigns *cpp
	TO the next char. Character maybe: glyph and control code. Glyph may
	be present as _TCHAR or with special sequence.

	**  Here are the current rules (based on testing ACAD); they aren't
	**  perfect yet:
	**
	**      %% sequences:
	**          Recognized following chars (case insensitive):
	**              c : Diameter symbol             (0x2205)
	**              d : Degree symbol               (0xB0)
	**              o : Overscore toggle            (-2L)
	**              p : Plus-or-minus symbol        (0xB1)
	**              u : Underscore toggle           (-3L)
	**            nnn : char with code nnn          (Unicode of this char)
	**              ? : Anything else (including %) (That char)
	**
	**      \U+xxxx sequences (the 'U' is case insensitive):
	**          All 4 chars of xxxx must be given (leading 0's, if necessary)
	**          and must be valid hex digits.
	**
	**      \M+nxxxx (the 'M' is case insensitive):
	**          Same as \U+xxxx (?n must be 1-4, but we are ignoring
	**          its meaning for now). (not implemented yet)
	**
	**  Returns the UNICODE chr code or one of the following:
	**       0L : Something specified chr 0 (%%0, \U+0000, \M+n0000),
	**              or, somehow, this function got called with
	**              *cpp pointing to the string-terminator.
	**      -1L : Error (cpp==NULL)
	**      -2L : Overscore  toggle
	**      -3L : Underscore toggle
	**
	**      (The return value is a long so that negative codes can be used
	**      for special purposes without conflicting with valid
	**      unsign short UNICODE values.)
	*/
	_TCHAR*     cp = NULL;
	_TCHAR*     hexp = NULL;
	_TCHAR      fuc1;
	short fi1;
	long rc;


	rc = 0L;
	//
	//    (*cpp)++;
	//    return 0x9d49;
	//
	if (cpp==NULL) 
	{ 
		rc = -1L; 
		goto out; 
	}; // if

	cp=(*cpp);

	rc=*cp;  /* Default */

	if ( _tcsnccmp(cp, _T("%"), 1) == 0) {
		if (_tcsnccmp(_tcsinc(cp), _T("%"), 1) == 0 && _tcslen(cp) > 2) 
		{
			cp = _tcsinc(_tcsinc(cp));
			if (_tcsncicmp(cp, _T("C"), 1) == 0 )
			{
				if (type == SHAPE_1_0)
					rc = 129;
				else if (type == SHAPE_1_1)
					rc = 258;
				//rc = 0x2205; 
				else
				{
					rc = 0x2205;

					if(!pFontTable || !pFontTable->FindGlyph(rc))
						rc = 216;

				}
			}else

				if (_tcsncicmp(cp, _T("D"), 1) == 0 )
				{
					if (type == SHAPE_1_0)
						rc = 127;
					else if (type == SHAPE_1_1)
						rc = 256;
					else
						rc = 0x00B0; 
				}else

					if (_tcsncicmp(cp, _T("O"), 1) == 0 )
					{
						rc = -2L;
					}else

						if (_tcsncicmp(cp, _T("P"), 1) == 0 )
						{
							if (type == SHAPE_1_0)
								rc = 128;
							else if (type == SHAPE_1_1)
								rc = 257;
							else
								rc = 0x00B1; 
						}else

							if (_tcsncicmp(cp, _T("U"), 1) == 0 )
							{
								rc = -3L;
							}else
							{
								// Covers %%%, too.
								// Look for up to 3 decimal digits (nnn):
								for (fi1=0; fi1<3 && cp[fi1] != 0 && _istdigit(*cp); fi1++);
								TCHAR       char_code[2];
								wchar_t     wchar_code;

								if (fi1 != 0)
								{ 
									fuc1 = *_tcsninc(cp, fi1);
									*_tcsninc(cp, fi1) = 0;
									char_code[0] = atoi((char *)cp);
									*_tcsninc(cp, fi1) = fuc1;
									cp = _tcsninc(cp, fi1 - 1);
								} else
								{
									char_code[0] = *cp;
									if (strlen(cp) > 1)
									{
										char_code[1] = *(cp + 1);
									};
								}; // if

								if (MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, (_TCHAR*)&char_code, 
									1, (wchar_t*)&wchar_code, sizeof(rc)) == 0 )
								{
									ASSERT(false);
								}; // if
								rc = wchar_code;
							}; // if
		}; // if
	} else if (_tcsnicmp(cp, _T("\\"), 1) == 0) {
		hexp=NULL;
		if (_tcsnicmp(_tcsinc(cp), _T("U+"), 2) == 0) {
			_TCHAR* begNum = _tcsninc(cp, 3);

			//Special symbols in SHAPE_1_0 files are stored at pos 127 - 129; in SHAPE_1_1 files they are
			//stored at 256 - 258 therefore I add 129 to the SHAPE_1_0 position to get the SHAPE_1_1 position.
			int addpos = 0;  
			if (type == SHAPE_1_1)
				addpos += 129;

			if ((type == SHAPE_1_0) || (type == SHAPE_1_1)) 
			{
				if (_tcsnicmp(begNum, _T("2205"), 4) == 0) 
					rc = 129 + addpos;

				else if (_tcsnicmp(begNum, _T("00B0"), 4) == 0)
					rc = 127 + addpos;

				else if (_tcsnicmp(begNum, _T("00B1"), 4) == 0)
					rc = 128 + addpos;

				else
					hexp=_tcsninc(cp, 3);
			}else
			{
				if (_tcsnicmp(begNum, _T("2205"), 4) == 0)
				{

					if(!pFontTable || !pFontTable->FindGlyph(0x2205))
						rc = 216;

					cp = _tcsninc(cp, 6);
				}
				else
					hexp=_tcsninc(cp, 3);
			}; // if

			if (hexp != NULL) { // if it's odinary (not special) Unicode char
				for (fi1 = 0; fi1 < 4 && _istxdigit(hexp[fi1]); fi1++);
				if (fi1>=4) {
					fuc1 = hexp[4]; 
					hexp[4] = 0;
					rc = _tcstol(hexp, NULL, 16);
					hexp[4] = fuc1;
					cp = _tcsninc(hexp, 3);
				}; // if
			}; // if

		} else if (_tcsnicmp(_tcsinc(cp), _T("M+"), 2) == 0) {
			hexp=_tcsninc(cp, 3);
			if (_istxdigit(*hexp))
			{
				UINT    code_page;
				if (_tcsnicmp(hexp, _T("0"), 1) == 0) {
					code_page = dwg_code_page;    // Current
				} else if (_tcsnicmp(hexp, _T("1"), 1) == 0) {
					code_page = 932;    // Japanese
				} else if (_tcsnicmp(hexp, _T("2"), 1) == 0) {
					code_page = 950;    // Traditional Chinese
				} else if (_tcsnicmp(hexp, _T("3"), 1) == 0) {
					code_page = 949;    // Wansung (Korean)
				} else if (_tcsnicmp(hexp, _T("4"), 1) == 0) {
					code_page = 1361;   // Johab
				} else if (_tcsnicmp(hexp, _T("5"), 1) == 0) {
					code_page = 936;    // Simplified Chinese
				} else {
					goto out;   // incorrect \M+ sequence.
				}; // if

				hexp = _tcsinc(hexp);

				char    multi_byte[2];


				for (fi1 = 0; fi1 < 2 && _istxdigit(hexp[fi1]); fi1++);
				if (fi1>=2) {
					fuc1 = hexp[2]; 
					hexp[2] = 0;
					multi_byte[0] = (char)_tcstol(hexp, NULL, 16);
					hexp[2] = fuc1;
					hexp = _tcsninc(hexp, 2);

					for (fi1 = 0; fi1 < 2 && _istxdigit(hexp[fi1]); fi1++);
					if (fi1>=2) {
						fuc1 = hexp[2]; 
						hexp[2] = 0;
						multi_byte[1] = (char)_tcstol(hexp, NULL, 16);
						hexp[2] = fuc1;

						wchar_t     wchar_code;

						// character from "\M+nxxxx" to Unicode
						if (MultiByteToWideChar(code_page, MB_PRECOMPOSED, 
							multi_byte, 2, &wchar_code, sizeof(wchar_code)) == 0 )
						{
							cp = *cpp;
							goto out;
						}; // if
						rc = wchar_code;
						cp = _tcsinc(hexp);
					}; // if
				}; // if

			}; // if

		}; // if (end of "\U+..", "\M+.." proceessing)

	}; // if (end of "%%" , "\\" proceesing)

out:
	//{{Modify 05-12-29 zhangq bug
	//	if (cp == *cpp) // if the character is not special or control
	//    {
	//        wchar_t     wchar_code[2];
	//        
	//        // multibyte (or just ordinary ANSI) to Unicode
	////79455 - Start		
	//		int res;
	//        if ((res=MultiByteToWideChar(dwg_code_page, MB_PRECOMPOSED, cp, 2, 
	//			wchar_code, sizeof(wchar_code))) == 0 )
	//        {
	//            // if NLS files of dwg_code_page hasn't been installed
	//            if (MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, cp, 2, 
	//                wchar_code, sizeof(wchar_code)) == 0 )
	//            {
	//				//                OutPutError();
	//                assert(false);
	//                return -1L;
	//            }
	//        }; // if
	//        rc = wchar_code[0];
	//		if (res == 1) cp = _tcsinc(cp);
	////79455 - End
	//    }; // if
	//	
	//    cp = _tcsinc(cp);
	if (cp == *cpp) // if the character is not special or control
	{
		wchar_t     wchar_code[2];

		// multibyte (or just ordinary ANSI) to Unicode
		//79455 - Start		
		int writtenWideCharactres;
		int sourceBytes = 2;
		if ((writtenWideCharactres=MultiByteToWideChar(dwg_code_page, MB_PRECOMPOSED, cp, sourceBytes, 
			wchar_code, sizeof(wchar_code))) == 0 )
		{
			// if NLS files of dwg_code_page hasn't been installed
			if ((writtenWideCharactres = MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, cp, sourceBytes, 
				wchar_code, sizeof(wchar_code))) == 0 )
			{
				//                OutPutError();
				ASSERT(false);
				return -1L;
			}
		}; // if
		rc = wchar_code[0];
		int sourceBytesPerCharacter = sourceBytes / writtenWideCharactres;
		cp = ((char*)cp + sourceBytesPerCharacter);
	} // if
	else
	{
		cp = _tcsinc(cp);
	}	
	//79455 - End
	//}}

	if (cpp!=NULL) *cpp=(char *)cp;
	return rc;
}

bool fontchrlink::ShapeCreateVec(CharData* pOut)
{
	pOut->m_charInfo.gmBlackBoxX=0;
	pOut->m_charInfo.gmBlackBoxY=0;
	pOut->m_charInfo.gmCellIncX=0;
	pOut->m_charInfo.gmCellIncY=0;
	pOut->m_charInfo.gmptGlyphOrigin.x=0;
	pOut->m_charInfo.gmptGlyphOrigin.y=0;
	short vertonly,gotdxdy,circ,genpc,arcmode;
	//开始解析字体
	fontchrlink* link=this;
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
	Point2F pts[102];//数据
	Point2F curpt(0,0);
	Point2F ap1;
	Point2F endpt;
	Point2F basePt(0,0);
	int npt=0;
	vex2Ds* curPart;

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
								if ((didx+=2)>=link->defsz) break;
								if (skip) break;

								arcmode=1;

								cw=((link->def[didx]&'\x80')!=0);

								/* 获取半径: */
								rad=(double)
									((unsigned char)link->def[didx-1]);
								if (rad<1.0) rad=1.0;


								/* 开始角度: */
								double sa=0.25*IC_PI*(((unsigned char)
									(link->def[didx]&'\x70'))>>4);

								/* Included angle: */
                            if ((fi1=link->def[didx]&'\x07')==0) {
                                /* Get the unit tangent direction vector: */
                                if (cw) { ap1.X=-sin(sa); ap1.Y= cos(sa); }
                                else    { ap1.X= sin(sa); ap1.Y=-cos(sa); }

                                /* Put the 2nd pt 0.01 font vector units away */
                                /* from the 1st pt in that direction: */
                                double ar1=0.01;
                                dx=ar1*ap1.X; dy=ar1*ap1.Y;
                                /*
                                **  The approx angle subtended is ar1/rad.
                                **  The tangent of one fourth of this is the
                                **  fabs(bulge) of the short arc.  The
                                **  inverse of that is the fabs(bulge) of
                                **  the long arc:
                                */
                                bulge=1.0/tan(0.25*ar1/rad);

                                circ=1;
                            } else {
                                double iang=0.25*IC_PI*fi1;
								double ea;
                                if (cw) ea=sa-iang; else ea=sa+iang;

                                /* dx and dy to endpt: */
                                dx=rad*(cos(ea)-cos(sa));
                                dy=rad*(sin(ea)-sin(sa));

                                /* abs(bulge): */
                                bulge=tan(0.25*iang);
                            }
                            if (cw) bulge=-bulge;  /* Neg bulge for CW arcs. */

							}
							break;
						case 11:  /* Fractional arc (next 5 bytes) y 5字节圆弧*/

							/* See documentation in learned.doc for this one; */
							/* ACAD's documentation is incorrect and */
							/* insufficient. */
							{

								double soct,eoctrel,soff,eoff;

                            if ((didx+=5)>=link->defsz) break;
                            if (skip) break;

                            arcmode=1;  /* Set up for bulge format (so CW arcs */
                                        /* and circles can be done by */
                                        /* gr_arc2pc() for connectivity with */
                                        /* a chain that's already been started). */

                            cw=((link->def[didx]&'\x80')!=0);

                            /* GET THE PARAMETERS IN FONT VECTOR UNITS: */
                            /* (Note we work in degrees for rounding.) */

                            /* Radius: */
                            rad=256.0*((double)((unsigned char)
                                link->def[didx-2]))+
                                (double)((unsigned char)
                                link->def[didx-1]);
                            if (rad<1.0) rad=1.0;

                            /* Starting octant (deg): */
                            soct=45.0*(((unsigned char)
                                (link->def[didx]&'\x70'))>>4);

                            /* Ending octant rel to starting octant (deg): */
                            if ((fi1=link->def[didx]&'\x07')<1) fi1=8;
                            eoctrel=45.0*(fi1-1);

                            /* Starting offset (deg): */
                            double ar1=((double)((unsigned char)
                                link->def[didx-4]))*45.0/256.0;
                            if (modf(ar1,&soff)>=0.5) soff+=1.0;

                            /* Ending offset (deg): */
                            ar1=((double)((unsigned char)
                                link->def[didx-3]))*45.0/256.0;
                            if (modf(ar1,&eoff)>=0.5) eoff+=1.0;
                            if (eoff<0.9) eoff=45.0;

                            /* Starting angle: */
                            double sa=soct+((cw) ? -soff : soff);

                            /* Ending angle: */
                            ar1=eoctrel+eoff;
                            double ea=soct+((cw) ? -ar1 : ar1);

                            /* Convert to radians finally: */
                            ar1=IC_PI/180.0;
                            sa*=ar1; ea*=ar1;

                            /* Get the included angle: */
                            ic_normang(&sa,&ea);
                            double iang=ea-sa; if (cw) iang=IC_TWOPI-iang;

                            ar1=FABS(iang);
                            if (ar1<IC_ZRO || FABS(ar1-IC_TWOPI)<IC_ZRO) {
                                /* Circle; set up an arc that almost closes */
                                /* (so we can continue to use bulge format */
                                /* as discussed above). */

                                /* Get the unit tangent direction vector: */
                                if (cw) { ap1.X=-sin(sa); ap1.Y= cos(sa); }
                                else    { ap1.X= sin(sa); ap1.Y=-cos(sa); }

                                /* Put the 2nd pt 0.01 font vector units away */
                                /* from the 1st pt in that direction: */
                                ar1=0.01;
                                dx=ar1*ap1.X; dy=ar1*ap1.Y;
                                /*
                                **  The approx angle subtended is ar1/rad.
                                **  The tangent of one fourth of this is the
                                **  fabs(bulge) of the short arc.  The
                                **  inverse of that is the fabs(bulge) of
                                **  the long arc:
                                */
                                bulge=1.0/tan(0.25*ar1/rad);

                                circ=1;
                            } else {
                                /* dx and dy to endpt: */
                                dx=rad*(cos(ea)-cos(sa));
                                dy=rad*(sin(ea)-sin(sa));

                                /* abs(bulge): */
                                bulge=tan(0.25*iang);
                            }
                            if (cw) bulge=-bulge;  /* Neg bulge for CW arcs. */
							}
							break;

						case 12:  /* Arc by bulge (next 3 bytes) 3字节圆弧*/
							{
								if ((didx+=3)>=link->defsz) break;
								if (skip) break;

								arcmode=1;  /* Set up for bulge format (so CW arcs */
								/* and circles can be done by */
								/* gr_arc2pc() for connectivity with */
								/* a chain that's already been started). */

								/* GET THE PARAMETERS IN FONT VECTOR UNITS: */

								/* dx and dy to endpt: */
								dx=(double)link->def[didx-2];
								dy=(double)link->def[didx-1];

								/* Bulge */
								if ((fi1=link->def[didx])>127) fi1=127;
								else if (fi1<-127) fi1=-127;
								bulge=((double)fi1)/127.0;
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

			if (!repmode) skip=(vertonly);// && !pTextInfo->vert);
			if (gotdxdy || arcmode>-1) {  /* Process vector or arc cmd 开始处理 矢量或圆弧*/

				endpt.X=curpt.X+ dx*vfactx;
				endpt.Y= curpt.Y+ dy*vfacty;
				pOut->m_charInfo.gmCellIncX=max(pOut->m_charInfo.gmCellIncX,endpt.X);
				pOut->m_charInfo.gmCellIncY=max(pOut->m_charInfo.gmCellIncY,endpt.Y);
				if(pendown)
				{
					pOut->m_charInfo.gmBlackBoxX=max(pOut->m_charInfo.gmBlackBoxX,endpt.X);
					pOut->m_charInfo.gmBlackBoxY=max(pOut->m_charInfo.gmBlackBoxY,endpt.Y);
					if(npt==1)
					{
						pOut->m_parts.Add(vex2Ds());
						curPart=&pOut->m_parts[pOut->m_parts.Count()-1];
					}
					if(arcmode>-1)
					{//弧,生成弧，存储到curPart

						Point2D center;
						double radius,sang,eang;

// 						ic_bulge2arc(curpt,endpt,bulge,center,&radius,&sang,&eang);						
// 						Point2Ds chain=ArcGen(center,radius,sang,eang,1,1000);

						Point2Ds chain=ArcGen(curpt,endpt,bulge,1,1000);
						for(int i=0;i<chain.Count();i++)
							{
								curPart->Add(chain[i].X);
								curPart->Add(chain[i].Y);								
							}
						if (circ)
						{
							curPart->Add(curpt.X);
							curPart->Add(curpt.Y);
						}
						
					}
					else
					{		
						if(npt>0)
						{
							curPart->Add(endpt.X);
							curPart->Add(endpt.Y);							
						}
						npt++;
					}
				}
				curpt.Assign(endpt);				
		 }			
	}

	}
	return true;
}
short ic_bulge2arc(const Point2F& p0, const Point2F& p1, double bulge,
					Point2F& cc, double *rr, double *sa, double *ea)
{
	/*
	**  Given an arc defined by two pts and bulge, determines the CCW arc's
	**  center, radius, starting angle and ending angle.
	**
	**  Returns:
	**       0 : OK
	**      -1 : Points coincident
	**       1 : It's a line
	**		-2 : Non planar arc
	*/
	short fi1;
	double dx,dy,sep,ss, ara[2];

	if (icadRealEqual(bulge,0.0)) 
		return 1;   /* Line */


	dx = p1.X-p0.X; 
	dy = p1.Y-p0.Y;
	if (icadRealEqual((sep=sqrt(dx*dx+dy*dy)), 0.0)) 
		return -1;   /* Coincident */

	*rr = FABS(sep * (bulge+1.0/bulge)/4.0);  /* Radius */

	if ((ss = (*rr)*(*rr) - sep*sep/4.0) > 0.0) 
		ss = sqrt(ss);
	else
		ss = 0.0;  /* Should never be negative*/

	/* Find center: */
	ara[0] = ss/sep;
	if (bulge < -1.0 || (bulge > 0.0 && bulge < 1.0)) {  /* Step left of midpt */
		cc.X = (p0.X+p1.X) / 2.0 - ara[0]*dy; 
		cc.Y = (p0.Y+p1.Y) / 2.0 + ara[0]*dx;
	} else {  /* Step left of midpt */
		cc.X = (p0.X+p1.X) / 2.0 + ara[0]*dy; 
		cc.Y = (p0.Y+p1.Y) / 2.0 - ara[0]*dx;
	}
	

	/* Find starting and ending angles: */
	dx = p0.X-cc.X; 
	dy = p0.Y-cc.Y; 
	ara[0] = ic_atan2(dy,dx);  /* Avoid METAWARE bug */
	dx = p1.X-cc.X; 
	dy = p1.Y-cc.Y; 
	ara[1] = ic_atan2(dy,dx);

	/* If bulge>=0.0, take starting angle from p0: */
	*sa = ara[fi1 = (bulge<0.0)]; 
	*ea = ara[!fi1];

	/* Make both 0.0<=ang<IC_TWOPI : */
	if (*sa < 0.0) 
		*sa+= IC_TWOPI;
	if (*ea < 0.0) 
		*ea+= IC_TWOPI;

	return 0;
}


double ic_atan2(double yy, double xx) {

	short inf = 0;
	short dexp = 300; 
	double fd1, fd2;
	double dmin = 1.0E-300; 
	double absxx = FABS(xx); 
	double absyy = FABS(yy);

	if (absxx < dmin) {
		if (absyy < dmin) 
			return 0.0;  /* Should give error, too */
		inf = 1;
	} else if (absyy >= dmin) {
		fd1 = log10(absyy); 
		fd2 = log10(absxx);
		if (fd1-fd2 > dexp) 
			inf=1;
	}
	if (inf) 
		return (yy > 0.0) ? IC_PI/2.0 : -IC_PI/2.0;

	fd1 = atan(yy/xx);
	if (xx < 0.0) 
		fd1 += (yy < 0.0) ? -IC_PI : IC_PI;

	return fd1;
}
Point2Ds ArcGen(Point2F pt1,Point2F pt2,double bulge,double pixsz,short curvdispqual)
{
	double dx,dy,dang;
	int cw=0;
	if (bulge<0.0)
		cw=1;
	if (curvdispqual<1) 
		curvdispqual=1;
	else if (curvdispqual>20000) 
		curvdispqual=20000;
	int maxnchords=400;  /* Max # of chords per arc or circle. */
	if ( curvdispqual > 1000 )
	{
		maxnchords *= (curvdispqual / 1000 );
	}
	
	double sa,ea;
	Point2F cen;
	double r;
	ic_bulge2arc(pt1,pt2,bulge,cen,&r,&sa,&ea);

	ic_normang(&sa,&ea);
	double iang = ea-sa;

	double ar1=0.3222*sqrt(curvdispqual*r/pixsz);

	double fract=iang/IC_TWOPI;
	int minnchords =(short)(8.99*fract);
	if (minnchords<1) minnchords=1;
	ar1*=fract; 

	if (ar1<minnchords) ar1=minnchords;
	else if (ar1>maxnchords) ar1=maxnchords;

	int fi1=0;
	int segNum=(short)ar1;
	if (fabs(fract-1.0)<IC_ZRO && (fi1=segNum%4)!=0) segNum+=4-fi1;

	Point2D ap1,ap2;
	Point2Ds chain;
	int nchords=segNum;	

	if (cw)
	{
		double cosr4=cos(ea);
		double sinr4=sin(ea);
		ap1.X=cen.X+r*cosr4;
		ap1.Y=cen.Y+r*sinr4;

		dang =-(ea-sa)/nchords;

		dx=cen.X+r*cos(ea+dang)-ap1.X;
		dy=cen.Y+r*sin(ea+dang)-ap1.Y;
	}
	else{

		double cosr3=cos(sa);
		double sinr3=sin(sa);
		ap1.X=cen.X+r*cosr3;
		ap1.Y=cen.Y+r*sinr3;

		dang =(ea-sa)/nchords;

		dx=cen.X+r*cos(sa+dang)-ap1.X;
		dy=cen.Y+r*sin(sa+dang)-ap1.Y;
	}
	

	double cs=cos(dang); 
	double sn=sin(dang);
	chain.Add(ap1);
	for (int i=0; i<nchords; i++) 
	{

		ap2.X=ap1.X+dx;
		ap2.Y=ap1.Y+dy;
		chain.Add(ap2);	
		double ar1=dx*cs-dy*sn; dy=dy*cs+dx*sn; dx=ar1;
		ap1 = ap2;
	}
	
	return chain;

}

void SetPoint(float*chain,int nIndex,const Point2D& val)
{
	chain[nIndex*2]=val.X;
	chain[nIndex*2+1]=val.Y;
}

void ic_normang(double *a1, double *a2)
{
	double ar1 = 1.0e5 * IC_TWOPI;
	while(*a1 > ar1) {
		*a1 -= ar1;
	}
	while(*a1 < -ar1) {
		*a1 += ar1;
	}

	ar1 = 1.0e3 * IC_TWOPI;
	while(*a1 > ar1) {
		*a1-=ar1;
	}
	while(*a1 < -ar1) {
		*a1+=ar1;
	}

	if (*a1 + IC_ZRO < 0.0) {
		do { 
			*a1+=IC_TWOPI; 
		} while (*a1 + IC_ZRO < 0.0);
	} 
	else if (*a1 - IC_ZRO >= IC_TWOPI) {
		do { 
			*a1-=IC_TWOPI; 
		} while (*a1 - IC_ZRO >= IC_TWOPI);
	}

	if (a2 != NULL) 
	{
		ar1=1.0e5*IC_TWOPI;
		while(*a2 > ar1) {
			*a2 -= ar1;
		}
		while(*a2 < -ar1) {
			*a2 += ar1;
		}

		ar1=1.0e3*IC_TWOPI;
		while(*a2 > ar1) {
			*a2 -= ar1;
		}
		while(*a2 < -ar1) {
			*a2+=ar1;
		}

		if (*a2 + IC_ZRO < 0.0) {
			do { 
				*a2 += IC_TWOPI; 
			} while (*a2 + IC_ZRO < 0.0);
		} 
		else if (*a2 - IC_ZRO >= IC_TWOPI) {
			do 	{ 
				*a2 -= IC_TWOPI; 
			} while (*a2 - IC_ZRO >= IC_TWOPI);
		}

		/* Make sure *a2>=*a1 */
		if (*a2 < *a1) {
			*a2 += IC_TWOPI;  
		}
	}
}