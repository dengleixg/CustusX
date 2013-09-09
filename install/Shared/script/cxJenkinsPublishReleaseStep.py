#!/usr/bin/env python

#####################################################
# Unix jenkins script
# Author: Christian Askeland, SINTEF Medical Technology
# Date:   2013.09.08
#
# Description:
#
#
#####################################################

import logging
import time    
import subprocess
import sys
import argparse        

from cx.cxShell import *
from cx.cxPrintFormatter import PrintFormatter
import cx.cxInstallData
import cx.cxComponents
import cx.cxComponentAssembly
import cx.cxCustusXBuilder
import cx.cxJenkinsBuildScriptBase
import cx.cxCustusXInstaller
import cx.cxCustusXTestInstallation


class Controller(cx.cxJenkinsBuildScriptBase.JenkinsBuildScriptBaseBase):
    '''
    '''
    def __init__(self):
        ''
        self.cxInstaller = cx.cxCustusXInstaller.CustusXInstaller()
        super(Controller, self).__init__()

    def getDescription(self):                  
        return 'Jenkins script for creating a release folder and publishing it.'
           
    def _addArgumentParserArguments(self):
        'subclasses can add parser arguments here'
        super(Controller, self)._addArgumentParserArguments()
        p = self.argumentParser
        p.add_argument('--skip_publish_release', action='store_true', default=False, help='Skip the publish release to server step')

    def _applyArgumentParserArguments(self, options):
        'apply arguments defined in _addArgumentParserArguments()'
        super(Controller, self)._applyArgumentParserArguments(options)
        data = self.cxBuilder.assembly.controlData        
        data.setBuildType("Release")
  
        self._initializeInstallationObject()
 
    def _initializeInstallationObject(self):
        '''
        Initialize CustusXInstallation object with data 
        from the build process.
        '''
        assembly = self.cxBuilder.assembly                
        custusxdata = assembly.getComponent(cx.cxComponents.CustusX3Data)
        custusx = assembly.getComponent(cx.cxComponents.CustusX3)
        
        self.cxInstaller.setRootDir(assembly.controlData.getRootDir())
        self.cxInstaller.setInstallerPath(custusx.buildPath())        
        self.cxInstaller.setSourcePath(custusx.sourcePath())        

    def run(self):
        folder = self.cxInstaller.createReleaseFolder()
        if not self.argumentParserArguments.skip_publish_release:
            self.cxInstaller.publishReleaseFolder(folder)
        self.cxBuilder.finish()
    
if __name__ == '__main__':
    controller = Controller()
    controller.run()
