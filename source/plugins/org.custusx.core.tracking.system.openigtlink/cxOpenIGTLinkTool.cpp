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

#include "cxOpenIGTLinkTool.h"

#include <vtkConeSource.h>
#include "cxTrackingPositionFilter.h"
#include "cxLogger.h"
#include "cxProbeImpl.h"

namespace cx
{

OpenIGTLinkTool::OpenIGTLinkTool(QString uid) :
    ToolImpl(uid, uid),
    mPolyData(NULL),
    mTimestamp(0)
{
    connect(&mTpsTimer, SIGNAL(timeout()), this, SLOT(calculateTpsSlot()));


    mTypes = this->determineTypesBasedOnUid(Tool::mUid);
    if (this->isProbe())
    {
        mProbe = ProbeImpl::New(Tool::mUid, "OpenIGTLink");
        connect(mProbe.get(), SIGNAL(sectorChanged()), this, SIGNAL(toolProbeSector()));
    }

    this->createPolyData();
    this->toolVisibleSlot(true);
}

OpenIGTLinkTool::~OpenIGTLinkTool()
{
}

std::set<Tool::Type> OpenIGTLinkTool::getTypes() const
{
    return mTypes;
}

vtkPolyDataPtr OpenIGTLinkTool::getGraphicsPolyData() const
{
    return mPolyData;
}

ProbePtr OpenIGTLinkTool::getProbe() const
{
    return mProbe;
}

double OpenIGTLinkTool::getTimestamp() const
{
    return mTimestamp;
}

bool OpenIGTLinkTool::getVisible() const
{
    //TODO add some logic, visible if transform arrived in the last X seconds???
    return true;
}

bool OpenIGTLinkTool::isInitialized() const
{
    //TODO when is a tool initialized? when it is connected to the tracker?
    return true;
}

QString OpenIGTLinkTool::getUid() const
{
    return Tool::mUid;
}

QString OpenIGTLinkTool::getName() const
{
    return Tool::mName;
}

double OpenIGTLinkTool::getTooltipOffset() const
{
    if(this->getProbe())
        return this->getProbe()->getProbeData().getDepthStart();
    return ToolImpl::getTooltipOffset();
}

void OpenIGTLinkTool::setTooltipOffset(double val)
{
    if(this->getProbe())
        return;
    ToolImpl::setTooltipOffset(val);
}

std::set<Tool::Type> OpenIGTLinkTool::determineTypesBasedOnUid(const QString uid) const
{
    std::set<Type> retval;
    retval.insert(TOOL_POINTER);
    if(uid.contains("probe", Qt::CaseInsensitive))
    {
        retval.insert(TOOL_US_PROBE);
    }
    return retval;
}

bool OpenIGTLinkTool::isProbe() const
{
    return (mTypes.find(TOOL_US_PROBE) != mTypes.end()) ? true : false;
}

void OpenIGTLinkTool::createPolyData()
{
    //TODO this is copy paste from IGSTK tool: refactor out
    vtkConeSourcePtr coneSource = vtkConeSourcePtr::New();
    coneSource->SetResolution(25);
    coneSource->SetRadius(10);
    coneSource->SetHeight(100);

    coneSource->SetDirection(0, 0, 1);
    double newCenter[3];
    coneSource->GetCenter(newCenter);
    newCenter[2] = newCenter[2] - coneSource->GetHeight() / 2;
    coneSource->SetCenter(newCenter);

    coneSource->Update();
    mPolyData = coneSource->GetOutput();
}

bool OpenIGTLinkTool::isCalibrated() const
{
    //TODO how to know if an openigtlink tool is calibrated??
    CX_LOG_WARNING() << "OpenIGTLinkTool types are hardcoded to always be calibrated. Is this correct?.";
    return true;
}

Transform3D OpenIGTLinkTool::getCalibration_sMt() const
{
    Transform3D identity = Transform3D::Identity();
    return identity;
}

void OpenIGTLinkTool::setCalibration_sMt(Transform3D calibration)
{
    Q_UNUSED(calibration);
    CX_LOG_WARNING() << "Cannot set calibration on a openigtlink tool";
}

void OpenIGTLinkTool::toolTransformAndTimestampSlot(Transform3D matrix, double timestamp)
{
    Transform3D prMt_filtered = matrix;

    if (mTrackingPositionFilter)
    {
        mTrackingPositionFilter->addPosition(matrix, timestamp);
        prMt_filtered = mTrackingPositionFilter->getFilteredPosition();
    }

    mTimestamp = timestamp;
    (*mPositionHistory)[timestamp] = matrix; // store original in history
    m_prMt = prMt_filtered;
    emit toolTransformAndTimestamp(m_prMt, timestamp);

    //TODO is this needed?
//	ToolImpl::set_prMt(matrix, timestamp);
}

void OpenIGTLinkTool::calculateTpsSlot()
{
    int tpsNr = 0;

    int numberOfTransformsToCheck = ((mPositionHistory->size() >= 10) ? 10 : mPositionHistory->size());
    if (	numberOfTransformsToCheck <= 1)
    {
        emit tps(0);
        return;
    }

    TimedTransformMap::reverse_iterator it = mPositionHistory->rbegin();
    double lastTransform = it->first;
    for (int i = 0; i < numberOfTransformsToCheck; ++i)
        ++it;
    double firstTransform = it->first;
    double secondsPassed = (lastTransform - firstTransform) / 1000;

    if (!similar(secondsPassed, 0))
        tpsNr = (int) (numberOfTransformsToCheck / secondsPassed);

    emit tps(tpsNr);
}

void OpenIGTLinkTool::toolVisibleSlot(bool on)
{
    if (on)
        mTpsTimer.start(1000); //calculate tps every 1 seconds
    else
        mTpsTimer.stop();
}

void OpenIGTLinkTool::set_prMt(const Transform3D& prMt, double timestamp)
{
    CX_LOG_WARNING() << "Cannot set prMt on a openigtlink tool.";
}

void OpenIGTLinkTool::setVisible(bool vis)
{
    CX_LOG_WARNING() << "Cannot set visible on a openigtlink tool.";
}

}//namespace cx
