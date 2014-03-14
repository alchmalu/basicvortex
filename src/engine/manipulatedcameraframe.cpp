/****************************************************************************

 Copyright (C) 2002-2008 Gilles Debunne. All rights reserved.

 This file is part of the QGLViewer library version 2.3.9.

 http://www.libqglviewer.com - contact@libqglviewer.com

 This file may be used under the terms of the GNU General Public License
 versions 2.0 or 3.0 as published by the Free Software Foundation and
 appearing in the LICENSE file included in the packaging of this file.
 In addition, as a special exception, Gilles Debunne gives you certain
 additional rights, described in the file GPL_EXCEPTION in this package.

 libQGLViewer uses dual licensing. Commercial/proprietary software must
 purchase a libQGLViewer Commercial License.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/

#include "manipulatedcameraframe.h"
#include "camera.h"


namespace vortex {
using namespace util;
/*! Default constructor.

 flySpeed() is set to 0.0 and flyUpVector() is (0,1,0). The revolveAroundPoint() is set to (0,0,0).

  \attention Created object is removeFromMouseGrabberPool(). */
ManipulatedCameraFrame::ManipulatedCameraFrame()
  : driveSpeed_(0.0), flyUpVector_(0.0, 1.0, 0.0)
{
  setFlySpeed(0.0);
//  removeFromMouseGrabberPool();

//  connect(&flyTimer_, SIGNAL(timeout()), SLOT(flyUpdate()));
}

/*! Equal operator. Calls ManipulatedFrame::operator=() and then copy attributes. */
ManipulatedCameraFrame& ManipulatedCameraFrame::operator=(const ManipulatedCameraFrame& mcf)
{
  ManipulatedFrame::operator=(mcf);

  setFlySpeed(mcf.flySpeed());
  setFlyUpVector(mcf.flyUpVector());

  return *this;
}

/*! Copy constructor. Performs a deep copy of all members using operator=(). */
ManipulatedCameraFrame::ManipulatedCameraFrame(const ManipulatedCameraFrame& mcf)
  : ManipulatedFrame(mcf)
{
//  removeFromMouseGrabberPool();
//  connect(&flyTimer_, SIGNAL(timeout()), SLOT(flyUpdate()));
  (*this)=(mcf);
}

////////////////////////////////////////////////////////////////////////////////

/*! Overloading of ManipulatedFrame::spin().

Rotates the ManipulatedCameraFrame around its revolveAroundPoint() instead of its origin. */
void ManipulatedCameraFrame::spin()
{
  rotateAroundPoint(spinningQuaternion(), revolveAroundPoint());
}


/*! Called for continuous frame motion in fly mode (see QGLViewer::MOVE_FORWARD). Emits
  manipulated(). */
void ManipulatedCameraFrame::flyUpdate()
{
    /*
  static Vec flyDisp(0.0, 0.0, 0.0);
  switch (action_)
    {
    case QGLViewer::MOVE_FORWARD:
      flyDisp.z = -flySpeed();
      translate(localInverseTransformOf(flyDisp));
      break;
    case QGLViewer::MOVE_BACKWARD:
      flyDisp.z = flySpeed();
      translate(localInverseTransformOf(flyDisp));
      break;
    case QGLViewer::DRIVE:
      flyDisp.z = flySpeed() * driveSpeed_;
      translate(localInverseTransformOf(flyDisp));
      break;
    default:
      break;
    }
*/
  // Needs to be out of the switch since ZOOM/fastDraw()/wheelEvent use this callback to trigger a final draw().
  // #CONNECTION# wheelEvent.
  //Q_EMIT manipulated();
}


/*! This method will be called by the Camera when its orientation is changed, so that the
flyUpVector (private) is changed accordingly. You should not need to call this method. */
void ManipulatedCameraFrame::updateFlyUpVector()
{
  flyUpVector_ = inverseTransformOf(glm::vec3(0.0, 1.0, 0.0));
}

////////////////////////////////////////////////////////////////////////////////
//                 M o u s e    h a n d l i n g                               //
////////////////////////////////////////////////////////////////////////////////

/*! Protected internal method used to handle mouse events. */
void ManipulatedCameraFrame::startAction(int ma, bool withConstraint)
{
  ManipulatedFrame::startAction(ma, withConstraint);

  switch (action_)
    {
    case VortexEngine::MOVE_FORWARD:
    case VortexEngine::MOVE_BACKWARD:
    case VortexEngine::DRIVE:
      //flyTimer_.start(10);
      break;
    default:
      break;
    }
}


// TODO CODE THIS FIRST

/*! Overloading of ManipulatedFrame::mouseMoveEvent().

Motion depends on mouse binding (see <a href="../mouse.html">mouse page</a> for details). The
resulting displacements are basically inverted from those of a ManipulatedFrame. */
bool ManipulatedCameraFrame::mouseMoveEvent(const VortexEngine::MouseEvent* const event, Camera* const camera)
{
    //std::cerr << "Mouse at " << event->x << " x " << event->y << std::endl;
    //std::cerr << "ManipulatedCameraFrame::mouseMoveEvent" << std::endl;
  // #CONNECTION# QGLViewer::mouseMoveEvent does the updateGL.
  switch (action_)
    {
    case VortexEngine::TRANSLATE:
      {
        const glm::ivec2 delta = prevPos_ - glm::ivec2(event->x, event->y);
        glm::vec3 trans(delta.x, -delta.y, 0.0);
        // Scale to fit the screen mouse displacement
        switch (camera->type())
          {
          case Camera::PERSPECTIVE :
            trans *= 2.0 * tan(camera->fieldOfView()/2.0) *
              fabs((camera->frame()->coordinatesOf(revolveAroundPoint())).z) / camera->screenHeight();
            break;
          case Camera::ORTHOGRAPHIC :
            {
              GLdouble w,h;
              camera->getOrthoWidthHeight(w, h);
              trans[0] *= 2.0 * w / camera->screenWidth();
              trans[1] *= 2.0 * h / camera->screenHeight();
              break;
            }
          }
        translate(inverseTransformOf(translationSensitivity()*trans));
        break;
      }

    case VortexEngine::MOVE_FORWARD:
      {
        Quaternion rot = pitchYawQuaternion(event->x, event->y, camera);
        rotate(rot);
        //#CONNECTION# wheelEvent MOVE_FORWARD case
        // actual translation is made in flyUpdate().
        //translate(inverseTransformOf(Vec(0.0, 0.0, -flySpeed())));
        break;
      }

    case VortexEngine::MOVE_BACKWARD:
      {
        Quaternion rot = pitchYawQuaternion(event->x, event->y, camera);
        rotate(rot);
        // actual translation is made in flyUpdate().
        //translate(inverseTransformOf(Vec(0.0, 0.0, flySpeed())));
        break;
      }

    case VortexEngine::DRIVE:
      {
        Quaternion rot = turnQuaternion(event->x, camera);
        rotate(rot);
        // actual translation is made in flyUpdate().
        driveSpeed_ = 0.01 * (event->y - pressPos_.y);
        break;
      }

    case VortexEngine::ZOOM:
      {
        //#CONNECTION# wheelEvent() ZOOM case
        const float coef = glm::max(fabsf((camera->frame()->coordinatesOf(camera->revolveAroundPoint())).z), 0.2f*camera->sceneRadius());
        glm::vec3 trans(0.0, 0.0, -coef * (event->y - prevPos_.y) / camera->screenHeight());
        translate(inverseTransformOf(trans));
        break;
      }

    case VortexEngine::LOOK_AROUND:
      {
        Quaternion rot = pitchYawQuaternion(event->x, event->y, camera);
        rotate(rot);
        break;
      }

    case VortexEngine::ROTATE:
      {
        glm::vec3 trans = camera->projectedCoordinatesOf(revolveAroundPoint());
        Quaternion rot = deformedBallQuaternion(event->x, event->y, trans[0], trans[1], camera);
        //#CONNECTION# These two methods should go together (spinning detection and activation)
        computeMouseSpeed(event);
        setSpinningQuaternion(rot);
        spin();
        break;
      }

    case VortexEngine::SCREEN_ROTATE:
      {
        glm::vec3 trans = camera->projectedCoordinatesOf(revolveAroundPoint());

        const float angle = atan2(event->y - trans[1], event->x - trans[0]) - atan2(prevPos_.y-trans[1], prevPos_.x-trans[0]);

        Quaternion rot(glm::vec3(0.0, 0.0, 1.0), angle);
        //#CONNECTION# These two methods should go together (spinning detection and activation)
        computeMouseSpeed(event);
        setSpinningQuaternion(rot);
        spin();
        updateFlyUpVector();
        break;
      }

    case VortexEngine::ROLL:
      {
        const float angle = M_PI * (event->x - prevPos_.x) / camera->screenWidth();
        Quaternion rot(glm::vec3(0.0, 0.0, 1.0), angle);
        rotate(rot);
        setSpinningQuaternion(rot);
        updateFlyUpVector();
        break;
      }

    case VortexEngine::SCREEN_TRANSLATE:
      {
        glm::vec3 trans;
        int dir = mouseOriginalDirection(event);
        if (dir == 1)
          trans = glm::vec3(prevPos_.x - event->x, 0.0, 0.0);
        else if (dir == -1)
          trans=glm::vec3(0.0, event->y - prevPos_.y, 0.0);

        switch (camera->type())
          {
          case Camera::PERSPECTIVE :
            trans *= 2.0 * tan(camera->fieldOfView()/2.0) *
              fabs((camera->frame()->coordinatesOf(revolveAroundPoint())).z) / camera->screenHeight();
            break;
          case Camera::ORTHOGRAPHIC :
            {
              GLdouble w,h;
              camera->getOrthoWidthHeight(w, h);
              trans[0] *= 2.0 * w / camera->screenWidth();
              trans[1] *= 2.0 * h / camera->screenHeight();
              break;
            }
          }

        translate(inverseTransformOf(translationSensitivity()*trans));
        break;
      }

    case VortexEngine::ZOOM_ON_REGION:
    case VortexEngine::NO_MOUSE_ACTION:
      break;
    }

  if (action_ != VortexEngine::NO_MOUSE_ACTION)
    {
      prevPos_ = glm::ivec2(event->x, event->y);
      if (action_ != VortexEngine::ZOOM_ON_REGION)
        // ZOOM_ON_REGION should not emit manipulated().
        // prevPos_ is used to draw rectangle feedback.
        //Q_EMIT manipulated();
        return true;
    }
  return false;
}


/*! This is an overload of ManipulatedFrame::mouseReleaseEvent(). The QGLViewer::MouseAction is
  terminated. */
void ManipulatedCameraFrame::mouseReleaseEvent(const VortexEngine::MouseEvent* const event, Camera* const camera)
{
 /* if ((action_ == VortexEngine::MOVE_FORWARD) || (action_ == VortexEngine::MOVE_BACKWARD) || (action_ == VortexEngine::DRIVE))
    flyTimer_.stop();
*/
/*  if (action_ == VortexEngine::ZOOM_ON_REGION)
    camera->fitScreenRegion(QRect(pressPos_, event->pos()));
*/
  ManipulatedFrame::mouseReleaseEvent(event, camera);
}

/*! This is an overload of ManipulatedFrame::wheelEvent().

The wheel behavior depends on the wheel binded action. Current possible actions are QGLViewer::ZOOM,
QGLViewer::MOVE_FORWARD, QGLViewer::MOVE_BACKWARD. QGLViewer::ZOOM speed depends on
wheelSensitivity() while QGLViewer::MOVE_FORWARD and QGLViewer::MOVE_BACKWARD depend on flySpeed().
See QGLViewer::setWheelBinding() to customize the binding. */
bool ManipulatedCameraFrame::wheelEvent(const VortexEngine::MouseEvent* const event, Camera* const camera)
{
    bool modified =false;

  // #CONNECTION# QGLViewer::setWheelBinding, ManipulatedFrame::wheelEvent.
  switch (action_)
    {
    case VortexEngine::ZOOM:
      {
        const float wheelSensitivityCoef = 8E-4f;
        //#CONNECTION# mouseMoveEvent() ZOOM case
        const float coef = glm::max(fabsf((camera->frame()->coordinatesOf(camera->revolveAroundPoint())).z), 0.2f*camera->sceneRadius());
        glm::vec3 trans(0.0, 0.0, coef * event->delta * wheelSensitivity() * wheelSensitivityCoef);
        translate(inverseTransformOf(trans));
        //Q_EMIT manipulated();
        modified=true;
        break;
      }
    case VortexEngine::MOVE_FORWARD:
    case VortexEngine::MOVE_BACKWARD:
      //#CONNECTION# mouseMoveEvent() MOVE_FORWARD case
      translate(inverseTransformOf(glm::vec3(0.0, 0.0, 0.2*flySpeed()*event->delta)));
      //Q_EMIT manipulated();
      modified=true;
      break;
    default:
      break;
    }

  // #CONNECTION# startAction should always be called before
  if (previousConstraint_)
    setConstraint(previousConstraint_);

  // The wheel triggers a fastDraw. A final updateGL is needed after the last wheel event to
  // polish the rendering using draw(). Since the last wheel event does not say its name, we use
  // the flyTimer_ to trigger flyUpdate(), which emits manipulated. Two wheel events
  // separated by more than this delay milliseconds will trigger a draw().
/*
  const int finalDrawAfterWheelEventDelay = 400;

  // Starts (or prolungates) the timer.
#if QT_VERSION >= 0x040000
  flyTimer_.setSingleShot(true);
  flyTimer_.start(finalDrawAfterWheelEventDelay);
#else
  flyTimer_.start(finalDrawAfterWheelEventDelay, true);
#endif
*/
  // This could also be done *before* manipulated is emitted, so that isManipulated() returns false.
  // But then fastDraw would not be used with wheel.
  // Detecting the last wheel event and forcing a final draw() is done using the timer_.
  action_ = VortexEngine::NO_MOUSE_ACTION;
  return modified;
}

////////////////////////////////////////////////////////////////////////////////

/*! Returns a Quaternion that is a rotation around current camera Y, proportionnal to the horizontal mouse position. */
Quaternion ManipulatedCameraFrame::turnQuaternion(int x, const Camera* const camera)
{
  return Quaternion(glm::vec3(0.0, 1.0, 0.0), rotationSensitivity()*(prevPos_.x-x)/camera->screenWidth());
}

/*! Returns a Quaternion that is the composition of two rotations, inferred from the
  mouse pitch (X axis) and yaw (flyUpVector() axis). */
Quaternion ManipulatedCameraFrame::pitchYawQuaternion(int x, int y, const Camera* const camera)
{
  const Quaternion rotX(glm::vec3(1.0, 0.0, 0.0), rotationSensitivity()*(prevPos_.y-y)/camera->screenHeight());
  const Quaternion rotY(transformOf(flyUpVector()), rotationSensitivity()*(prevPos_.x-x)/camera->screenWidth());
  return rotY * rotX;
}

} // namespace vortex
