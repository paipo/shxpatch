/*------------------------------------------------------------------------
$Workfile$
$Archive$
$Revision: 1.2 $
$Date: 2005/04/13 08:29:20 $
$Author: master $

$Contents: class to get glyph index of a character$
$Log: glyph.cpp,v $
Revision 1.2  2005/04/13 08:29:20  master
Update Head branch

Revision 1.1.10.2.4.3  2004/10/01 18:33:24  alexeym
merged BricsCAD changes

Revision 1.1.10.2.4.2  2004/09/01 06:42:47  denisp
merged BricsCAD changes

Revision 1.1.10.2.4.1  2004/08/04 18:20:05  yamunav
Icad_cee initial version

Revision 1.6  2003/12/19 23:09:14  dmitryg
new approach for converting character code to glyph index

Revision 1.5  2003/12/05 07:49:16  natalias
at initCMAP

Revision 1.4  2003/11/27 22:51:20  dmitryg
increased performance of TTF_table::InitCMAP function which was a bottleneck
of TTF text generating (BugN128)

Revision 1.3  2003/02/12 13:11:45  alexanderv
fixed problem when size of allocated buffer does not correspond to actual font data
"backup" font for missing chars

Revision 1.1.12.2  2003/01/23 22:58:01  denisp
fixed problem when size of allocated buffer does not correspond to actual font data

Revision 1.1.12.1  2002/09/02 18:09:32  denisp
"backup" font for missing chars

Revision 1.3  2001/10/11 09:43:54  denisp
backup font for missing chars routines

Revision 1.2  2001/03/02 16:28:34  vitalys
kinsoku rule for CJP text wrapping

Revision 1.1  2001/01/24 13:13:19  vitalys
enable showing ideographs on win 9.x

------------------------------------------------------------------------*/

//	thanks to Eric D Crahen [crahen@cse.Buffalo.EDU] for idea.

#include "stdafx.h"
#include "glyph.h"

#ifdef FIND_GLYPH_INDICES_OURSELVES

// in-place swapping increases performance of InitCMAP in times
//位交换：双字节
inline void SWAPSHORT_InPlace(short* p)
{
	char c = *((char*)p);
	*((char*)p) = ((char*)p)[1];
	((char*)p)[1] = c;
}


#define X ((char *) &x)


/**
 * Reverse the byte alignment of a given unsigned long
 */
//位交换：四字节
inline unsigned long 
TTF_table::SWAPLONG(
	unsigned long	x
) 
{
  char p;

  p    = X[0];
  X[0] = X[3];
  X[3] = p;

  p    = X[1];
  X[1] = X[2];
  X[2] = p;

  return x;

}

/**
 * Reverse the byte alignment of a given unsigned short
 */
inline unsigned short 
TTF_table::SWAPSHORT(
	unsigned short	x
) 
{
  char p;
  p    = X[0];
  X[0] = X[1];
  X[1] = p;
  return (x);
}


TTF_table::TTF_table() : cmap(NULL), gsub(NULL) {}


TTF_table::~TTF_table() 
{

  if(cmap)
		delete [] cmap;

  if(gsub) 
		delete [] gsub;

  cmap = NULL;
  gsub = NULL;

  return;
}


/**
 *  Convert Unicode character code into an glyph index for the font. 
 *  转换代码转换为Unicode字符的字体字形索引
 */
int 
TTF_table::FindGlyph(   // R: Glyph index (0 if it fails)
	int		jis         // I: Unicode to be processed
) 
{
  int i;

    //定位到对应的分节seg
	for( i = 0; (i < count) && (jis > end[i]); i++);              // Find segment containning charcter
	if( jis < start[i] )
		return 0;
	if( !offset[i] )
		jis += delta[i];                              // Displacement character表位移
	else
		jis = *(offset[i]/2 + (jis - start[i]) + &offset[i]); // Get value from glyph array.

	jis &= 0x0000ffff;	// 2005-12-15 IntelliJapan(Masami) VERTICAL_TRUETYPE_FONT_FIX
	if( !jis )
		return 0;
	if( gsub )
	{  //对垂直字体的处理                                                 // If this is here we are printing vertical.
		for( i = 0; (i < vcount) && (from[i] < jis); i++);          // Find the glyph in the substitution list.
		if( from[i] == jis )
			jis = to[i];                            // Do we need to stubstitue.
	}
	return (jis);                                                 // Return value.
}

#define CMAP (*((unsigned long *) "cmap"))      // Character map tag
#define GSUB (*((unsigned long *) "GSUB"))      // Glyph substitution tag
#define VERT (*((unsigned long *) "vert"))      // Vertical printing tag

//
//  Macro used for addressing within the TrueType font structure.  This calucates and address from
//  the base of one structure and casts the result to the final type.  Note the offset can only
//  be short, since the swap is done here.
//  用于字体地址偏移，仅支持双字节
#define ADDRESS(t,b,o)    (struct t *) (((char *) b)+SWAPSHORT(o))


/**
 *  This routine opens a font up for operations.
 *  This function must be called after True Type font being attached to HDC
 */
                                                                    
BOOL 
TTF_table::InitCMAP(        // R: success
					HDC			hdc,        // I: Display context for the font (screen or printer).
					bool		vertical
					) 
{
	if(cmap)
		delete [] cmap;
	
	if(gsub) 
		delete [] gsub;
	
	cmap = NULL;
	gsub = NULL;
	
	struct cmap_header {      // cmap header structure
		unsigned short  version;         //   file version number (not used).
		unsigned short  number;          //   Number of encodings
	};
	struct table {            // Encoding sub-tables 
		unsigned short  platform_id;     //   platform id (we want 3 == PC)
		unsigned short  encoding_id;     //   encoding id (we want 1 == UNICODE)
		unsigned long  offset;          //   offset from start of cmap to the actual sub-table
	} *tables;
	struct subtable {         // A partial version of a cmap sub-table
		unsigned short  format;          //   format (should be 4)
		unsigned short  length;          //   length of sub-table
		unsigned short  version;         //   version (not used)
		unsigned short  segCountX2;      //   twice the number of segments in the sub table 
		unsigned short  searchRange;     //   junk 预留
		unsigned short  entrySelector;   //   junk
		unsigned short  rangeShift;      //   junk
		unsigned short  endCount;        //   start of end list table 
	} *sub;                   // -- Rest of table is calculated on the fly, since this table cannot be represented with a C++ structure.
	int  i;
	long j, size;
	
	j = ::GetFontData(hdc,CMAP,0,NULL,0);  // 取得字体数量Size of the cmap data
	if ( j == GDI_ERROR )
	{
		return FALSE;
	}; // if
	//  if (!(cmap = (char *) malloc(j))) 
	//		return (true);    // Make a memory block for this
	if(!j) 
		return FALSE;
	
	cmap = new char[j];
	
	// Read the cmap.
	size = ::GetFontData(hdc,CMAP,0,cmap,j);
	if (size == GDI_ERROR) 
		return FALSE;   
	
	// Get number of sub-tables.
	j = SWAPSHORT (((struct cmap_header *) cmap)->number);       
	
	// Get sub-tables
	tables = (struct table *) (cmap+sizeof(struct cmap_header));
	
	// Find our sub-table (3,1)
	for (i = 0; i < j; i++) {                                         
		if ((SWAPSHORT(tables[i].platform_id) == 3) && (SWAPSHORT(tables[i].encoding_id) == 1)) 
			break;
	}; // for i
	
	// Could not find sub-table
	if (i >= j) 
		return FALSE;
	
	// Find the actual sub-table
	sub = (struct subtable *) (cmap+ SWAPLONG(tables[i].offset));
	if (SWAPSHORT(sub->format) != 4) // Check, is the table format 4.
		return FALSE;
	
	count = SWAPSHORT(sub->segCountX2)/2; // Get count (number of segments)
	end = &sub->endCount; // Get address of segment tables
	start = end+count+1;
	delta = start+count;
	offset = delta+count;
	glyph = offset+count; // Get address of the glyph array
	j = SWAPSHORT(sub->length)/2; // Swap bytes in the sub-table
	
	if((long)sub - (long)cmap + 2 * j > size)
		j = (size - (long)sub + (long)cmap) / 2;
	
	_swab((char*)sub, (char*)sub, j*sizeof(short));
	//
	//  If the font is to be printed vertical we will need to change the offsets and change some of 
	//  the gylphs for vertical printing.
	//
	if (vertical) {
#pragma pack(push,1)                // Necessary to get the structure alignement correct!
		struct gsub_header {                // Header structure for the GSUB 
			long   version;                   // version (not used)    
			unsigned short  script_list;               // offset to script list (not actually used)
			unsigned short  feature_list;              // offset to the feature list (this is checked)
			unsigned short  lookup_list;               // offset to the lookup list.
		} *header;
		struct feature_element {            // List element for featchers stored in the GSUB
			unsigned long  tag;                       // Tag.
			unsigned short  offset;                    // Offset in the feature list.
		};
		struct feature_list {               // Features stored in this GSUB
			unsigned short  count;                     // Number of features in the GSUB
			struct feature_element elements;  // List of actual feature elements.
		} *feature_list;
		struct feature {                    // Description of an actual feature parameters
			unsigned short  param;                     // parameters (not used).
			unsigned short  count;                     // Number of indexes (assumed to be 1).
			unsigned short  index;                     // Actual index to feature.
		} *feature;
		struct lookup_list {                // Contains offset to lookup lists
			unsigned short  count;                     // Number of lookup lists (not used, we get index from feature)
			unsigned short  offset;                    // Offset to list
		} *lookup_list;
		struct lookup {                     // Describes a lookup table and process.
			unsigned short  type;                      // lookup type (assumed to be 1 = substitution).
			unsigned short  flags;                     // not used.
			unsigned short  count;                     // Number of lists in the set (assumed to be 1)
			unsigned short  offset;                    // Offset to the first part of the list.
		} *lookup;
		struct sub_table {                  // This describes the replacment glyph list or substituiton table
			unsigned short  format;                    // format (assumed to be 2 = output glyphs)
			unsigned short  offset;                    // offset to the the from (coverage table).
			unsigned short  count;                     // number of entires.
			unsigned short  glyphs;                    // glyph list (fisrt glyph)
		} *sub_table;
		struct coverage {                   // Table indicates what glyphs are to be replaced with substitions
			unsigned short  format;                    // format (assumed to be 1)
			unsigned short  count;                     // number of entries (assumed to match sub-table)
			unsigned short  glyphs;                    // glyph list (first glyph)                       
		} *coverage;
#pragma pack(pop)                   // Restore the structure alignment
		
		j = ::GetFontData(hdc,GSUB,0,NULL,0); // Get size of the gsub
		if(!j)	
			return FALSE;
		
		gsub = new char[j];
		
		// Allocat space
		//if (!(gsub = (char *) malloc(j))) return (true);                    
		
		::GetFontData(hdc,GSUB,0,gsub,j); 
		header = (struct gsub_header *) gsub; 
		// Find the feature list.
		feature_list = ADDRESS(feature_list,gsub,header->feature_list); 
		j = SWAPSHORT(feature_list->count); 
		
		for (i = 0; (i < j) && (VERT != (&feature_list->elements)[i].tag); i++) NULL; 
		
		// No vert then we cannot use this font
		if (i >= j) 
			return FALSE;
		
		feature     = ADDRESS(feature,feature_list,(&feature_list->elements)[i].offset);        // Get feature for 'vert'
		lookup_list = ADDRESS(lookup_list,gsub,header->lookup_list);                            // This is the base of the lookup tables.
		lookup      = ADDRESS(lookup,lookup_list,(&lookup_list->offset)[SWAPSHORT(feature->index)]);   // Select lookup table we need
		sub_table   = ADDRESS(sub_table,lookup,lookup->offset);             // Glyphs that are the substitues.
		coverage    = ADDRESS(coverage,sub_table,sub_table->offset);        // Glyphs that are to be replaced.
		vcount      = SWAPSHORT(coverage->count);                          // These are the reall working values.
		from        = &coverage->glyphs;
		to          = &sub_table->glyphs;
		
		_swab((char*)from, (char*)from, vcount*sizeof(short));
		_swab((char*)to, (char*)to, vcount*sizeof(short));
		
	}; // if (vertical)
	
	return TRUE;
}


#endif // FIND_GLYPH_INDICES_OURSELVES
