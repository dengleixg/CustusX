/*
 * cxRegistrationMethodsWidget.h
 *
 *  Created on: May 2, 2011
 *      Author: christiana
 */

#ifndef CXREGISTRATIONMETHODSWIDGET_H_
#define CXREGISTRATIONMETHODSWIDGET_H_

#include "cxTabbedWidget.h"

namespace cx
{

class LandmarkRegistrationsWidget : public TabbedWidget
{
public:
  LandmarkRegistrationsWidget(QWidget* parent, QString objectName, QString windowTitle);
  virtual ~LandmarkRegistrationsWidget(){};
  virtual QString defaultWhatsThis() const;
};
//------------------------------------------------------------------------------
class FastRegistrationsWidget : public TabbedWidget
{
public:
  FastRegistrationsWidget(QWidget* parent, QString objectName, QString windowTitle);
  virtual ~FastRegistrationsWidget(){};
  virtual QString defaultWhatsThis() const;
};
//------------------------------------------------------------------------------
class Image2ImageRegistrationWidget : public TabbedWidget
{
public:
  Image2ImageRegistrationWidget(QWidget* parent, QString objectName, QString windowTitle);
  virtual ~Image2ImageRegistrationWidget(){};
  virtual QString defaultWhatsThis() const;
};
//------------------------------------------------------------------------------
class Image2PlateRegistrationWidget : public TabbedWidget
{
public:
  Image2PlateRegistrationWidget(QWidget* parent, QString objectName, QString windowTitle);
  virtual ~Image2PlateRegistrationWidget(){};
  virtual QString defaultWhatsThis() const;
};
//------------------------------------------------------------------------------
class RegistrationMethodsWidget : public TabbedWidget
{
public:
  RegistrationMethodsWidget(QWidget* parent, QString objectName, QString windowTitle);
  virtual ~RegistrationMethodsWidget(){};
  virtual QString defaultWhatsThis() const;
};
//------------------------------------------------------------------------------

}

#endif /* CXREGISTRATIONMETHODSWIDGET_H_ */
