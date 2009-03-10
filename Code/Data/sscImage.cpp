#include "sscImage.h"

#include <vtkImageReslice.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkDoubleArray.h>
#include <vtkLookupTable.h>
#include <sstream>

#define USE_TRANSFORM_RESCLICER

namespace ssc
{

Image::~Image()
{
}

Image::Image(const std::string& uid, const vtkImageDataPtr& data) :
	mImageTransferFunctions3D(data),
	mImageLookupTable2D(data),
	mUid(uid), mName(uid), mBaseImageData(data),
	mLandmarks(vtkDoubleArray::New())
{
	mOutputImageData = mBaseImageData;
#ifdef USE_TRANSFORM_RESCLICER
	//mBaseImageData->ReleaseDataFlagOn();
	mOrientator = vtkImageReslicePtr::New();
	mOrientator->AutoCropOutputOn();	
	mOrientator->SetInput(mBaseImageData);
	mOutputImageData = mOrientator->GetOutput();
	mOutputImageData->Update();
	//mOutputImageData->UpdateInformation();
#endif		
	mLandmarks->SetNumberOfComponents(4);
	mOutputImageData->GetScalarRange();	// this line updates some internal vtk value, and (on fedora) removes 4.5s in the second render().
	mAlpha = 0.5;
	mTreshold = 1.0; 
}

void Image::setVtkImageData(const vtkImageDataPtr& data)
{
	mBaseImageData = data;
#ifdef USE_TRANSFORM_RESCLICER
	mOrientator->SetInput(mBaseImageData);
	mOrientator->SetResliceAxes(mTransform.matrix());
	mOutputImageData->Update();
	mOutputImageData->UpdateInformation();
	mOutputImageData->GetScalarRange();	// this line updates some internal vtk value, and (on fedora) removes 4.5s in the second render().
#endif
	mImageTransferFunctions3D = ImageTF3D(data);
	mImageLookupTable2D = ImageLUT2D(data);
	
	emit vtkImageDataChanged();
}
void Image::setClut(vtkLookupTablePtr clut)
{
	mImageLookupTable2D.setLookupTable( clut );
	std::cout<<"ssc::Image, set new lut"<<std::endl;
}

double Image::treshold()
{
	return mTreshold;
}

void Image::setTreshold( double val )
{
}

double Image::getAlpha()
{ 
	return mAlpha; 
}

void Image::setAlpha(double val)
{
	mAlpha = val;
	emit alphaChange();
}

ImageTF3D& Image::transferFunctions3D()
{
	return mImageTransferFunctions3D;
}

ImageLUT2D& Image::lookupTable2D()
{
	return mImageLookupTable2D;
}

void Image::setName(const std::string& name)
{
	mName = name;
}

void Image::setTransform(const Transform3D& trans)
{
	//std::cout << "Image::setTransform(): \n" << trans << std::endl;
	if (similar(trans, mTransform))
	{
		return;
	}

	mTransform = trans;

#ifdef USE_TRANSFORM_RESCLICER
	mOrientator->SetResliceAxes(mTransform.matrix());
	mOutputImageData->Update();
	mOutputImageData->UpdateInformation();
	mOutputImageData->GetScalarRange();	// this line updates some internal vtk value, and (on fedora) removes 4.5s in the second render().
#endif

	emit transformChanged();
}

std::string Image::getUid() const
{
	return mUid;
}

std::string Image::getName() const
{
	return mName;
}

Transform3D Image::getTransform() const
{
	return mTransform;
}

REGISTRATION_STATUS Image::getRegistrationStatus() const
{
	return rsNOT_REGISTRATED;
}

vtkImageDataPtr Image::getBaseVtkImageData()
{
	return mBaseImageData;
}

vtkImageDataPtr Image::getRefVtkImageData()
{
	return mOutputImageData;
}

vtkDoubleArrayPtr Image::getLandmarks()
{
	return mLandmarks;
}
void Image::connectRep(const RepWeakPtr& rep)
{
	mReps.insert(rep);
}
void Image::disconnectRep(const RepWeakPtr& rep)
{
	mReps.erase(rep);
}
/** If index is found, it's treated as an edit operation, else
 * it's an add operation.
 * @param x
 * @param y
 * @param z
 * @param index
 */
void Image::addLandmarkSlot(double x, double y, double z, unsigned int index)
{
	double addLandmark[4] = {x, y, z, (double)index};

	int numberOfLandmarks = mLandmarks->GetNumberOfTuples();
	//if index exists, we treat it as an edit operation
	for(int i=0; i<= numberOfLandmarks-1; i++)
	{
		double* landmark = mLandmarks->GetTuple(i);
		if(landmark[3] == index)
		{
			mLandmarks->SetTupleValue(i, addLandmark);
			emit landmarkAdded(x, y, z, index);
			return;
		}
	}
	//else it's an add operation
	mLandmarks->InsertNextTupleValue(addLandmark);
	emit landmarkAdded(x, y, z, index);
}
/** If index is found that tuple(landmark) is removed from the array, else
 * it's just ignored.
 * @param x
 * @param y
 * @param z
 * @param index
 */
void Image::removeLandmarkSlot(double x, double y, double z, unsigned int index)
{
	int numberOfLandmarks = mLandmarks->GetNumberOfTuples();
	for(int i=0; i<= numberOfLandmarks-1; i++)
	{
		double* landmark = mLandmarks->GetTuple(i);
		if(landmark[3] == index)
		{
			mLandmarks->RemoveTuple(i);
			emit landmarkRemoved(x, y, z, index);
		}
	}
}
void Image::printLandmarks()
{
	std::cout << "Landmarks: " << std::endl;
	for(int i=0; i<= mLandmarks->GetNumberOfTuples()-1; i++)
	{
		double* landmark = mLandmarks->GetTuple(i);
		std::stringstream stream;
		stream << i << ": (";
		stream << landmark[0];
		stream << ",";
		stream << landmark[1];
		stream << ",";
		stream << landmark[2];
		stream << ",";
		stream << landmark[3];
		stream << ")";
		std::cout << stream.str() << std::endl;
	}
}

DoubleBoundingBox3D Image::boundingBox() const
{
	mOutputImageData->UpdateInformation();
	DoubleBoundingBox3D bounds(mOutputImageData->GetBounds());
	return bounds;
}

} // namespace ssc
