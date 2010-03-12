#ifndef CXVIEW2D_H_
#define CXVIEW2D_H_

#include "sscView.h"

class QWidget;
class QMenu;

namespace cx
{
class DataManager;
class RepManager;
class MessageManager;
class ViewManager;

/**
 * \class View2D
 *
 * \brief Class for displaying 2D representations.
 *
 * \date Dec 9, 2008
 * \author: Janne Beate Bakeng, SINTEF
 */
class View2D : public ssc::View
{
  Q_OBJECT
public:
  View2D(const std::string& uid, const std::string& name="", QWidget *parent = NULL, Qt::WFlags f = 0); ///< constructor
  virtual ~View2D(); ///< empty

  virtual Type getType() const { return VIEW_2D;}; ///< get the class type
  void contextMenuEvent(QContextMenuEvent *event); ///< decides what happens when you rightclick in a view

protected:
  QMenu* mContextMenu; ///< right click menu

  DataManager* mDataManager; ///< has all the data loaded into the system
  RepManager* mRepManager; ///< has a pool of reps

  //INFO:
  //Cannot have the viewmanager here because that creates a never-ending
  //dependency cycle between view2d and viewmanager, use
  //ViewManager::getInstance in the code instead
  //ViewManager* mViewManager; ///< has a pool of views and handles layout


};
}//namespace cx
#endif /* CXVIEW2D2_H_ */
