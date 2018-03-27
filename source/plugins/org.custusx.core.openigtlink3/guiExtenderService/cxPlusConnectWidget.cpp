/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxPlusConnectWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProcess>
#include <QComboBox>
#include <QLabel>
#include <QFileDialog>
#include <QSpacerItem>
#include "cxLogger.h"
#include "cxOpenIGTLinkStreamerService.h"
#include "cxVisServices.h"
#include "cxVideoService.h"
#include "cxBoolProperty.h"
#include "cxProfile.h"
#include "cxProcessWrapper.h"
#include "cxSettings.h"
#include "cxDataLocations.h"
#include "cxFileSelectWidget.h"
#include "cxCheckBoxWidget.h"
#include "cxFilePreviewWidget.h"

#define START_TEXT "Start PLUS server and connect tracking and streaming"
#define STOP_TEXT "Stop PLUS server and disconnect tracking and streaming"

namespace cx
{
PlusConnectWidget::PlusConnectWidget(VisServicesPtr services, QWidget* parent) :
  BaseWidget(parent, "plus_connect_widget" ,"Connect to PLUS"),
  mServices(services),
	mPlusRunning(false),
	mExternalProcess(new ProcessWrapper("PLUS")),
	mFilePreviewWidget(NULL),
	mPlusPathComboBox(NULL)
{
	QVBoxLayout* toplayout = new QVBoxLayout(this);

	QGridLayout* layout = new QGridLayout();
	toplayout->addLayout(layout);
//	layout->setMargin(0);
	int line = 0;

	// PLUS path
	mPlusPath = settings()->value("plus/Path").toString();
	if(!QFile::exists(mPlusPath))
		this->searchForPlus();

	QLabel* plusPathLabel = new QLabel(tr("PLUS path:"));
	mPlusPathComboBox = new QComboBox();
	mPlusPathComboBox->addItem(mPlusPath);
	QAction* browsePlusPathAction = new QAction(QIcon(":/icons/open.png"), tr("Browse for PLUS..."), this);
	connect(browsePlusPathAction, &QAction::triggered, this, &PlusConnectWidget::browsePlusPathSlot);
	QToolButton* browsePlusPathButton = new QToolButton(this);
	browsePlusPathButton->setDefaultAction(browsePlusPathAction);

	layout->addWidget(plusPathLabel, line, 0);
	layout->addWidget(mPlusPathComboBox, line, 1);
	layout->addWidget(browsePlusPathButton, line, 2);
	++line;

	layout->addWidget(new QLabel("PLUS config File:", this), line, 0);
	mPlusConfigFileWidget = new FileSelectWidget(this);
	mPlusConfigFileWidget->setNameFilter(QStringList("*.xml"));
	mPlusConfigFileWidget->setPaths(this->getPlusConfigFilePaths());
	mPlusConfigFileWidget->setFilename("Select PLUS config file...");
	connect(mPlusConfigFileWidget, &FileSelectWidget::fileSelected, this, &PlusConnectWidget::configFileFileSelected);
	layout->addWidget(mPlusConfigFileWidget, line, 1, 1, 2);
	++line;


	mShowPlusOutput = BoolProperty::initialize("showPlusOutput",
		"Show PLUS output",
		"Display all output from PlusServer",
		false);
	layout->addWidget(new CheckBoxWidget(this, mShowPlusOutput), line, 0, 1, 3);
	++line;

	mConnectButton = new QPushButton(START_TEXT);
	mConnectButton->setToolTip("Remove all saved clip planes from the selected volume");
	connect(mConnectButton, &QPushButton::clicked, this, &PlusConnectWidget::connectButtonClickedSlot);
	layout->addWidget(mConnectButton, line, 0, 1, 3);
	++line;

	mFilePreviewWidget = new FilePreviewWidget(this);
	mFilePreviewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->addWidget(mFilePreviewWidget, line, 0, 1, 3);
	++line;


//	toplayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

	connect(mExternalProcess.get(), &ProcessWrapper::stateChanged, this, &PlusConnectWidget::plusAppStateChanged);
}

void PlusConnectWidget::searchForPlus()
{
	QString possiblePath = QDir::homePath() + "/dev/plus-2.6/PlusB-bin/bin/PlusServer";

	if(QFile::exists(possiblePath))
	{
		CX_LOG_DEBUG() << "Found PLUS in: " << possiblePath;
		mPlusPath = possiblePath;
	}
}

QStringList PlusConnectWidget::getPlusConfigFilePaths()
{
	QStringList retval;
	retval << DataLocations::getRootConfigPath() + "/plus"; //Installed
	retval << QString(CX_OPTIONAL_CONFIG_ROOT) + "/plus";   //CustusS build tree
	return retval;
}

void PlusConnectWidget::searchForPlusConfigFile()
{
	// CustusX app folder config/tool
//	QString possiblePath = QDir::homePath() + "/dev/plus-2.6/PlusB-bin/bin/PlusServer";
	QString possiblePath = DataLocations::getRootConfigPath() + "/plus/PlusDeviceSet_Server_Bk_Polaris.xml";

	if(QFile::exists(possiblePath))
	{
		CX_LOG_DEBUG() << "Found PLUS config file in: " << possiblePath;
		mPlusConfigFile = possiblePath;
	}
}

void PlusConnectWidget::browsePlusPathSlot()
{
	QFileInfo fileInfo(mPlusPath);
	mPlusPath = QFileDialog::getOpenFileName(this, tr("Find PLUS executable"), fileInfo.absolutePath());

	if(!mPlusPath.isEmpty())
	{
		mPlusPathComboBox->addItem( mPlusPath );
		mPlusPathComboBox->setCurrentIndex( mPlusPathComboBox->currentIndex() + 1 );
	}
}

void PlusConnectWidget::configFileFileSelected(QString filename)
{
	mPlusConfigFile = filename;

	QFileInfo fileinfo(filename);
	mPlusConfigFileWidget->setPath(fileinfo.absolutePath());

	mFilePreviewWidget->previewFileSlot(filename);
}


void PlusConnectWidget::connectButtonClickedSlot()
{
	if(mPlusRunning)
	{
		CX_LOG_INFO() << "Stopping PLUS server and disconnecting";
		if(this->stopPlus())
		{
			mConnectButton->setText(START_TEXT);
			mPlusRunning = false;
		}
		else
			CX_LOG_WARNING() << "Failed to stop/disconnect PLUS server";
	}
	else
	{
		CX_LOG_INFO() << "Starting PLUS server and connecting";
		if(this->startPlus())
		{
			mConnectButton->setText(STOP_TEXT);
			mPlusRunning = true;
		}
		else
			CX_LOG_WARNING() << "Failed to start/connect PLUS server";
	}
}
bool PlusConnectWidget::stopPlus()
{
	StreamerServicePtr streamerService = this->getStreamerService();
	if(!streamerService)
		return false;

	//Trigger OpenIGTLinkStreamerService::stopTrackingAndOpenIGTLinkClientIfStartedFromThisObject
	streamerService->stop();

	if(mExternalProcess->isRunning())
		mExternalProcess->getProcess()->close();

	//Stop video streaming
	mServices->video()->closeConnection();

	//Stop output
	disconnect(mExternalProcess->getProcess(), &QProcess::readyRead, this, &PlusConnectWidget::processReadyRead);


	return mExternalProcess->waitForFinished(1000);
//	mExternalProcess.reset();
}

bool PlusConnectWidget::startPlus()
{
	StreamerServicePtr streamerService = this->getStreamerService();
	if(!streamerService)
		return false;

	if(!this->startExternalPlusServer())
		 return false;

	this->turnOnStartTrackingInOpenIGTLinkStreamer(streamerService);
	streamerService->createStreamer(this->getXmlVideoElement());

	//Start video streaming
	mServices->video()->openConnection();

	return true;
}

void PlusConnectWidget::plusAppStateChanged()
{
	if(mExternalProcess->getProcess()->state() == QProcess::Starting)
	{
		mConnectButton->setText("PLUS starting...");
		return;
	}

	if(!mExternalProcess->isRunning())
	{
		CX_LOG_DEBUG() << "plusAppStateChanged not running. Stopping...";
		mPlusRunning = false;
		mConnectButton->setText(START_TEXT);
		this->stopPlus();
	}
	else
	{
		mPlusRunning = true;
		mConnectButton->setText(STOP_TEXT);
	}

}

bool PlusConnectWidget::startExternalPlusServer()
{
	if(!this->configFileIsValid())
		return false;

	QStringList arguments;

	arguments << QString("--config-file=").append(mPlusConfigFile);

	//Show PLUS output
	if(mShowPlusOutput->getValue())
	{
		QProcess *process = mExternalProcess->getProcess();
		connect(process, &QProcess::readyRead, this, &PlusConnectWidget::processReadyRead);
		process->setProcessChannelMode(QProcess::MergedChannels);
		process->setReadChannel(QProcess::StandardOutput);
	}

	mExternalProcess->launch(mPlusPath, arguments);
	return mExternalProcess->waitForStarted(1000);
}

bool PlusConnectWidget::configFileIsValid()
{
	QFileInfo fileInfo(mPlusConfigFile);
	if(fileInfo.exists())
		return true;
	else
	{
		CX_LOG_WARNING() << "Plus config file is not existing: " << mPlusConfigFile;
		return false;
	}
}

void PlusConnectWidget::processReadyRead()
{
	report(QString(mExternalProcess->getProcess()->readAllStandardOutput()));
}

QDomElement PlusConnectWidget::getXmlVideoElement()
{
	XmlOptionFile xmlFile = profile()->getXmlSettings().descend("video");
	QDomElement element = xmlFile.getElement("video");
	return element;
}

void PlusConnectWidget::turnOnStartTrackingInOpenIGTLinkStreamer(StreamerServicePtr streamerService)
{
	QDomElement element = this->getXmlVideoElement();

	std::vector<PropertyPtr> settings = streamerService->getSettings(element);
	for(unsigned i = 0; i < settings.size(); ++i)
	{
		if (settings[i]->getUid().contains("start_tracking"))
		{
			CX_LOG_DEBUG() << "Turning on track and stream in OpenIGTLinkStreamer";
			settings[i]->setValueFromVariant(true);
		}
	}
}

StreamerServicePtr PlusConnectWidget::getStreamerService()
{
	StreamerServicePtr streamer = mServices->video()->getStreamerService(OPENIGTLINK3_STREAMER);
	if(!streamer)
		CX_LOG_WARNING() << "PlusConnectWidget::getOpenIGTLinkStreamerService(): Cannot get StreamerServicePtr";

	return streamer;
}

}//namespace cx
