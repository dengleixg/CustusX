#ifndef CXLANDMARKIMAGEREGISTRATIONWIDGET_H_
#define CXLANDMARKIMAGEREGISTRATIONWIDGET_H_

#include "cxLandmarkRegistrationWidget.h"
#include "cxRegistrationDataAdapters.h"
#include "Rep/cxLandmarkRep.h"
#include "cxDominantToolProxy.h"

class QVBoxLayout;
class QComboBox;
class QTableWidget;
class QPushButton;
class QLabel;
class QSlider;

namespace cx
{
/**
 * \file
 * \addtogroup cxPluginRegistration
 * @{
 */


/**
 * \class LandmarkImageRegistrationWidget
 *
 * \brief Widget for performing landmark based image registration
 *
 * \date Jan 27, 2009
 * \\author Janne Beate Bakeng, SINTEF
 */
class LandmarkImageRegistrationWidget : public LandmarkRegistrationWidget
{
  Q_OBJECT

public:

  LandmarkImageRegistrationWidget(RegistrationManagerPtr regManager, QWidget* parent, QString objectName, QString windowTitle); ///< sets up layout and connects signals and slots
  virtual ~LandmarkImageRegistrationWidget(); ///< empty
  virtual QString defaultWhatsThis() const;

signals:
  void thresholdChanged(const int value); ///< the value selected by the slider

protected slots:
  virtual void activeImageChangedSlot(); ///< listens to the datamanager for when the active image is changed
  void addLandmarkButtonClickedSlot(); ///< reacts when the Add Landmark button is clicked
  virtual void editLandmarkButtonClickedSlot(); ///< reacts when the Edit Landmark button is clicked
  void removeLandmarkButtonClickedSlot(); ///< reacts when the Remove Landmark button is clicked
  virtual void cellClickedSlot(int row, int column); ///< when a landmark i selected from the table
  void thresholdChangedSlot(const int value); ///< reemits the valueChanged signal from the slider
  void enableButtons();

//  void registerSlot();
//  void autoRegisterSlot(bool checked);

protected:
  virtual void showEvent(QShowEvent* event); ///<updates internal info before showing the widget
  virtual void hideEvent(QHideEvent* event);
  virtual ssc::LandmarkMap getTargetLandmarks() const;
  virtual void performRegistration() {}
  virtual void populateTheLandmarkTableWidget(); ///< populates the table widget
  QString getLandmarkName(QString uid);
  virtual ssc::Transform3D getTargetTransform() const;
//  void internalPerformRegistration(bool doIt);
  ssc::PickerRepPtr getPickerRep();

  //gui
//  RegistrationFixedImageStringDataAdapterPtr mFixedDataAdapter;
  ssc::StringDataAdapterPtr mActiveImageAdapter;
  ImageLandmarksSourcePtr mImageLandmarkSource;
  DominantToolProxyPtr mDominantToolProxy;

  QPushButton* mAddLandmarkButton; ///< the Add Landmark button
  QPushButton* mEditLandmarkButton; ///< the Edit Landmark button
  QPushButton* mRemoveLandmarkButton; ///< the Remove Landmark button
  QLabel*      mThresholdLabel; ///< label for the tresholdslider
  QSlider*     mThresholdSlider; ///< slider for setting the picking treshold

//  QPushButton* mRegisterButton;
//  QCheckBox* mAutoRegisterCheckBox;
  ;

private:
  LandmarkImageRegistrationWidget(); ///< not implemented
};

/**
 * @}
 */
}//namespace cx

#endif /* CXLANDMARKIMAGEREGISTRATIONWIDGET_H_ */
