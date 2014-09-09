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

#ifndef CXREGISTRATIONWIDGET_H_
#define CXREGISTRATIONWIDGET_H_

#include <QWidget>
#include "cxServiceTrackerListener.h"
#include "cxRegistrationMethodService.h"
class QVBoxLayout;
class QTabWidget;

namespace cx
{

/**
 * Widget for use in the Registration GUI plugin
 *
 * \ingroup org_custusx_registration_gui
 *
 * \date Sep 08 2014
 * \author Ole Vegard Solberg, SINTEF
 */
class RegistrationWidget : public QWidget
{
	Q_OBJECT
public:
	RegistrationWidget(ctkPluginContext *context, QWidget* parent = 0);
	virtual ~RegistrationWidget();

private:
	QString defaultWhatsThis() const;

	void onServiceAdded(RegistrationMethodService *service);
	void onServiceRemoved(RegistrationMethodService *service);
	QTabWidget *findTypeWidget(QString registrationType);
	QTabWidget *createTypeWidget(QString registrationType);

	ctkPluginContext* mPluginContext;
	QVBoxLayout*  mVerticalLayout;
	boost::shared_ptr<ServiceTrackerListener<RegistrationMethodService> > mServiceListener;

	std::map<QString, QTabWidget*> mCategoryWidgetsMap; ///< map containing groups
	std::map<QString, QWidget*> mCategorizedWidgets;
};

} /* namespace cx */

#endif /* CXREGISTRATIONWIDGET_H_ */