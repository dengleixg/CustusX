#ifndef CXREPMANAGER_H_
#define CXREPMANAGER_H_

#include <QObject>
#include <map>
#include "sscTransform3D.h"
#include "sscToolRep3D.h"
#include "sscVolumetricRep.h"
#include "sscProbeRep.h"
#include "sscGeometricRep.h"
#include "sscProgressiveLODVolumetricRep.h"
#include "cxTool.h"
#include "cxInriaRep3D.h"
#include "cxInriaRep2D.h"
#include "cxLandmarkRep.h"

namespace cx
{
typedef ssc::ProbeRep ProbeRep;
typedef ssc::Transform3D Transform3D;
typedef ssc::ProbeRepPtr ProbeRepPtr;
typedef ssc::ProgressiveLODVolumetricRep ProgressiveVolumetricRep;
typedef ssc::ProgressiveLODVolumetricRepPtr ProgressiveVolumetricRepPtr;

typedef std::map<std::string, ssc::RepPtr> RepMap;
typedef std::map<std::string, InriaRep2DPtr> InriaRep2DMap;
typedef std::map<std::string, InriaRep3DPtr> InriaRep3DMap;
typedef std::map<std::string, ssc::VolumetricRepPtr> VolumetricRepMap;
typedef std::map<std::string, ProbeRepPtr> ProbeRepMap;
typedef std::map<std::string, ProgressiveVolumetricRepPtr> ProgressiveVolumetricRepMap;
typedef std::map<std::string, LandmarkRepPtr> LandmarkRepMap;
typedef std::map<std::string, ssc::ToolRep3DPtr> ToolRep3DMap;
typedef std::map<std::string, ssc::GeometricRepPtr> GeometricRepMap;

class MessageManager;

/**
 * \class RepManager
 *
 * \brief Creates a pool of reps (representations) and offers an interface to
 * access them.
 *
 * \warning ProgressiveLODVolumetricRep does not work!
 *
 * \date Dec 10, 2008
 * \author: Janne Beate Bakeng, SINTEF
 */
class RepManager : public QObject
{
  Q_OBJECT

public:
  static RepManager* getInstance(); ///< get the only instance of this class
  static void destroyInstance(); ///< destroy the only instance of this class

  std::vector<std::pair<std::string, std::string> > getRepUidsAndNames(); ///< get unique id and name of all reps in the pool

  RepMap* getReps(); ///< get all reps in the pool
  InriaRep3DMap* getInria3DReps(); ///< get all Inria3D reps in the pool
  InriaRep2DMap* getInria2DReps(); ///< get all Inria2D reps in the pool
  VolumetricRepMap* getVolumetricReps(); ///< get all Volumetric reps in the pool
  ProbeRepMap* getProbeReps(); ///< get all Probe reps in the pool
  ProgressiveVolumetricRepMap* getProgressiveVolumetricReps(); ///< get all ProgressiveLODVolumetric reps in the pool
  LandmarkRepMap* getLandmarkReps(); ///< get all Landmark reps in the pool
  ToolRep3DMap* getToolRep3DReps(); ///< get all Tool3D reps in the pool
  GeometricRepMap* getGeometricReps(); ///< get all Geometric reps in the pool

  ssc::RepPtr getRep(const std::string& uid); ///< get one specific rep
  InriaRep3DPtr getInria3DRep(const std::string& uid); ///< get one specific Inria3D rep
  InriaRep2DPtr getInria2DRep(const std::string& uid); ///< get one specific Inria2D rep
  ssc::VolumetricRepPtr getVolumetricRep(const std::string& uid); ///< get one specific Volumetric rep
  ProbeRepPtr getProbeRep(const std::string& uid); ///< get one specific Probe rep
  ProgressiveVolumetricRepPtr getProgressiveVolumetricRep(const std::string& uid); ///< get one specific ProgressiveLODVolumetric rep
  LandmarkRepPtr getLandmarkRep(const std::string& uid); ///<  get one specific Landmark rep
  ssc::ToolRep3DPtr getToolRep3DRep(const std::string& uid); ///<  get one specific Tool3D rep
  ssc::GeometricRepPtr getGeometricRep(const std::string& uid); ///<  get one specific Geometric rep

protected slots:
  void syncInria2DRepsSlot(double x,double y,double z); ///< updates the inria2dreps with a new position on mouseclick and tooltransform
  void dominantToolChangedSlot(const std::string& toolUid); ///< makes sure the inriareps are connected to the right tool
  void receiveToolTransfromAndTimeStampSlot(Transform3D prMt, double timestamp); ///< listens for new transforms from the tool

protected:
  static RepManager*  mTheInstance;         ///< the only instance of this class

  const int           MAX_INRIAREP3DS; ///< number of Inria3D reps in the pool
  std::string         mInriaRep3DNames[2]; ///< the name of the reps in the pool
  InriaRep3DMap       mInriaRep3DMap;     ///< the reps in the pool

  const int           MAX_INRIAREP2DS; ///< number of Inria2D reps in the pool
  std::string         mInriaRep2DNames[9]; ///< the name of the reps in the pool
  InriaRep2DMap       mInriaRep2DMap;     ///< the reps in the pool

  const int           MAX_VOLUMETRICREPS; ///< number of Volumetric reps in the pool
  std::string         mVolumetricRepNames[2]; ///< the name of the reps in the pool
  VolumetricRepMap    mVolumetricRepMap;  ///< the reps in the pool

  const int           MAX_PROGRESSIVEVOLUMETRICREPS; ///< number of ProgressiveLODVolumetric reps in the pool
  std::string         mProgressiveVolumetricRepNames[2]; ///< the name of the reps in the pool
  ProgressiveVolumetricRepMap    mProgressiveVolumetricRepMap;  ///< the reps in the pool

  const int           MAX_PROBEREPS; ///< number of Probe reps in the pool
  std::string         mProbeRepNames[2]; ///< the name of the reps in the pool
  ProbeRepMap         mProbeRepMap;  ///< the reps in the pool

  const int           MAX_LANDMARKREPS; ///< number of Landmark reps in the pool
  std::string         mLandmarkRepNames[2]; ///< the name of the reps in the pool
  LandmarkRepMap      mLandmarkRepMap;    ///< the reps in the pool

  const int           MAX_TOOLREP3DS; ///< number of Tool3D reps in the pool
  std::string         mToolRep3DNames[5]; ///< the name of the reps in the pool
  ToolRep3DMap        mToolRep3DMap;      ///< the reps in the pool

  const int           MAX_GEOMETRICREPS; ///< number of Geometric reps in the pool
  std::string         mGeometricRepNames[6]; ///< the name of the reps in the pool
  GeometricRepMap     mGeometricRepMap;      ///< the reps in the pool

  ssc::ToolPtr mConnectedTool; ///< the tool we are listening to transforms from (to update inria2drep points)

private:
  RepManager(); ///< creates a pool of reps
  ~RepManager(); ///< empty
  RepManager(RepManager const&); ///< not implemented
  RepManager& operator=(RepManager const&); ///< not implemented
};
}//namespace cx
#endif /* CXREPMANAGER_H_ */
