/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#ifndef CXSTLMESHREADER_H
#define CXSTLMESHREADER_H

#include "cxFileReaderWriterService.h"
#include "org_custusx_core_filemanager_Export.h"

class ctkPluginContext;
namespace cx
{
/**\brief Reader for STL files.
 *
 */
class org_custusx_core_filemanager_EXPORT StlMeshReader: public FileReaderWriterImplService
{
public:
	Q_INTERFACES(cx::FileReaderWriterService)

	StlMeshReader(PatientModelServicePtr patientModelService);
	virtual ~StlMeshReader(){}

	bool isNull(){return false;}

	virtual bool canRead(const QString& type, const QString& filename);
	virtual bool readInto(DataPtr data, QString path);
	bool readInto(MeshPtr mesh, QString filename);
	virtual QString canReadDataType() const { return "mesh"; }
	virtual DataPtr read(const QString& uid, const QString& filename);
	std::vector<DataPtr> read(const QString &filename);

	void write(DataPtr data, const QString &filename);
	QString canWriteDataType() const;
	bool canWrite(const QString &type, const QString &filename) const;

	virtual vtkPolyDataPtr loadVtkPolyData(QString filename);
};

}

#endif // CXSTLMESHREADER_H
