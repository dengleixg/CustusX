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

#ifndef CXFASTORIENTATIONREGISTRATIONWIDGET_H_
#define CXFASTORIENTATIONREGISTRATIONWIDGET_H_

#include "cxRegistrationBaseWidget.h"
#include "cxForwardDeclarations.h"
#include "cxTransform3D.h"
#include "cxActiveToolProxy.h"

class QPushButton;
class QCheckBox;

namespace cx
{
/**
 * \file
 * \addtogroup cx_module_registration
 * @{
 */

/**
 * \class FastOrientationRegistrationWidget
 *
 * \brief Widget for registrating the orientation part of a fast registration
 *
 * \date 27. sep. 2010
 * \\author Janne Beate Bakeng
 */

class FastOrientationRegistrationWidget : public RegistrationBaseWidget
{
  Q_OBJECT

public:
	FastOrientationRegistrationWidget(RegServices services, QWidget* parent);
  ~FastOrientationRegistrationWidget();
  virtual QString defaultWhatsThis() const;

protected:
  virtual void showEvent(QShowEvent* event);
  virtual void hideEvent(QHideEvent* event);

private slots:
  void setOrientationSlot();
  void enableToolSampleButtonSlot();
private:
  Transform3D get_tMtm() const;
  QPushButton* mSetOrientationButton;
  QCheckBox* mInvertButton;
  ActiveToolProxyPtr mActiveToolProxy;
};

/**
 * @}
 */
}
#endif /* CXFASTORIENTATIONREGISTRATIONWIDGET_H_ */
