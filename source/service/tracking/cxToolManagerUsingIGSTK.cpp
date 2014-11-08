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

#define _USE_MATH_DEFINES

#include "cxToolManagerUsingIGSTK.h"

#include <QTimer>
#include <QDir>
#include <QList>
#include <QMetaType>
#include <QFileInfo>
#include <vtkDoubleArray.h>
#include <QCoreApplication>

#include "cxRegistrationTransform.h"
#include "cxReporter.h"
#include "cxTypeConversions.h"
#include "cxPositionStorageFile.h"
#include "cxTime.h"
#include "cxEnumConverter.h"
#include "cxDummyTool.h"
#include "cxToolUsingIGSTK.h"
#include "cxIgstkTracker.h"
#include "cxToolConfigurationParser.h"
#include "cxManualToolAdapter.h"
#include "cxSettings.h"
#include "cxDataLocations.h"
#include "cxIgstkTrackerThread.h"
#include "cxPlaybackTool.h"
#include "cxLogger.h"
#include "cxPlaybackTime.h"
#include "cxTrackingPositionFilter.h"
#include "cxXMLNodeWrapper.h"
#include "cxTrackerConfigurationImpl.h"
#include "cxUtilHelpers.h"

#include "cxTrackingSystemIGSTKService.h"
#include "cxTrackingSystemDummyService.h"
#include "cxTrackingSystemPlaybackService.h"

namespace cx
{

ToolManagerUsingIGSTK::ToolManagerUsingIGSTKPtr ToolManagerUsingIGSTK::create()
{
	ToolManagerUsingIGSTKPtr retval;
	retval.reset(new ToolManagerUsingIGSTK());
	return retval;
}

QStringList ToolManagerUsingIGSTK::getSupportedTrackingSystems()
{
	QStringList retval;
	retval = IgstkTracker::getSupportedTrackingSystems();
	return retval;
}

ToolManagerUsingIGSTK::ToolManagerUsingIGSTK() :
				mLoggingFolder(""),
				mLastLoadPositionHistory(0),
				mToolTipOffset(0)
{
	this->initializeManualTool(); // do this after setting self.

	TrackingSystemServicePtr igstk(new TrackingSystemIGSTKService());
	this->installTrackingSystem(igstk);

	connect(settings(), SIGNAL(valueChangedFor(QString)), this, SLOT(globalConfigurationFileChangedSlot(QString)));
}

ToolManagerUsingIGSTK::~ToolManagerUsingIGSTK()
{
	while (!mTrackingSystems.empty())
		this->unInstallTrackingSystem(mTrackingSystems.back());
}

void ToolManagerUsingIGSTK::onSystemStateChanged()
{
	this->rebuildCachedTools();
	emit stateChanged();
}

/**Set playback mode.
 * Set endpoints into controller.
 *
 * The original tools are wrapped by playback tools. The original ones are
 * not changed, only their movement is ignored.
 */
void ToolManagerUsingIGSTK::setPlaybackMode(PlaybackTimePtr controller)
{
	if (mPlaybackSystem)
	{
		mPlaybackSystem->setState(Tool::tsNONE);
		this->unInstallTrackingSystem(mPlaybackSystem);
		this->installTrackingSystem(mPlaybackSystem->getBase());
		mPlaybackSystem.reset();
	}

	if (controller)
	{
		mPlaybackSystem.reset(new TrackingSystemPlaybackService(controller, mTrackingSystems.back(), mManualTool));
		this->unInstallTrackingSystem(mPlaybackSystem->getBase());
		this->installTrackingSystem(mPlaybackSystem);
		mPlaybackSystem->setState(Tool::tsTRACKING);
	}
}

bool ToolManagerUsingIGSTK::isPlaybackMode() const
{
	return mPlaybackSystem && (mPlaybackSystem->getState()>=Tool::tsCONFIGURED);
}

void ToolManagerUsingIGSTK::runDummyTool(DummyToolPtr tool)
{
	TrackingSystemServicePtr dummySystem;
	dummySystem.reset(new TrackingSystemDummyService(tool));
	this->installTrackingSystem(dummySystem);

	dummySystem->setState(Tool::tsTRACKING);
	this->setDominantTool(tool->getUid());
}

void ToolManagerUsingIGSTK::installTrackingSystem(TrackingSystemServicePtr system)
{
	mTrackingSystems.push_back(system);
	connect(system.get(), &TrackingSystemService::stateChanged, this, &ToolManagerUsingIGSTK::onSystemStateChanged);
	this->onSystemStateChanged();
}

void ToolManagerUsingIGSTK::unInstallTrackingSystem(TrackingSystemServicePtr system)
{
	disconnect(system.get(), &TrackingSystemService::stateChanged, this, &ToolManagerUsingIGSTK::onSystemStateChanged);

	for (unsigned i=0; i<mTrackingSystems.size(); ++i)
	{
		if (mTrackingSystems[i]!=system)
			continue;
		mTrackingSystems.erase(mTrackingSystems.begin()+i);
		break;
	}

	this->onSystemStateChanged();
}

void ToolManagerUsingIGSTK::initializeManualTool()
{
	if (!mManualTool)
	{
		//adding a manual tool as default
		mManualTool.reset(new ManualToolAdapter("ManualTool"));
		mTools["ManualTool"] = mManualTool;
		mManualTool->setVisible(true);
		connect(mManualTool.get(), &Tool::toolVisible, this, &ToolManagerUsingIGSTK::dominantCheckSlot);
		connect(mManualTool.get(), &Tool::toolTransformAndTimestamp, this, &ToolManagerUsingIGSTK::dominantCheckSlot);
	}

	Transform3D rMpr = Transform3D::Identity(); // not known: not really important either
	Transform3D prMt = rMpr.inv() * createTransformRotateY(M_PI) * createTransformRotateZ(M_PI/2);
	mManualTool->set_prMt(prMt);
	this->dominantCheckSlot();
}

Tool::State ToolManagerUsingIGSTK::getState() const
{
	Tool::State state = Tool::tsNONE;
	for (unsigned i=0; i<mTrackingSystems.size(); ++i)
		state = std::max(state, mTrackingSystems[i]->getState());
	return state;
}

void ToolManagerUsingIGSTK::setState(const Tool::State val)
{
	for (unsigned i=0; i<mTrackingSystems.size(); ++i)
		mTrackingSystems[i]->setState(val);
}

void ToolManagerUsingIGSTK::rebuildCachedTools()
{
	mTools.clear();
	for (unsigned i=0; i<mTrackingSystems.size(); ++i)
	{
		this->addToolsFrom(mTrackingSystems[i]);
	}

	mTools[mManualTool->getUid()] = mManualTool;
	this->imbueManualToolWithRealProperties();
	this->setDominantTool(this->getManualTool()->getUid());
	this->loadPositionHistory(); // the tools are always reconfigured after a setloggingfolder

//	reportSuccess("ToolManager is set to state ...");
}

void ToolManagerUsingIGSTK::imbueManualToolWithRealProperties()
{
	// debug: give the manual tool properties from the first non-manual tool. Nice for testing tools
	if (!settings()->value("giveManualToolPhysicalProperties").toBool())
		return;

	for (ToolManager::ToolMap::iterator iter = mTools.begin(); iter != mTools.end(); ++iter)
	{
		if (iter->second == mManualTool)
			continue;
		if (iter->second->hasType(Tool::TOOL_REFERENCE))
			continue;
		mManualTool->setBase(iter->second);
		report("Manual tool imbued with properties from " + iter->first);
		break;
	}
}

void ToolManagerUsingIGSTK::addToolsFrom(TrackingSystemServicePtr system)
{
	std::vector<ToolPtr> tools = system->getTools();
	for (unsigned i=0; i<tools.size(); ++i)
	{
		ToolPtr tool = tools[i];
		mTools[tool->getUid()] = tool;
		connect(tool.get(), SIGNAL(toolVisible(bool)), this, SLOT(dominantCheckSlot()));
//		connect(tool.get(), &Tool::toolTransformAndTimestamp, this, &ToolManagerUsingIGSTK::dominantCheckSlot);
		connect(tool.get(), &Tool::tooltipOffset, this, &ToolManagerUsingIGSTK::onTooltipOffset);

		if (tool->hasType(Tool::TOOL_REFERENCE))
			mReferenceTool = tool;
		if (tool->hasType(ToolUsingIGSTK::TOOL_US_PROBE))
			emit probeAvailable();
	}
}

void ToolManagerUsingIGSTK::onTooltipOffset(double val)
{
	for (ToolManager::ToolMap::iterator iter = mTools.begin(); iter != mTools.end(); ++iter)
	{
		iter->second->setTooltipOffset(val);
	}
}

SessionToolHistoryMap ToolManagerUsingIGSTK::getSessionHistory(double startTime, double stopTime)
{
	SessionToolHistoryMap retval;

	ToolManager::ToolMap tools = this->getTools();
	ToolManager::ToolMap::iterator it = tools.begin();
	for (; it != tools.end(); ++it)
	{
		TimedTransformMap toolMap = it->second->getSessionHistory(startTime, stopTime);
		if (toolMap.empty())
			continue;
		retval[it->second] = toolMap;
	}
	return retval;
}

ToolManager::ToolMap ToolManagerUsingIGSTK::getTools()
{
	return mTools;
}

ToolPtr ToolManagerUsingIGSTK::getTool(const QString& uid)
{
	if (uid == "active")
		return this->getDominantTool();

	ToolPtr retval;
	ToolManager::ToolMap::iterator it = mTools.find(uid);
	if (it != mTools.end())
		retval = it->second;

	return retval;
}

ToolPtr ToolManagerUsingIGSTK::getDominantTool()
{
	return mDominantTool;
}

void ToolManagerUsingIGSTK::setDominantTool(const QString& uid)
{
	if (mDominantTool && mDominantTool->getUid() == uid)
		return;

	ToolPtr newTool;
	newTool = this->getTool(uid);

	ToolPtr oldTool = mDominantTool;
	mDominantTool = newTool; // set dominant before calling setters, which possibly can emit signal and cause cycles.

	// special case for manual tool
	if (newTool && newTool->hasType(Tool::TOOL_MANUAL) && mManualTool)
	{
		if (oldTool && (oldTool!=mManualTool))
			mManualTool->set_prMt(oldTool->get_prMt());
		mManualTool->setVisible(true);
	}
	else
	{
		mManualTool->setVisible(false);
	}

	emit dominantToolChanged(uid);
}

ToolPtr ToolManagerUsingIGSTK::getReferenceTool() const
{
	return mReferenceTool;
}

void ToolManagerUsingIGSTK::savePositionHistory()
{
	QString filename = mLoggingFolder + "/toolpositions.snwpos";

	PositionStorageWriter writer(filename);

	ToolManager::ToolMap::iterator it = mTools.begin();
	for (; it != mTools.end(); ++it)
	{
		ToolPtr current = it->second;
		TimedTransformMapPtr data = current->getPositionHistory();

		if (!data)
			continue;

		// save only data acquired after mLastLoadPositionHistory:
		TimedTransformMap::iterator iter = data->lower_bound(mLastLoadPositionHistory);
		for (; iter != data->end(); ++iter)
			writer.write(iter->second, (iter->first), current->getUid());
	}

	mLastLoadPositionHistory = getMilliSecondsSinceEpoch();
}

void ToolManagerUsingIGSTK::loadPositionHistory()
{
	if (this->getState()==Tool::tsNONE)
		return;
	// save all position data acquired so far, in case of multiple calls.
	this->savePositionHistory();

	QString filename = mLoggingFolder + "/toolpositions.snwpos";

	PositionStorageReader reader(filename);

	Transform3D matrix = Transform3D::Identity();
	double timestamp;
	QString toolUid;

	QStringList missingTools;

	while (!reader.atEnd())
	{
		if (!reader.read(&matrix, &timestamp, &toolUid))
			break;

		ToolPtr current = this->getTool(toolUid);
		if (current)
		{
			(*current->getPositionHistory())[timestamp] = matrix;
		}
		else
		{
			missingTools << toolUid;
		}
	}

	missingTools.removeDuplicates();
	missingTools.removeAll("");

	if (!missingTools.empty())
	{
		reportWarning(QString("Loaded position history, but some of the tools "
							  "are not present in the configuration:"
							  "\n  \t%1").arg(missingTools.join("\n  \t")));
	}

	mLastLoadPositionHistory = getMilliSecondsSinceEpoch();
}

void ToolManagerUsingIGSTK::setLoggingFolder(QString loggingFolder)
{
	if (mLoggingFolder == loggingFolder)
		return;

	for (unsigned i=0; i<mTrackingSystems.size(); ++i)
		mTrackingSystems[i]->setLoggingFolder(loggingFolder);
	mLoggingFolder = loggingFolder;
}

void ToolManagerUsingIGSTK::globalConfigurationFileChangedSlot(QString key)
{
	if (key.contains("TrackingPositionFilter"))
	{
		this->resetTrackingPositionFilters();
	}
}

void ToolManagerUsingIGSTK::resetTrackingPositionFilters()
{
	bool enabled = settings()->value("TrackingPositionFilter/enabled", false).toInt();

	for (ToolMap::iterator iter=mTools.begin(); iter!=mTools.end(); ++iter)
	{
		TrackingPositionFilterPtr filter;
		if (enabled)
			filter.reset(new TrackingPositionFilter());
		iter->second->resetTrackingPositionFilter(filter);
	}
}

void ToolManagerUsingIGSTK::dominantCheckSlot()
{
	if (this->manualToolHasMostRecentTimestamp())
	{
		this->setDominantTool(this->getManualTool()->getUid());
		return;
	}

	bool use = settings()->value("Automation/autoSelectDominantTool").toBool();
	if (!use)
		return;

	//make a sorted vector of all visible tools
	std::vector<ToolPtr> tools = this->getVisibleTools();
	tools.push_back(mManualTool);

	if (!tools.empty())
	{
		//sort most important tool to the start of the vector:
		sort(tools.begin(), tools.end(), toolTypeSort);
		const QString uid = tools[0]->getUid();
		this->setDominantTool(uid);
	}
}

bool ToolManagerUsingIGSTK::manualToolHasMostRecentTimestamp()
{
	// original comment (was wrapped in an ifplayblack):
	// In static playback mode, tools does not turn invisible since
	// time dont move. Here we check whether manual tool has a newer
	// timestamp than the playback tools. If it has, make it dominant.
	// This enables automatic change to manual tool if the user
	// manipulates the manual tool in some way.

	double bestTime = 0;
	for (ToolMap::iterator it = mTools.begin(); it != mTools.end(); ++it)
	{
		if (it->second->hasType(Tool::TOOL_MANUAL))
			continue;
		bestTime = std::max(bestTime, it->second->getTimestamp());
	}

	return (this->getManualTool()->getTimestamp() >= bestTime);
}

std::vector<ToolPtr> ToolManagerUsingIGSTK::getVisibleTools()
{
	std::vector<ToolPtr> retval;
	for (ToolMap::iterator it = mTools.begin(); it != mTools.end(); ++it)
		if (it->second->getVisible())
			retval.push_back(it->second);
	return retval;
}



namespace
{
/**
 * \return a priority of the tool. High means this tool is preferred more.
 */
int getPriority(ToolPtr tool)
{
	if (tool->hasType(Tool::TOOL_MANUAL)) // place this first, in case a tool has several attributes.
		return 2;

	if (tool->hasType(Tool::TOOL_US_PROBE))
		return 4;
	if (tool->hasType(Tool::TOOL_POINTER))
		return 3;
	if (tool->hasType(Tool::TOOL_REFERENCE))
		return 1;
	return 0;
}
}

/**
 * sorts tools in descending order of type
 * @param tool1 the first tool
 * @param tool2 the second tool
 * @return whether the second tool is of higher priority than the first or not
 */
bool toolTypeSort(const ToolPtr tool1, const ToolPtr tool2)
{
	return getPriority(tool2) < getPriority(tool1);
}

void ToolManagerUsingIGSTK::addXml(QDomNode& parentNode)
{
	XMLNodeAdder parent(parentNode);
	XMLNodeAdder base(parent.addElement("toolManager"));

	base.addTextToElement("toolTipOffset", qstring_cast(mToolTipOffset));
	base.addTextToElement("manualTool", "\n" + qstring_cast(mManualTool->get_prMt()));

	//Tools
	XMLNodeAdder toolsNode(base.addElement("tools"));
	ToolManager::ToolMap tools = this->getTools();
	ToolManager::ToolMap::iterator toolIt = tools.begin();
	for (; toolIt != tools.end(); ++toolIt)
	{
		cxToolPtr tool = boost::dynamic_pointer_cast<ToolUsingIGSTK>(toolIt->second);
		if (tool)
		{
			toolsNode.addObjectToElement("tool", tool);
		}
	}
}

void ToolManagerUsingIGSTK::clear()
{
	mManualTool->set_prMt(Transform3D::Identity());
}

void ToolManagerUsingIGSTK::parseXml(QDomNode& dataNode)
{
	if (dataNode.isNull())
		return;

	XMLNodeParser base(dataNode);

	QString manualToolText = dataNode.namedItem("manualTool").toElement().text();
	mManualTool->set_prMt(Transform3D::fromString(manualToolText));

	mToolTipOffset = base.parseDoubleFromElementWithDefault("toolTipOffset", 0.0);

	//Tools
	ToolManager::ToolMap tools = this->getTools();
//	QDomNode toolssNode = dataNode.namedItem("tools");
	XMLNodeParser toolssNode(dataNode.namedItem("tools"));
	std::vector<QDomElement> toolNodes = toolssNode.getDuplicateElements("tool");

	for (unsigned i=0; i<toolNodes.size(); ++i)
	{
		QDomElement toolNode = toolNodes[i];
		QString tool_uid = toolNode.attribute("uid");
		if (tools.find(tool_uid) != tools.end())
		{
			cxToolPtr tool = boost::dynamic_pointer_cast<ToolUsingIGSTK>(tools.find(tool_uid)->second);
			tool->parseXml(toolNode);
		}
	}
}

ManualToolPtr ToolManagerUsingIGSTK::getManualTool()
{
	return mManualTool;
}

/**\brief Find a probe that can be connected to a rt source.
 *
 * Priority:
 *  - active probe
 *  - visible probe
 *  - any probe
 *
 */
ToolPtr ToolManagerUsingIGSTK::findFirstProbe()
{
	ToolPtr active = this->getDominantTool();
	if (active && active->getProbe() && active->getProbe()->isValid())
		return active;

	ToolManager::ToolMap tools = this->getTools();

	// look for visible probes
	for (ToolManager::ToolMap::iterator iter = tools.begin(); iter != tools.end(); ++iter)
		if (iter->second->getProbe() && iter->second->getProbe()->isValid() && iter->second->getVisible())
			return iter->second;

	// pick the first probe, visible or not.
	for (ToolManager::ToolMap::iterator iter = tools.begin(); iter != tools.end(); ++iter)
		if (iter->second->getProbe() && iter->second->getProbe()->isValid())
			return iter->second;

	return ToolPtr();
}


TrackerConfigurationPtr ToolManagerUsingIGSTK::getConfiguration()
{
	for (unsigned i=0; i<mTrackingSystems.size(); ++i)
	{
		TrackerConfigurationPtr config = mTrackingSystems[i]->getConfiguration();
		if (config)
			return config;
	}
	return TrackerConfigurationPtr();
}


} //namespace cx
