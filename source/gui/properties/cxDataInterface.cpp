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
#include "cxDataInterface.h"
#include "cxImage.h"
#include <QSet>
#include "cxMesh.h"
#include "cxImageLUT2D.h"
#include "cxTrackingService.h"
#include "cxTypeConversions.h"
#include "cxDefinitionStrings.h"
#include "cxEnumConverter.h"
#include "cxTool.h"
#include "cxImageAlgorithms.h"
#include "cxRegistrationTransform.h"
#include "cxLogger.h"
#include "cxActiveImageProxy.h"
#include "cxVideoSource.h"
#include "cxVideoService.h"
#include "cxPatientModelService.h"
#include "cxDominantToolProxy.h"

//TODO: remove
#include "cxLegacySingletons.h"

namespace cx
{
DoublePropertyActiveToolOffset::DoublePropertyActiveToolOffset()
{
  mActiveTool = DominantToolProxy::New(trackingService());
  connect(mActiveTool.get(), &DominantToolProxy::tooltipOffset, this, &Property::changed);
}

double DoublePropertyActiveToolOffset::getValue() const
{
	return mActiveTool->getTool()->getTooltipOffset();
}

bool DoublePropertyActiveToolOffset::setValue(double val)
{
  mActiveTool->getTool()->setTooltipOffset(val);
  return true;
}

DoubleRange DoublePropertyActiveToolOffset::getValueRange() const
{
  double range = 200;
  return DoubleRange(0,range,1);
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

DoublePropertyActiveImageBase::DoublePropertyActiveImageBase(PatientModelServicePtr patientModelService) :
	mPatientModelService(patientModelService)
{
	mActiveImageProxy = ActiveImageProxy::New(patientModelService);
	connect(mActiveImageProxy.get(), &ActiveImageProxy::activeImageChanged, this, &DoublePropertyActiveImageBase::activeImageChanged);
	connect(mActiveImageProxy.get(), &ActiveImageProxy::transferFunctionsChanged, this, &Property::changed);
}
void DoublePropertyActiveImageBase::activeImageChanged()
{
  mImage = mPatientModelService->getActiveImage();
  emit changed();
}
double DoublePropertyActiveImageBase::getValue() const
{
  if (!mImage)
    return 0.0;
  return getValueInternal();
}
bool DoublePropertyActiveImageBase::setValue(double val)
{
  if (!mImage)
    return false;
  setValueInternal(val);
  return true;
}

//---------------------------------------------------------
//---------------------------------------------------------

double DoubleProperty2DWindow::getValueInternal() const
{
  return mImage->getLookupTable2D()->getWindow();
}
void DoubleProperty2DWindow::setValueInternal(double val)
{
  mImage->getLookupTable2D()->setWindow(val);
}
DoubleRange DoubleProperty2DWindow::getValueRange() const
{
  if (!mImage)
    return DoubleRange();
  double range = mImage->getRange();
  return DoubleRange(1,range,range/1000.0);
}

//---------------------------------------------------------
//---------------------------------------------------------

double DoubleProperty2DLevel::getValueInternal() const
{
  return mImage->getLookupTable2D()->getLevel();
}
void DoubleProperty2DLevel::setValueInternal(double val)
{
  mImage->getLookupTable2D()->setLevel(val);
}
DoubleRange DoubleProperty2DLevel::getValueRange() const
{
  if (!mImage)
    return DoubleRange();

  double max = mImage->getMax();
  return DoubleRange(1,max,max/1000.0);
}

////---------------------------------------------------------
////---------------------------------------------------------
////---------------------------------------------------------

StringPropertySelectRTSourceBase::StringPropertySelectRTSourceBase(PatientModelServicePtr patientModelService) :
	mPatientModelService(patientModelService)
{
	connect(mPatientModelService.get(), &PatientModelService::streamLoaded, this, &Property::changed);
}

StringPropertySelectRTSourceBase::~StringPropertySelectRTSourceBase()
{
	disconnect(mPatientModelService.get(), &PatientModelService::streamLoaded, this, &Property::changed);
}

QStringList StringPropertySelectRTSourceBase::getValueRange() const
{
  std::map<QString, VideoSourcePtr> streams = mPatientModelService->getStreams();
  QStringList retval;
  retval << "<no real time source>";
  std::map<QString, VideoSourcePtr>::iterator it = streams.begin();
  for (; it !=streams.end(); ++it)
    retval << qstring_cast(it->second->getUid());
  return retval;
}

QString StringPropertySelectRTSourceBase::convertInternal2Display(QString internal)
{
  VideoSourcePtr rtSource = mPatientModelService->getStream(internal);
  if (!rtSource)
    return "<no real time source>";
  return qstring_cast(rtSource->getName());
}
//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertyActiveVideoSource::StringPropertyActiveVideoSource()
{
	connect(videoService().get(), &VideoService::activeVideoSourceChanged, this, &Property::changed);
}

QString StringPropertyActiveVideoSource::getDisplayName() const
{
	return "Stream";
}

bool StringPropertyActiveVideoSource::setValue(const QString& value)
{
	if (value == this->getValue())
		return false;
	videoService()->setActiveVideoSource(value);
	emit changed();
	return true;
}

QString StringPropertyActiveVideoSource::getValue() const
{
	return videoService()->getActiveVideoSource()->getUid();
}

QStringList StringPropertyActiveVideoSource::getValueRange() const
{
	std::vector<VideoSourcePtr> sources = videoService()->getVideoSources();
	QStringList retval;
	for (unsigned i=0; i<sources.size(); ++i)
		retval << sources[i]->getUid();
	return retval;
}

QString StringPropertyActiveVideoSource::getHelp() const
{
	return "Select the active video source.";
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertySelectToolBase::StringPropertySelectToolBase()
{
	connect(trackingService().get(), &TrackingService::stateChanged, this, &Property::changed);
}

QStringList StringPropertySelectToolBase::getValueRange() const
{
	TrackingService::ToolMap tools = trackingService()->getTools();

	QStringList retval;
	for (TrackingService::ToolMap::iterator iter=tools.begin(); iter!=tools.end(); ++iter)
		retval << iter->second->getUid();
	return retval;
}

QString StringPropertySelectToolBase::convertInternal2Display(QString internal)
{
  ToolPtr tool = trackingService()->getTool(internal);
  if (!tool)
  {
    return "<no tool>";
  }
  return qstring_cast(tool->getName());
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertySelectCoordinateSystemBase::StringPropertySelectCoordinateSystemBase()
{
}

QStringList StringPropertySelectCoordinateSystemBase::getValueRange() const
{
  QStringList retval;
  retval << "";
  retval << qstring_cast(csREF);
  retval << qstring_cast(csDATA);
  retval << qstring_cast(csPATIENTREF);
  retval << qstring_cast(csTOOL);
  retval << qstring_cast(csSENSOR);
  return retval;
}

QString StringPropertySelectCoordinateSystemBase::convertInternal2Display(QString internal)
{
  if (internal.isEmpty())
    return "<no coordinate system>";

  //as requested by Frank
  if(internal == "reference")
    return "data reference";
  if(internal == "data")
    return "data (image/mesh)";
  if(internal == "patient reference")
    return "patient/tool reference";
  if(internal == "tool")
    return "tool";
  if(internal == "sensor")
    return "tools sensor";

  return internal;
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------


StringPropertySelectRTSource::StringPropertySelectRTSource(PatientModelServicePtr patientModelService) :
	StringPropertySelectRTSourceBase(patientModelService),
    mValueName("Select Real Time Source")
{
	connect(patientModelService.get(), &PatientModelService::streamLoaded, this, &StringPropertySelectRTSource::setDefaultSlot);
	this->setDefaultSlot();
}

QString StringPropertySelectRTSource::getDisplayName() const
{
  return mValueName;
}

bool StringPropertySelectRTSource::setValue(const QString& value)
{
  if(mRTSource && (mRTSource->getUid() == value))
    return false;

  if(mRTSource)
	  disconnect(mRTSource.get(), &VideoSource::streaming, this, &Property::changed);

  VideoSourcePtr rtSource = mPatientModelService->getStream(value);
  if(!rtSource)
    return false;

  mRTSource = rtSource;
  connect(mRTSource.get(), &VideoSource::streaming, this, &Property::changed);

  emit changed();
  return true;
}

QString StringPropertySelectRTSource::getValue() const
{
  if(!mRTSource)
    return "<no real time source>";
  return mRTSource->getUid();
}

QString StringPropertySelectRTSource::getHelp() const
{
  return "Select a real time source";
}

VideoSourcePtr StringPropertySelectRTSource::getRTSource()
{
  return mRTSource;
}

void StringPropertySelectRTSource::setValueName(const QString name)
{
  mValueName = name;
}

void StringPropertySelectRTSource::setDefaultSlot()
{
  std::map<QString, VideoSourcePtr> streams = mPatientModelService->getStreams();
  std::map<QString, VideoSourcePtr>::iterator it = streams.begin();
  if(it != streams.end())
  {
    this->setValue(it->first);
  }
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertySelectCoordinateSystem::StringPropertySelectCoordinateSystem()
{
	mCoordinateSystem = csCOUNT;
  connect(trackingService().get(), &TrackingService::stateChanged, this, &StringPropertySelectCoordinateSystem::setDefaultSlot);
}

QString StringPropertySelectCoordinateSystem::getDisplayName() const
{
  return "Select coordinate system";
}

bool StringPropertySelectCoordinateSystem::setValue(const QString& value)
{
  mCoordinateSystem = string2enum<COORDINATE_SYSTEM>(value);
  emit changed();
  return true;
}

QString StringPropertySelectCoordinateSystem::getValue() const
{
  return qstring_cast(mCoordinateSystem);
}

QString StringPropertySelectCoordinateSystem::getHelp() const
{
  return "Select a coordinate system";
}

void StringPropertySelectCoordinateSystem::setDefaultSlot()
{
  this->setValue(qstring_cast(csPATIENTREF));
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertySelectTool::StringPropertySelectTool()
{
	mValueName = "Select a tool";
	mHelp = mValueName;
}

void StringPropertySelectTool::setHelp(QString help)
{
  mHelp = help;
}

void StringPropertySelectTool::setValueName(QString name)
{
  mValueName = name;
}

QString StringPropertySelectTool::getDisplayName() const
{
  return mValueName;
}

bool StringPropertySelectTool::setValue(const QString& value)
{
  if(mTool && value==mTool->getUid())
    return false;
  ToolPtr temp = trackingService()->getTool(value);
  if(!temp)
    return false;

  mTool = temp;
  emit changed();
  return true;
}

QString StringPropertySelectTool::getValue() const
{
  if(!mTool)
    return "<no tool>";
  return mTool->getUid();
}

QString StringPropertySelectTool::getHelp() const
{
  return mHelp;
}

ToolPtr StringPropertySelectTool::getTool() const
{
  return mTool;
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------


StringPropertyParentFrame::StringPropertyParentFrame(PatientModelServicePtr patientModelService) :
	mPatientModelService(patientModelService)
{
	connect(mPatientModelService.get(), &PatientModelService::dataAddedOrRemoved, this, &Property::changed);
}

void StringPropertyParentFrame::setData(DataPtr data)
{
  if (mData)
	  disconnect(mData.get(), &Data::transformChanged, this, &Property::changed);
  mData = data;
  if (mData)
	  connect(mData.get(), &Data::transformChanged, this, &Property::changed);
  emit changed();
}

StringPropertyParentFrame::~StringPropertyParentFrame()
{
	disconnect(mPatientModelService.get(), &PatientModelService::dataAddedOrRemoved, this, &Property::changed);
}

QString StringPropertyParentFrame::getDisplayName() const
{
  return "Parent Frame";
}

bool StringPropertyParentFrame::setValue(const QString& value)
{
  if (!mData)
    return false;
  mData->get_rMd_History()->addParentSpace(value);
  return true;
}

QString StringPropertyParentFrame::getValue() const
{
  if (!mData)
    return "";
  return qstring_cast(mData->getParentSpace());
}

QString StringPropertyParentFrame::getHelp() const
{
  if (!mData)
    return "";
  return "Select the parent frame for " + qstring_cast(mData->getName()) + ".";
}

QStringList StringPropertyParentFrame::getValueRange() const
{
  QStringList retval;
  retval << "";

  std::map<QString, DataPtr> allData = mPatientModelService->getData();
  for (std::map<QString, DataPtr>::iterator iter=allData.begin(); iter!=allData.end(); ++iter)
  {
    if (mData && (mData->getUid() == iter->first))
      continue;

    retval << qstring_cast(iter->first);
  }
  return retval;
}

QString StringPropertyParentFrame::convertInternal2Display(QString internal)
{
  DataPtr data = mPatientModelService->getData(internal);
  if (!data)
    return "<no data>";
  return qstring_cast(data->getName());
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertySetParentFrame::StringPropertySetParentFrame(PatientModelServicePtr patientModelService) :
	StringPropertyParentFrame(patientModelService)
{
}

bool StringPropertySetParentFrame::setValue(const QString& value)
{
  if (!mData)
    return false;
  mData->get_rMd_History()->setParentSpace(value);
  return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertyDataNameEditable::StringPropertyDataNameEditable()
{
}

QString StringPropertyDataNameEditable::getDisplayName() const
{
  return "Name";
}

bool StringPropertyDataNameEditable::setValue(const QString& value)
{
  if (!mData)
    return false;
  mData->setName(value);
  return true;
}

QString StringPropertyDataNameEditable::getValue() const
{
  if (mData)
    return mData->getName();
  return "";
}

void StringPropertyDataNameEditable::setData(DataPtr data)
{
  mData = data;
  emit changed();
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertyDataUidEditable::StringPropertyDataUidEditable()
{
}

QString StringPropertyDataUidEditable::getDisplayName() const
{
  return "Uid";
}

bool StringPropertyDataUidEditable::setValue(const QString& value)
{
  return false;
}

QString StringPropertyDataUidEditable::getValue() const
{
  if (mData)
    return mData->getUid();
  return "";
}

void StringPropertyDataUidEditable::setData(DataPtr data)
{
  mData = data;
  emit changed();
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertyDataModality::StringPropertyDataModality(PatientModelServicePtr patientModelService) :
	mPatientModelService(patientModelService)
{
	connect(mPatientModelService.get(), &PatientModelService::dataAddedOrRemoved, this, &Property::changed);
}

StringPropertyDataModality::~StringPropertyDataModality()
{
	disconnect(mPatientModelService.get(), &PatientModelService::dataAddedOrRemoved, this, &Property::changed);
}

void StringPropertyDataModality::setData(ImagePtr data)
{
	if (mData)
		disconnect(mData.get(), &Data::propertiesChanged, this, &Property::changed);
	mData = data;
	if (mData)
		connect(mData.get(), &Data::propertiesChanged, this, &Property::changed);
	emit changed();
}

QString StringPropertyDataModality::getDisplayName() const
{
	return "Modality";
}

bool StringPropertyDataModality::setValue(const QString& value)
{
	if (!mData)
		return false;
	mData->setModality(value);
	return true;
}

QString StringPropertyDataModality::getValue() const
{
	if (!mData)
		return "";
	return mData->getModality();
}

QString StringPropertyDataModality::getHelp() const
{
	if (!mData)
		return "";
	return "Select the modality for " + qstring_cast(mData->getName()) + ".";
}

QStringList StringPropertyDataModality::getValueRange() const
{
	QStringList retval;
	retval << "";
	if (mData)
		retval << mData->getModality();
	retval << "CT" << "MR" << "US";
	return QStringList::fromSet(QSet<QString>::fromList(retval));
}

//---------------------------------------------------------
//---------------------------------------------------------
//---------------------------------------------------------

StringPropertyImageType::StringPropertyImageType(PatientModelServicePtr patientModelService) :
	mPatientModelService(patientModelService)
{
	connect(mPatientModelService.get(), &PatientModelService::dataAddedOrRemoved, this, &Property::changed);
}

void StringPropertyImageType::setData(ImagePtr data)
{
	if (mData)
		disconnect(mData.get(), &Data::propertiesChanged, this, &Property::changed);
	mData = data;
	if (mData)
		connect(mData.get(), &Data::propertiesChanged, this, &Property::changed);
	emit changed();
}

StringPropertyImageType::~StringPropertyImageType()
{
	disconnect(mPatientModelService.get(), &PatientModelService::dataAddedOrRemoved, this, &Property::changed);
}

QString StringPropertyImageType::getDisplayName() const
{
	return "Image Type";
}

bool StringPropertyImageType::setValue(const QString& value)
{
	if (!mData)
		return false;
	mData->setImageType(value);
	return true;
}

QString StringPropertyImageType::getValue() const
{
	if (!mData)
		return "";
	return mData->getImageType();
}

QString StringPropertyImageType::getHelp() const
{
	if (!mData)
		return "";
	return "Select the image type for " + qstring_cast(mData->getName()) + ".";
}

QStringList StringPropertyImageType::getValueRange() const
{
	QStringList retval;
	retval << "";
	if (mData)
	{
		retval << mData->getImageType();
		if (mData->getModality()=="CT")
			retval << "";
		if (mData->getModality()=="MR")
			retval << "T1" << "T2" << "ANGIO";
		if (mData->getModality()=="US")
			retval << "B-Mode" << "Angio";
	}
	return QStringList::fromSet(QSet<QString>::fromList(retval));
}

} // namespace cx
