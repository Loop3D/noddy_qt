/* ============================================================================
 * [XVT -> Qt PORT] This file is unchanged calculation/application logic.
 * Its XVT toolkit calls now run against the Qt5-backed compatibility layer
 * in qt_compat/ (see qt_compat/xvt.h and CLAUDE.md) instead of the original
 * proprietary XVT SDK. No calculation or algorithmic code in this file was
 * modified to accomplish this port -- only the underlying GUI toolkit
 * implementation, in qt_compat/, changed.
 * ============================================================================
 */
#ifndef _MPL
#include "xvt.h"
#endif
#include "noddy.h"

/************************************************************************
*									*
*	void lubksb(a,n,indx,b)						*
*	float **a,b[]; 							* 
*	int n,*indx;	n is size of square array			*
*			indx is permutation vector			*
*			returned by ludcmp				*
*									*
*	lubksb function solves a.x = b	from numerical recipes in C	*
*									*
*	lubksb() takes 4 arguments					*
*	lubksb() returns no value					*
*									*
************************************************************************/
void lubksb(a,n,indx,b)
float **a,b[];
int n,*indx;
{
	int i,ii=0,ip,j;
	float sum;

	for (i=1;i<=n;i++)
	{
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii)
			for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
		else if (sum) ii=i;
		b[i]=sum;
	}
	for (i=n;i>=1;i--)
	{
		sum=b[i];
		for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
		b[i]=sum/a[i][i];
	}
}
