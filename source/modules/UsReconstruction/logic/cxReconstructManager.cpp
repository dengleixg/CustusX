// This file is part of SSC,
// a C++ Library supporting Image Guided Therapy Applications.
//
// Copyright (C) 2008- SINTEF Medical Technology
// Copyright (C) 2008- Sonowand AS
//
// SSC is owned by SINTEF Medical Technology and Sonowand AS,
// hereafter named the owners. Each particular piece of code
// is owned by the part that added it to the library.
// SSC source code and binaries can only be used by the owners
// and those with explicit permission from the owners.
// SSC shall not be distributed to anyone else.
//
// SSC is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.
//
// See sscLicense.txt for more information.

#include "cxReconstructManager.h"

#include "cxStringDataAdapterXml.h"
#include "cxDoubleDataAdapterXml.h"
#include "cxBoolDataAdapterXml.h"
#include "cxCompositeTimedAlgorithm.h"
#include "cxReconstructThreads.h"
#include "cxUSFrameData.h"
#include "cxUsReconstructionFileReader.h"
#include "cxReconstructPreprocessor.h"
#include "cxReconstructParams.h"
#include "cxReconstructAlgorithm.h"


//Windows fix
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cx
{


ReconstructManager::ReconstructManager(XmlOptionFile settings, QString shaderPath) :
	mOutputRelativePath(""), mOutputBasePath(""), mShaderPath(shaderPath)
{
	mSettings = settings;
	mSettings.getElement("algorithms");

	mParams.reset(new ReconstructParams(settings));
	connect(mParams.get(), SIGNAL(changedInputSettings()), this, SLOT(setSettings()));
	connect(mParams.get(), SIGNAL(transferFunctionChanged()), this, SLOT(transferFunctionChangedSlot()));

	this->initAlgorithm();
}

ReconstructManager::~ReconstructManager()
{

}

ReconstructAlgorithmPtr ReconstructManager::createAlgorithm()
{
	QString name = mParams->mAlgorithmAdapter->getValue();

	ReconstructCorePtr core; ///< in progress: algorithm part of class moved here.
	core.reset(new ReconstructCore());
	ReconstructAlgorithmPtr algo = core->createAlgorithm(name);
	return algo;
}

void ReconstructManager::initAlgorithm()
{
	ReconstructAlgorithmPtr algo = this->createAlgorithm();

	// generate settings for new algo
	if (algo)
	{
		QDomElement element = mSettings.getElement("algorithms", algo->getName());
		mAlgoOptions = algo->getSettings(element);
		emit algorithmChanged();
	}
}

void ReconstructManager::setSettings()
{
	this->initAlgorithm();
	this->updateFromOriginalFileData();
	emit paramsChanged();
}

void ReconstructManager::transferFunctionChangedSlot()
{
	//Use angio reconstruction also if only transfer function is set to angio
	if(mParams->mPresetTFAdapter->getValue() == "US Angio")
	{
		reportDebug("Reconstructing angio (Because of angio transfer function)");
		mParams->mAngioAdapter->setValue(true);
	}
	else if(mParams->mPresetTFAdapter->getValue() == "US B-Mode" && mParams->mAngioAdapter->getValue())
	{
		reportDebug("Not reconstructing angio (Because of B-Mode transfer function)");
		mParams->mAngioAdapter->setValue(false);
	}
}

std::vector<ReconstructCorePtr> ReconstructManager::startReconstruction()
{
	std::vector<ReconstructCorePtr> cores = this->createCores();

	if (cores.empty())
	{
		reportWarning("Failed to start reconstruction");
		return cores;
	}

	cx::CompositeTimedAlgorithmPtr algorithm = this->assembleReconstructionPipeline(cores);

	this->launch(algorithm);

	return cores;
}

std::set<cx::TimedAlgorithmPtr> ReconstructManager::getThreadedReconstruction()
{
	return mThreadedReconstruction;
}

void ReconstructManager::launch(cx::TimedAlgorithmPtr thread)
{
	mThreadedReconstruction.insert(thread);
	emit reconstructAboutToStart();
	connect(thread.get(), SIGNAL(finished()), this, SLOT(threadFinishedSlot())); // connect after emit, to allow listeners to get thread at finish
	thread->execute();
}

void ReconstructManager::threadFinishedSlot()
{
	std::set<cx::TimedAlgorithmPtr>::iterator iter;
	for(iter=mThreadedReconstruction.begin(); iter!=mThreadedReconstruction.end(); )
	{
		if ((*iter)->isFinished())
		{
			mThreadedReconstruction.erase(iter);
			iter = mThreadedReconstruction.begin();
		}
		else
			++iter;
	}

	if (mThreadedReconstruction.empty())
		mOriginalFileData.mUsRaw->purgeAll();
}

void ReconstructManager::clearAll()
{
	mOriginalFileData = USReconstructInputData();
	mOutputVolumeParams = OutputVolumeParams();
}

OutputVolumeParams ReconstructManager::getOutputVolumeParams() const
{
	return mOutputVolumeParams;
}

void ReconstructManager::setOutputVolumeParams(const OutputVolumeParams& par)
{
	mOutputVolumeParams = par;
	this->setSettings();
}

void ReconstructManager::setOutputRelativePath(QString path)
{
	mOutputRelativePath = path;
}

void ReconstructManager::setOutputBasePath(QString path)
{
	mOutputBasePath = path;
}

bool ReconstructManager::validInputData() const
{
	if (mOriginalFileData.mFrames.empty() || !mOriginalFileData.mUsRaw || mOriginalFileData.mPositions.empty())
		return false;
	if(mOriginalFileData.mUsRaw->is4D())
	{
		reportWarning("US reconstructer do not handle 4D US data");
		return false;
	}
	return true;
}

cx::CompositeTimedAlgorithmPtr ReconstructManager::assembleReconstructionPipeline(std::vector<ReconstructCorePtr> cores)
{
	cx::CompositeSerialTimedAlgorithmPtr pipeline(new cx::CompositeSerialTimedAlgorithm("US Reconstruction"));

	ReconstructPreprocessorPtr preprocessor = this->createPreprocessor();
	pipeline->append(ThreadedTimedReconstructPreprocessor::create(preprocessor, cores));

	cx::CompositeTimedAlgorithmPtr temp = pipeline;
	if(this->canCoresRunInParallel(cores))
	{
		cx::CompositeParallelTimedAlgorithmPtr parallel(new cx::CompositeParallelTimedAlgorithm());
		pipeline->append(parallel);
		temp = parallel;
		reportDebug("Running reconstruction cores in parallel.");
	}

	for (unsigned i=0; i<cores.size(); ++i)
		temp->append(ThreadedTimedReconstructCore::create(cores[i]));

	return pipeline;
}

bool ReconstructManager::canCoresRunInParallel(std::vector<ReconstructCorePtr> cores)
{
	bool parallelizable = true;

	std::vector<ReconstructCorePtr>::iterator it;
	for(it = cores.begin(); it != cores.end(); ++it)
		parallelizable = parallelizable && (it->get()->getInputParams().mAlgorithmUid == "PNN");

	return parallelizable;
}

ReconstructParamsPtr ReconstructManager::getParams()
{
	return mParams;
}

std::vector<DataAdapterPtr> ReconstructManager::getAlgoOptions()
{
	return mAlgoOptions;
}

XmlOptionFile ReconstructManager::getSettings()
{
	return mSettings;
}

QString ReconstructManager::getSelectedFilename() const
{
	return mOriginalFileData.mFilename;
}

USReconstructInputData ReconstructManager::getSelectedFileData()
{
	return mOriginalFileData;
}

void ReconstructManager::selectData(QString filename, QString calFilesPath)
{
	if (filename.isEmpty())
	{
		reportWarning("no file selected");
		return;
	}

	cx::UsReconstructionFileReaderPtr fileReader(new cx::UsReconstructionFileReader());
	USReconstructInputData fileData = fileReader->readAllFiles(filename, calFilesPath);
	fileData.mFilename = filename;
	this->selectData(fileData);
}

void ReconstructManager::selectData(USReconstructInputData fileData)
{
	this->clearAll();
	mOriginalFileData = fileData;
	this->updateFromOriginalFileData();
	emit inputDataSelected(fileData.mFilename);
}

void ReconstructManager::updateFromOriginalFileData()
{
	if (!this->validInputData())
		return;

	ReconstructPreprocessorPtr preprocessor = this->createPreprocessor();
	mOutputVolumeParams = preprocessor->getOutputVolumeParams();

	emit paramsChanged();
}

ReconstructCore::InputParams ReconstructManager::createCoreParameters()
{
	ReconstructCore::InputParams par;
	par.mAlgorithmUid = mParams->mAlgorithmAdapter->getValue();
	par.mAlgoSettings = mSettings.getElement("algorithms", par.mAlgorithmUid).cloneNode(true).toElement();
	par.mOutputBasePath = mOutputBasePath;
	par.mOutputRelativePath = mOutputRelativePath;
	par.mShaderPath = mShaderPath;
	par.mAngio = mParams->mAngioAdapter->getValue();
	par.mTransferFunctionPreset = mParams->mPresetTFAdapter->getValue();
	par.mMaxOutputVolumeSize = mParams->mMaxVolumeSize->getValue();
	par.mExtraTimeCalibration = mParams->mTimeCalibration->getValue();
	par.mAlignTimestamps = mParams->mAlignTimestamps->getValue();
	par.mMaskReduce = mParams->mMaskReduce->getValue().toDouble();
	par.mOrientation = mParams->mOrientationAdapter->getValue();
	return par;
}

ReconstructPreprocessorPtr ReconstructManager::createPreprocessor()
{
	if (!this->validInputData())
		return ReconstructPreprocessorPtr();

	ReconstructPreprocessorPtr retval(new ReconstructPreprocessor());

	ReconstructCore::InputParams par = this->createCoreParameters();

	USReconstructInputData fileData = mOriginalFileData;
	fileData.mUsRaw = mOriginalFileData.mUsRaw->copy();

	retval->initialize(par, fileData);

	return retval;
}

std::vector<ReconstructCorePtr> ReconstructManager::createCores()
{
	std::vector<ReconstructCorePtr> retval;

	// create both
	if (mParams->mCreateBModeWhenAngio->getValue() && mParams->mAngioAdapter->getValue())
	{
		ReconstructCorePtr core = this->createBModeCore();
		if (core)
			retval.push_back(core);
		core = this->createCore();
		if (core)
			retval.push_back(core);
	}
	// only one thread
	else
	{
		ReconstructCorePtr core = this->createCore();
		if (core)
			retval.push_back(core);
	}

	return retval;
}

ReconstructCorePtr ReconstructManager::createCore()
{
	if (!this->validInputData())
		return ReconstructCorePtr();

	ReconstructCorePtr retval(new ReconstructCore());

	ReconstructCore::InputParams par = this->createCoreParameters();
	retval->initialize(par);

	return retval;
}

ReconstructCorePtr ReconstructManager::createBModeCore()
{
	if (!this->validInputData())
		return ReconstructCorePtr();

	ReconstructCorePtr retval(new ReconstructCore());

	ReconstructCore::InputParams par = this->createCoreParameters();
	par.mAngio = false;
	par.mTransferFunctionPreset = "US B-Mode";

	retval->initialize(par);

	return retval;
}


}