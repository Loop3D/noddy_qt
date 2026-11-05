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
#ifndef _CROTATEHANDLE_H_
#define _CROTATEHANDLE_H_
#include "CParamHa.h"

class CRotateHandle : public CParamHandle
{
public:
	CRotateHandle ();
	~CRotateHandle ();

	int draw () const;
	int drawChangeArc () const;

	double setValue (double dwValue);
	double setScale (double dwScale);

	void getRotationAxis (double *dwRotationAxisX, double *dwRotationAxisY, double *dwRotationAxisZ) const;
	void setRotationAxis (double dwRotationAxisX, double dwRotationAxisY, double dwRotationAxisZ);
	void getTotalExtentScaleValues (double *dwMinScaleValue, double *dwMaxScaleValue) const;
	void setTotalExtentScaleValues (double dwMinScaleValue, double dwMaxScaleValue);
	void getMovementExtentValues (double *dwMinValue, double *dwMaxValue) const;
	void setMovementExtentValues (double dwMinValue, double dwMaxValue);
	void getMovementExtentScaleValues (double *dwMinScale, double *dwMaxScale) const;
	void setMovementExtentScaleValues (double dwMinScale, double dwMaxScale);

protected:
	double getAngle() const;	// retrun the angle at which a line should be drawn
	double getPercentScale() const;	// retrun the Scaleing which needs to be drawn along the line

private:
	// thereoretical scale of values for the sphere you can move around
	double m_dwMinScaleValue, m_dwMaxScaleValue;
	// Actual upper and lower values that can be recorded
	double m_dwMinValue, m_dwMaxValue;
	double m_dwMinScale, m_dwMaxScale;
	double m_dwRotationAxisX, m_dwRotationAxisY, m_dwRotationAxisZ;
		
};

#endif
