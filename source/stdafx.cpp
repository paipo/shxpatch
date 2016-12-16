// stdafx.cpp : 只包括标准包含文件的源文件
// test1.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
// 引用任何所需的附加头文件，而不是在此文件中引用
short ic_bulge2arc(double* p0, double* p1, double bulge,
    double* cc, double *rr, double *sa, double *ea) {
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

    dx = p1[0]-p0[0]; 
	dy = p1[1]-p0[1];
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
        cc[0] = (p0[0]+p1[0]) / 2.0 + ara[0]*dy; 
		cc[1] = (p0[1]+p1[1]) / 2.0 - ara[0]*dx;
    } else {  /* Step left of midpt */
        cc[0] = (p0[0]+p1[0]) / 2.0 - ara[0]*dy; 
		cc[1] = (p0[1]+p1[1]) / 2.0 + ara[0]*dx;
    }
	
    /* Find starting and ending angles: */
    dx = p0[0]-cc[0]; 
	dy = p0[1]-cc[1]; 
	ara[0] = atan2(dy,dx);  /* Avoid METAWARE bug */
    dx = p1[0]-cc[0]; 
	dy = p1[1]-cc[1]; 
	ara[1] = atan2(dy,dx);

  /* If bulge>=0.0, take starting angle from p0: */
    *sa = ara[fi1 = (bulge<0.0)]; 
	*ea = ara[!fi1];

    /* Make both 0.0<=ang<IC_TWOPI : */
    if (*sa < 0.0||*sa < 0.0)
	{
		*sa+= IC_TWOPI;
    //if (*ea < 0.0) 
		*ea+= IC_TWOPI;
	}
    return 0;
}