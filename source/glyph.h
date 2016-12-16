/*****************************************************************************
***     glyph.h
***
***     Copyright (C) 1997-2006 Microsoft Corporation. All rights reserved.
***     Derivative Portions Copyright (C) 2006 The IntelliCAD Technology
***     Consortium.  All rights reserved.
***
***     Use of the information contained herein, in part or in whole,
***     in/as source code and/or in/as object code, in any way by
***     anyone other than authorized employees of Microsoft Corporation or 
***     The IntelliCAD Technology Consortium, or by anyone to whom Microsoft
***     Corporation or The IntelliCAD Technology Consortium has not granted
***     use is illegal.
***
***     Description:
***
*****************************************************************************/

/*------------------------------------------------------------------------
$Workfile$
$Archive$
$Revision: 1.2.2.2 $
$Date: 2006/02/17 20:11:11 $
$Author: Ivan.Zhiboedov $
×ÖÐÎ
$Contents: class to get glyph index of a character$
$Log: glyph.h,v $
Revision 1.2.2.2  2006/02/17 20:11:11  Ivan.Zhiboedov
Fix for the 79897 bug (6.2 Spec Update Source Copyrights)

Revision 1.2.2.1  2006/01/19 04:59:25  Phil.Cordier
Update copyright notice for Microsoft-derived files that have been updated by the ITC

Revision 1.2  2005/04/13 08:29:20  master
Update Head branch

Revision 1.1.20.3  2004/10/01 18:33:24  alexeym
merged BricsCAD changes

Revision 1.1.20.2  2004/09/01 06:42:47  denisp
merged BricsCAD changes

Revision 1.1.20.1  2004/08/04 18:20:05  yamunav
Icad_cee initial version

Revision 1.4  2004/01/15 15:43:40  alexanderv
revert to enabling new font approach

Revision 1.3  2004/01/15 15:30:35  alexanderv
disable new font approach for tag 4-1-0018

Revision 1.2  2003/12/19 23:09:15  dmitryg
new approach for converting character code to glyph index

Revision 1.1  2002/01/22 08:58:15  alexanderv
icad3.1

Revision 1.1  2001/03/17 00:19:32  alexanderv
merge 14-9-2000 14-3-2001

Revision 1.2  2001/03/02 16:28:34  vitalys
kinsoku rule for CJP text wrapping

Revision 1.1  2001/01/24 13:13:19  vitalys
enable showing ideographs on win 9.x

------------------------------------------------------------------------*/

//	thanks to Eric D Crahen [crahen@cse.Buffalo.EDU] for idea.

/*DG - 20.12.2003*/// calculate glyph indices ourselves (using the TTF_table class)
// or let GDI's GetCharacterPlacement do that
#define FIND_GLYPH_INDICES_OURSELVES

#ifdef FIND_GLYPH_INDICES_OURSELVES

#ifndef _GLYPH_H_
#define _GLYPH_H_


class TTF_table {

	unsigned long SWAPLONG(unsigned long);
	unsigned short SWAPSHORT(unsigned short);

public:

	TTF_table ();
	~TTF_table();

	BOOL InitCMAP(HDC hdc, bool bVertical = false); //this function must be called after
                                                    //True Type font being attached to HDC

	int FindGlyph(int);
  
private:

  char   *cmap;                 // Copy of the cmap structure for this font.
  char   *gsub;                 // GSUB structre for this font, used for viertical glyph replacemnt
  unsigned short   count;                // Number of entries in the cmap subtable.
  unsigned short  *start;                // Start table for ranges                                
  unsigned short  *end;                  // End table for ranges
  unsigned short  *delta;                // Delta table for displacements
  unsigned short  *offset;               // Offest table for entriesinto the glyph table
  unsigned short  *glyph;                // Glyph table.
  unsigned short   vcount;               // Number of vertical replacement glyphs
  unsigned short  *from;                 // Glphys to be replaced.
  unsigned short  *to;                   // replacement glyphs.

};

#endif


#endif // FIND_GLYPH_INDICES_OURSELVES
