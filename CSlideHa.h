/* ============================================================================
 * [XVT -> Qt PORT] This file is unchanged calculation/application logic.
 * Its XVT toolkit calls now run against the Qt5-backed compatibility layer
 * in qt_compat/ (see qt_compat/xvt.h and CLAUDE.md) instead of the original
 * proprietary XVT SDK. No calculation or algorithmic code in this file was
 * modified to accomplish this port -- only the underlying GUI toolkit
 * implementation, in qt_compat/, changed.
 * ============================================================================
 */
// A child class for the implement a sliding handle from side to side.
//
#ifndef _CSLIDEHANDLE_H_
#define _CSLIDEHANDLE_H_
#include "CParamHa.h"

class CSlideHandle : public CParamHandle
{
public:
	CSlideHandle ();
	~CSlideHandle ();
	double setValue (double dwValue);

private:
	// thereoretical scale of values for the sphere you can move around
	double m_dwMinScaleValue, m_dwMaxScaleValue;
	// Actual upper and lower values that can be recorded
	double m_dwMinValue, m_dwMaxValue;
		
};

#endif
