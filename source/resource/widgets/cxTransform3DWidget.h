/*
 * cxAffineMatrixWidget.h
 *
 *  \date Mar 19, 2011
 *      \author christiana
 */

#ifndef CXTRANSFORM3DWIDGET_H_
#define CXTRANSFORM3DWIDGET_H_

#include "cxBaseWidget.h"
#include "cxDoubleDataAdapterXml.h"
#include "cxTransform3D.h"
#include "cxFrame3D.h"

class QTextEdit;
class QVBoxLayout;
class QAction;
class QFrame;

namespace cx
{

class MatrixTextEdit;

/**
 * \brief Widget for displaying and manipulating an affine matrix,
 * i.e. a rotation+translation matrix.
 *
 * \ingroup cx_resource_widgets
 *
 */
class Transform3DWidget : public BaseWidget
{
  Q_OBJECT
public:
  Transform3DWidget(QWidget* parent = NULL);
  virtual ~Transform3DWidget();
  virtual QString defaultWhatsThis() const;

  void setMatrix(const Transform3D& M);
  Transform3D getMatrix() const;
  void setEditable(bool edit);

signals:
  void changed();

private slots:
  void changedSlot();
  void toggleEditSlot();
  void textEditChangedSlot();
  void toggleInvertSlot();

private:
  void setMatrixInternal(const Transform3D& M);
  Transform3D getMatrixInternal() const;

  void updateValues();
  void addAngleControls(QString uid, QString name, int index, QVBoxLayout* layout);
  void addTranslationControls(QString uid, QString name, int index, QVBoxLayout* layout);
  void rotateSlot(QPointF delta, int index);
  void translateSlot(QPointF delta, int index);
  void updateAdapter(DoubleDataAdapterXmlPtr adapter, double value);
  void setActionText(QAction* action, QString text, QString tip);
  Transform3D convertToFromExternal(const Transform3D& M) const;
  void updateInvertAction();

//  Frame3D mFrame;
  MatrixTextEdit* mTextEdit;
  boost::array<DoubleDataAdapterXmlPtr, 3> mAngleAdapter;
  boost::array<DoubleDataAdapterXmlPtr, 3> mTranslationAdapter;
  bool recursive;
  bool mBlockChanges;
  QAction* mEditAction;
  QAction* mInvertAction;
  QFrame* aGroupBox;
  QFrame* tGroupBox;
  DecomposedTransform3D mDecomposition;

//  //  Vector3DWidget(QWidget* parent, Vector3DDataAdapterPtr data)
//    Vector3DDataAdapterPtr mAngleAdapter;
//    Vector3DDataAdapterPtr mTranslationAdapter;

//  template<class T>
//  QAction* createAction(QLayout* layout, QString iconName, QString text, QString tip, T slot);
};

}

#endif /* CXTRANSFORM3DWIDGET_H_ */
