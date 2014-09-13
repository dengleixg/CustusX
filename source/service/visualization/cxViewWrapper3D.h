/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/

/*
 * cxViewWrapper3D.h
 *
 *  \date Mar 24, 2010
 *      \author christiana
 */
#ifndef CXVIEWWRAPPER3D_H_
#define CXVIEWWRAPPER3D_H_

#include <vector>
#include <QPointer>
#include <QObject>
#include "cxDefinitions.h"
#include "cxViewWrapper.h"
#include "cxForwardDeclarations.h"
#include "cxVector3D.h"
#include "cxCoordinateSystemHelpers.h"

class QAction;
typedef vtkSmartPointer<class vtkAnnotatedCubeActor> vtkAnnotatedCubeActorPtr;
typedef vtkSmartPointer<class vtkOrientationMarkerWidget> vtkOrientationMarkerWidgetPtr;

namespace cx
{
typedef boost::shared_ptr<class Slices3DRep> Slices3DRepPtr;
typedef boost::shared_ptr<class DataMetricRep> DataMetricRepPtr;
typedef boost::shared_ptr<class MetricNamesRep> MetricNamesRepPtr;

}

namespace cx
{
typedef boost::shared_ptr<class Navigation> NavigationPtr;
typedef boost::shared_ptr<class ImageLandmarkRep> ImageLandmarkRepPtr;
typedef boost::shared_ptr<class PatientLandmarkRep> PatientLandmarkRepPtr;
typedef boost::shared_ptr<class MultiVolume3DRepProducer> MultiVolume3DRepProducerPtr;
typedef boost::shared_ptr<class AxisConnector> AxisConnectorPtr;

/**
 * \file
 * \addtogroup cx_service_visualization
 * @{
 */

typedef boost::shared_ptr<class ToolAxisConnector> ToolAxisConnectorPtr;

enum STEREOTYPE
{
	stFRAME_SEQUENTIAL, stINTERLACED, stDRESDEN, stRED_BLUE
};


/** Wrapper for a View3D.
 *  Handles the connections between specific reps and the view.
 *
 */
class ViewWrapper3D: public ViewWrapper
{
Q_OBJECT
public:
	ViewWrapper3D(int startIndex, ViewPtr view, VisualizationServiceBackendPtr backend);
	virtual ~ViewWrapper3D();
	virtual ViewPtr getView();
	virtual double getZoom2D() { return -1.0; }
	virtual void setSlicePlanesProxy(SlicePlanesProxyPtr proxy);
	virtual void setViewGroup(ViewGroupDataPtr group);
	void setStereoType(int type);

protected slots:
	virtual void dataViewPropertiesChangedSlot(QString uid);
private slots:
	void showSlices();
	void dominantToolChangedSlot(); ///< makes sure the reps are connected to the right tool
	void toolsAvailableSlot(); ///< add all tools when configured
	void showSlicePlanesActionSlot(bool checked);
	void fillSlicePlanesActionSlot(bool checked);
	void showAxesActionSlot(bool checked);
	void showManualToolSlot(bool visible);
	void resetCameraActionSlot();
	void activeImageChangedSlot();
	void showRefToolSlot(bool checked);
	void showToolPathSlot(bool checked);
	void PickerRepPointPickedSlot(Vector3D p_r);
	void centerImageActionSlot();
	void centerToolActionSlot();
	void optionChangedSlot();
	void showOrientationSlot(bool visible);
	void globalConfigurationFileChangedSlot(QString key);
	void setStereoEyeAngle(double angle);
	void settingsChangedSlot(QString key);
	void PickerRepDataPickedSlot(QString);
	void updateView();

private:
	virtual void appendToContextMenu(QMenu& contextMenu);
	void readDataRepSettings(RepPtr rep);
	void updateSlices();
	NavigationPtr getNavigation();

	QAction* createSlicesAction(QString title, QWidget* parent);

	void createSlicesActions(QWidget *parent);
	QAction* createSlicesAction(PlaneTypeCollection planes, QWidget* parent);

	void showLandmarks(bool on);
	void showPointPickerProbe(bool on);
	void setOrientationAnnotation();

	RepPtr createDataRep3D(DataPtr data);
    DataMetricRepPtr createDataMetricRep3D(DataPtr data);

	void addVolumeDataRep(DataPtr data);
	void removeVolumeDataRep(QString uid);

	void setTranslucentRenderingToDepthPeeling(bool setDepthPeeling);
	void initializeMultiVolume3DRepProducer();
	void updateMetricNamesRep();

	MultiVolume3DRepProducerPtr mMultiVolume3DRepProducer;
	typedef std::map<QString, RepPtr> RepMap;
	RepMap mDataReps;
	LandmarkRepPtr mLandmarkRep;
	PickerRepPtr mPickerRep;
	DisplayTextRepPtr mPlaneTypeText;
	DisplayTextRepPtr mDataNameText;
	MetricNamesRepPtr mMetricNames;
	std::vector<AxisConnectorPtr> mAxis;

	bool mShowAxes; ///< show 3D axes reps for all tools and ref space
	Slices3DRepPtr mSlices3DRep;
	SlicePlanes3DRepPtr mSlicePlanes3DRep;
	OrientationAnnotation3DRepPtr mAnnotationMarker;

	ViewPtr mView;
};
typedef boost::shared_ptr<ViewWrapper3D> ViewWrapper3DPtr;

/**
 * @}
 */
} // namespace cx

#endif /* CXVIEWWRAPPER3D_H_ */
