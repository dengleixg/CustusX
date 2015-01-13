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

#include "cxNavigationWidget.h"

#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QToolBar>

#include "cxTrackingService.h"
#include "cxDoubleWidgets.h"
#include "cxDataInterface.h"
#include "cxPropertyHelper.h"
#include "cxViewService.h"

//TODO: remove
#include "cxLegacySingletons.h"

namespace cx
{
NavigationWidget::NavigationWidget(QWidget* parent) :
    BaseWidget(parent, "NavigationWidget", "Navigation Properties"),
    mVerticalLayout(new QVBoxLayout(this)),
    mCameraGroupBox(new QGroupBox(tr("Camera Style"), this)),
    mCameraGroupLayout(new QVBoxLayout())
{

  //camera setttings
  mCameraGroupBox->setLayout(mCameraGroupLayout);

  QToolBar* toolBar = new QToolBar(this);
  mCameraGroupLayout->addWidget(toolBar);
  toolBar->addActions(viewService()->createInteractorStyleActionGroup()->actions());

  QWidget* toolOffsetWidget = new SliderGroupWidget(this, DoublePropertyActiveToolOffset::create());

  //layout
  this->setLayout(mVerticalLayout);
  mVerticalLayout->addWidget(mCameraGroupBox);
  mVerticalLayout->addWidget(toolOffsetWidget);
  mVerticalLayout->addStretch();
}

NavigationWidget::~NavigationWidget()
{}

QString NavigationWidget::defaultWhatsThis() const
{
  return "<html>"
      "<h3>Camera navigation.</h3>"
      "<p>Lets you select if the camera should follow a tool.</p>"
      "<p><i>Click the options to select camera following style.</i></p>"
      "</html>";
}

}
