#include "cxPatientRegistrationDockWidget.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include <vtkDoubleArray.h>

#include "sscDataManager.h"
#include "cxRegistrationManager.h"
#include "cxMessageManager.h"
#include "cxToolManager.h"

/**
 * cxPatientRegistrationDockWidget.cpp
 *
 * \brief
 *
 * \date Feb 3, 2009
 * \author: Janne Beate Bakeng, SINTEF
 */

namespace cx
{
PatientRegistrationDockWidget::PatientRegistrationDockWidget() :
  mGuiContainer(new QWidget(this)),
  mVerticalLayout(new QVBoxLayout(mGuiContainer)),
  mImagesComboBox(new QComboBox(mGuiContainer)),
  mLandmarkTableWidget(new QTableWidget(mGuiContainer)),
  mToolSampleButton(new QPushButton("Sample Tool", mGuiContainer)),
  mDoRegistrationButton(new QPushButton("Do Registration", mGuiContainer)),
  mAccuracyLabel(new QLabel(QString(" "),mGuiContainer)),
  mDataManager(DataManager::getInstance()),
  mRegistrationManager(RegistrationManager::getInstance()),
  mToolManager(ToolManager::getInstance()),
  mMessageManager(MessageManager::getInstance()),
  mCurrentRow(-1),
  mCurrentColumn(-1)
  //mToolToSample(mToolManager->getDominantTool())

{
  this->setWindowTitle("Patient Registration");
  this->setWidget(mGuiContainer);

  mVerticalLayout->addWidget(mImagesComboBox);
  mVerticalLayout->addWidget(mLandmarkTableWidget);
  mVerticalLayout->addWidget(mToolSampleButton);
  mVerticalLayout->addWidget(mDoRegistrationButton);
  mVerticalLayout->addWidget(mAccuracyLabel);

  //combobox
  mImagesComboBox->setEditable(false);
  connect(this, SIGNAL(visibilityChanged(bool)),
          this, SLOT(visibilityOfDockWidgetChangedSlot(bool)));

  connect(mImagesComboBox, SIGNAL(currentIndexChanged(const QString& )),
          this, SLOT(imageSelectedSlot(const QString& )));

  connect(mLandmarkTableWidget, SIGNAL(cellChanged(int, int)),
          this, SLOT(cellChangedSlot(int, int)));

  connect(mToolManager, SIGNAL(dominantToolChanged(const std::string&)),
          this, SLOT(dominantToolChangedSlot(const std::string&)));

  connect(mToolSampleButton, SIGNAL(clicked(bool)),
          this, SLOT(toolSampleButtonClickedSlot(bool)));


  ssc::ToolPtr dominantTool = mToolManager->getDominantTool();
  if(dominantTool.get() != NULL)
    this->dominantToolChangedSlot(dominantTool->getUid());
}

PatientRegistrationDockWidget::~PatientRegistrationDockWidget()
{}

void PatientRegistrationDockWidget::imageSelectedSlot(const QString& comboBoxText)
{
  if(comboBoxText.isEmpty() || comboBoxText.endsWith("..."))
    return;

  std::string imageId = comboBoxText.toStdString();

  //find the image
  ssc::ImagePtr image = mDataManager->getImage(imageId);
  if(image.get() == NULL)
  {
    mMessageManager->sendError("Could not find the selected image in the DataManager: "+imageId);
    return;
  }

  //get the images landmarks and populate the landmark table
  this->populateTheLandmarkTableWidget(image);

  //TODO
  //link volumetricRep and inriaReps

  //view3D
  // Work in progress :-)
  /*
  View3D* view3D_1 = mViewManager->get3DView("View3D_1");
  VolumetricRepPtr volumetricRep = mRepManager->getVolumetricRep("VolumetricRep_1");
  LandmarkRepPtr landmarkRep = mRepManager->getLandmarkRep("LandmarkRep_1");
  volumetricRep->setImage(mCurrentImage);
  landmarkRep->setImage(mCurrentImage);
  view3D_1->setRep(volumetricRep);
  view3D_1->addRep(landmarkRep);

  //view2D
  View2D* view2D_1 = mViewManager->get2DView("View2D_1");
  View2D* view2D_2 = mViewManager->get2DView("View2D_2");
  View2D* view2D_3 = mViewManager->get2DView("View2D_3");
  InriaRep2DPtr inriaRep2D_1 = mRepManager->getInria2DRep("InriaRep2D_1");
  InriaRep2DPtr inriaRep2D_2 = mRepManager->getInria2DRep("InriaRep2D_2");
  InriaRep2DPtr inriaRep2D_3 = mRepManager->getInria2DRep("InriaRep2D_3");
  view2D_1->setRep(inriaRep2D_1);
  view2D_2->setRep(inriaRep2D_2);
  view2D_3->setRep(inriaRep2D_3);
  inriaRep2D_1->getVtkViewImage2D()->SetOrientation(vtkViewImage2D::AXIAL_ID);
  inriaRep2D_2->getVtkViewImage2D()->SetOrientation(vtkViewImage2D::CORONAL_ID);
  inriaRep2D_3->getVtkViewImage2D()->SetOrientation(vtkViewImage2D::SAGITTAL_ID);
  inriaRep2D_1->getVtkViewImage2D()->AddChild(inriaRep2D_2->getVtkViewImage2D());
  inriaRep2D_2->getVtkViewImage2D()->AddChild(inriaRep2D_3->getVtkViewImage2D());
  inriaRep2D_3->getVtkViewImage2D()->AddChild(inriaRep2D_1->getVtkViewImage2D());
  inriaRep2D_1->getVtkViewImage2D()->SyncRemoveAllDataSet();
  //TODO: ...or getBaseVtkImageData()???
  inriaRep2D_1->getVtkViewImage2D()->SyncAddDataSet(mCurrentImage->getRefVtkImageData());
  inriaRep2D_1->getVtkViewImage2D()->SyncReset();
  */
}

void PatientRegistrationDockWidget::visibilityOfDockWidgetChangedSlot(bool visible)
{
  if(visible)
  {
    connect(mDataManager, SIGNAL(dataLoaded()),
            this, SLOT(populateTheImageComboBox()));
    this->populateTheImageComboBox();
  }
  else
  {
    disconnect(mDataManager, SIGNAL(dataLoaded()),
            this, SLOT(populateTheImageComboBox()));
  }

}

void PatientRegistrationDockWidget::toolVisibleSlot(bool visible)
{
  if(visible)
    mToolSampleButton->setEnabled(true);
  else
    mToolSampleButton->setEnabled(false);
}

void PatientRegistrationDockWidget::toolSampleButtonClickedSlot(bool checked)
{
  ssc::Transform3DPtr lastTransform = mToolToSample->getLastTransform();
  vtkMatrix4x4Ptr lastTransformMatrix = lastTransform->matrix();
  double x = lastTransformMatrix->GetElement(0,3);
  double y = lastTransformMatrix->GetElement(1,3);
  double z = lastTransformMatrix->GetElement(2,3);

  unsigned int index = mCurrentRow+1;
  //emit addToolPosition(x, y, z, index);
}
void PatientRegistrationDockWidget::landmarkSelectedSlot(int row, int column)
{}
void PatientRegistrationDockWidget::populateTheImageComboBox()
{
  mImagesComboBox->clear();

  //get a list of images from the datamanager
  std::map<std::string, ssc::ImagePtr> images = mDataManager->getImages();
  if(images.size() == 0)
  {
    mImagesComboBox->insertItem(1, QString("First do Image Registration..."));
    mImagesComboBox->setEnabled(false);
    return;
  }

  //add these to the combobox
  typedef std::map<std::string, ssc::ImagePtr>::iterator iterator;
  int listPosition = 1;
  for(iterator i = images.begin(); i != images.end(); ++i)
  {
    mImagesComboBox->insertItem(listPosition, QString(i->first.c_str()));
    listPosition++;
  }
  //enable the add point button if any images was found
  ssc::ImagePtr masterImage = mRegistrationManager->getMasterImage();

  if(masterImage.get() == NULL)
    return;

  std::string uid = masterImage->getUid();
  int comboboxIndex = mImagesComboBox->findText(QString(uid.c_str()));
  if (comboboxIndex < 0)
    return;

  mImagesComboBox->setCurrentIndex(comboboxIndex);

}

void PatientRegistrationDockWidget::cellChangedSlot(int row, int column)
{
  if (column!=0)
    return;

  Qt::CheckState state = mLandmarkTableWidget->item(row,column)->checkState();
  mLandmarkActiveVector.push_back(state);

}

void PatientRegistrationDockWidget::dominantToolChangedSlot(const std::string& uid)
{
  if(mToolToSample.get() != NULL && mToolToSample->getUid() == uid)
    return;

  ToolPtr newTool = ToolPtr(dynamic_cast<Tool*>(mToolManager->getTool(uid).get()));
  if(mToolToSample.get() != NULL)
  {
    if(newTool.get() == NULL)
      return;

    disconnect(mToolToSample.get(), SIGNAL(toolVisible()),
                this, SLOT(toolVisibleSlot()));
  }

  mToolToSample = newTool;

  connect(mToolToSample.get(), SIGNAL(toolVisible()),
              this, SLOT(toolVisibleSlot()));
}

void PatientRegistrationDockWidget::populateTheLandmarkTableWidget(ssc::ImagePtr image)
{

  // TODO - Work in progress :-) //

  std::map<std::string, bool> nameList = mRegistrationManager->getGlobalPointSetNameList();
  int numberOfNames = nameList.size();

  vtkDoubleArrayPtr landmarks =  image->getLandmarks();
  int numberOfLandmarks = landmarks->GetNumberOfTuples();

  mLandmarkActiveVector.clear();

  mLandmarkTableWidget->clear();
  mLandmarkTableWidget->setRowCount((numberOfLandmarks > numberOfNames ? numberOfLandmarks : numberOfNames));
  mLandmarkTableWidget->setColumnCount(3);
  QStringList headerItems(QStringList() << "Active" << "Name" << "Accuracy");
  mLandmarkTableWidget->setHorizontalHeaderLabels(headerItems);
  mLandmarkTableWidget->horizontalHeader()->
    setResizeMode(QHeaderView::ResizeToContents);

  typedef std::map<std::string, bool>::iterator Iterator;
  int row = 1;
  for(Iterator it = nameList.begin(); it != nameList.end(); ++it)
  {
    std::string name = it->first;
    QTableWidgetItem* columnOne = new QTableWidgetItem();
    columnOne->setCheckState(Qt::Checked);
    QTableWidgetItem* columnTwo = new QTableWidgetItem(tr(name.c_str()));
    QTableWidgetItem* columnThree;
    if(row <= numberOfLandmarks)
    {
      double* point = landmarks->GetTuple(row-1);
      columnTwo = new QTableWidgetItem(tr("(%1, %2, %3)").arg(point[0]).arg(point[1]).arg(point[2]));
    }
    else
    {
      columnTwo = new QTableWidgetItem(tr(" "));
    }
    columnThree->setFlags(Qt::ItemIsSelectable);

    mLandmarkTableWidget->setItem(row-1, 0, columnOne);
    mLandmarkTableWidget->setItem(row-1, 1, columnTwo);
    mLandmarkTableWidget->setItem(row-1, 2, columnThree);
    mLandmarkActiveVector.push_back(true);
    row++;
  }
  for(; row<=numberOfLandmarks; row++)
  {
    double* point = landmarks->GetTuple(row-1);

    QTableWidgetItem* columnOne = new QTableWidgetItem();
    columnOne->setCheckState(Qt::Checked);
    QTableWidgetItem* columnTwo = new QTableWidgetItem(tr("(%1, %2, %3)").arg(point[0]).arg(point[1]).arg(point[2]));
    QTableWidgetItem* columnThree = new QTableWidgetItem(tr(" "));
    columnThree->setFlags(Qt::ItemIsSelectable);

    mLandmarkTableWidget->setItem(row-1, 0, columnOne);
    mLandmarkTableWidget->setItem(row-1, 1, columnTwo);
    mLandmarkTableWidget->setItem(row-1, 2, columnThree);
    mLandmarkActiveVector.push_back(true);
  }

}


}//namespace cx
