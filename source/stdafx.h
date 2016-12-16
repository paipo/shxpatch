// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__43BFE2DD_E831_47E1_B38A_23A4ABAA0610__INCLUDED_)
#define AFX_STDAFX_H__43BFE2DD_E831_47E1_B38A_23A4ABAA0610__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "Point2_T.h"
// TODO: reference additional headers your program requires here

const double	IC_PI			= 3.14159265358979323846;
const double	IC_TWOPI		= 2 * IC_PI;
#define IC_ZRO           1.0E-10


#define strnsame(a,b,c) (!strncmp((a),(b),(c)))

#ifndef FABS
//on XP we observed that the fabs() function
//performs 10+ times slower than the macro version underhere.
//For some drawings 15+ % of total processing time was spent in fabs()!!!
//We did not yet profile fabs() on older operating systems
#define FABS(a)  ((a) >= 0.0 ? (a) : -(a))
//#define FABS(a)  fabs(a)
//#define FABS(a)  fastAbs(a)

#endif
const double IcadFuzzyDistanceX = 1.0e-11;
inline bool icadPointEqual(const double p1[3], const double p2[3], double tol= IcadFuzzyDistanceX)
{
	if (FABS(p2[0])<=tol) {
		if (FABS(p1[0])>tol)
			return false;
	}
	else if (FABS((p1[0]/p2[0])-1.0) >= tol)
		return false;

	if (FABS(p2[1])<=tol) {
		if (FABS(p1[1])>tol)
			return false;
	}
	else if (FABS((p1[1]/p2[1])-1.0) >= tol)
		return false;

	if (FABS(p2[2])<=tol) {
		if (FABS(p1[2])>tol)
			return false;
	}
	else if (FABS((p1[2]/p2[2])-1.0) >= tol)
		return false;

	return true;
}

// If you don't know the scale of the underlying measurements,
// using the test "abs(a/b - 1) < epsilon"
// is likely to be more robust than simply comparing the difference

inline bool icadRealEqual(double r1, double r2, double tol = IcadFuzzyDistanceX)
{
	if (FABS(r2)<=tol) {
		if (FABS(r1)>tol)
			return false;
	}
	else {
		if (FABS((r1/r2)-1.0) > tol)
			return false;
	}
	return true;
}
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__43BFE2DD_E831_47E1_B38A_23A4ABAA0610__INCLUDED_)
