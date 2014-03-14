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
#include <cstdlib>

#include "manipulatedframe.h"
#include "camera.h"



 namespace vortex {
 using namespace util;

/*! Default constructor.

  The translation is set to (0,0,0), with an identity rotation (0,0,0,1) (see Frame constructor
  for details).

  The different sensitivities are set to their default values (see rotationSensitivity(),
  translationSensitivity(), spinningSensitivity() and wheelSensitivity()). */
ManipulatedFrame::ManipulatedFrame()
  : action_(VortexEngine::NO_MOUSE_ACTION)//, keepsGrabbingMouse_(false)
{

  setRotationSensitivity(1.0f);
  setTranslationSensitivity(1.0f);
  setSpinningSensitivity(0.3f);
  setWheelSensitivity(1.0f);

  isSpinning_ = false;
  previousConstraint_ = NULL;

  //connect(&spinningTimer_, SIGNAL(timeout()), SLOT(spinUpdate()));
}

/*! Equal operator. Calls Frame::operator=() and then copy attributes. */
ManipulatedFrame& ManipulatedFrame::operator=(const ManipulatedFrame& mf)
{
  Frame::operator=(mf);

  setRotationSensitivity(mf.rotationSensitivity());
  setTranslationSensitivity(mf.translationSensitivity());
  setSpinningSensitivity(mf.spinningSensitivity());
  setWheelSensitivity(mf.wheelSensitivity());

  mouseSpeed_ = 0.0;
  dirIsFixed_ = false;
  //keepsGrabbingMouse_ = false;

  return *this;
}

/*! Copy constructor. Performs a deep copy of all attributes using operator=(). */
ManipulatedFrame::ManipulatedFrame(const ManipulatedFrame& mf)
  : Frame(mf)//, MouseGrabber()
{
  (*this)=mf;
}

#if 0
////////////////////////////////////////////////////////////////////////////////

/*! Implementation of the MouseGrabber main method.

The ManipulatedFrame grabsMouse() when the mouse is within a 10 pixels region around its
Camera::projectedCoordinatesOf() position().

See the <a href="../examples/mouseGrabber.html">mouseGrabber example</a> for an illustration. */
void ManipulatedFrame::checkIfGrabsMouse(int x, int y, const Camera* const camera)
{
  const int thresold = 10;
  const Vec proj = camera->projectedCoordinatesOf(position());
  setGrabsMouse(keepsGrabbingMouse_ || ((fabs(x-proj.x) < thresold) && (fabs(y-proj.y) < thresold)));
}
#endif

////////////////////////////////////////////////////////////////////////////////
//                 M o u s e    h a n d l i n g                               //
////////////////////////////////////////////////////////////////////////////////

/*! Returns \c true when the ManipulatedFrame is being manipulated with the mouse.

  Can be used to change the display of the manipulated object during manipulation.

  When Camera::frame() of the QGLViewer::camera() isManipulated(), QGLViewer::fastDraw() is used in
  place of QGLViewer::draw() for scene rendering. A simplified drawing will then allow for
  interactive camera displacements.  */
bool ManipulatedFrame::isManipulated() const
{
  return action_ != 0;//QGLViewer::NO_MOUSE_ACTION;
}

/*! Starts the spinning of the ManipulatedFrame.

This method starts a timer that will call spin() every \p updateInterval milliseconds. The
ManipulatedFrame isSpinning() until you call stopSpinning(). */
void ManipulatedFrame::startSpinning(int updateInterval)
{
  isSpinning_ = true;
//  spinningTimer_.start(updateInterval);
}

/*! Rotates the ManipulatedFrame by its spinningQuaternion(). Called by a timer when the
  ManipulatedFrame isSpinning(). */
void ManipulatedFrame::spin()
{
  rotate(spinningQuaternion());
}

/* spin() and spinUpdate() differ since spin can be used by itself (for instance by
   QGLViewer::SCREEN_ROTATE) without a spun emission. Much nicer to use the spinningQuaternion() and
   hence spin() for these incremental updates. Nothing special to be done for continuous spinning
   with this design. */
void ManipulatedFrame::spinUpdate()
{
  spin();
  //Q_EMIT spun();
}


/*! Protected internal method used to handle mouse events. */
void ManipulatedFrame::startAction(int ma, bool withConstraint)
{
  action_ = ma;

  // #CONNECTION# manipulatedFrame::wheelEvent, manipulatedCameraFrame::wheelEvent and mouseReleaseEvent()
  // restore previous constraint
  if (withConstraint)
    previousConstraint_ = NULL;
  else
    {
      previousConstraint_ = constraint();
      setConstraint(NULL);
    }

  switch (action_)
    {
    case VortexEngine::ROTATE:
    case VortexEngine::SCREEN_ROTATE:
      mouseSpeed_ = 0.0;
      //stopSpinning();
      break;

    case VortexEngine::SCREEN_TRANSLATE:
      dirIsFixed_ = false;
      break;

    default:
      break;
    }
}

/*! Updates mouse speed, measured in pixels/milliseconds. Should be called by any method which wants to
use mouse speed. Currently used to trigger spinning in mouseReleaseEvent(). */
void ManipulatedFrame::computeMouseSpeed(const VortexEngine::MouseEvent* const e)
{
  const glm::ivec2 delta = glm::ivec2(e->x, e->y) - prevPos_;
  const float dist = sqrt(delta.x*delta.x + delta.y*delta.y);
  delay_ = last_move_time.restart();
  if (delay_ == 0)
    // Less than a millisecond: assume delay = 1ms
    mouseSpeed_ = dist;
  else
    mouseSpeed_ = dist/delay_;
}

/*! Return 1 if mouse motion was started horizontally and -1 if it was more vertical. Returns 0 if
this could not be determined yet (perfect diagonal motion, rare). */
int ManipulatedFrame::mouseOriginalDirection(const VortexEngine::MouseEvent* const e)
{
  static bool horiz = true; // Two simultaneous manipulatedFrame require two mice !

  if (!dirIsFixed_)
    {
      const glm::ivec2 delta = glm::ivec2(e->x, e->y) - pressPos_;
      dirIsFixed_ = abs(delta.x) != abs(delta.y);
      horiz = abs(delta.x) > abs(delta.y);
    }

  if (dirIsFixed_)
    if (horiz)
      return 1;
    else
      return -1;
  else
    return 0;
}



/*! Initiates the ManipulatedFrame mouse manipulation.

Overloading of MouseGrabber::mousePressEvent(). See also mouseMoveEvent() and mouseReleaseEvent().

The mouse behavior depends on which button is pressed. See the <a href="../mouse.html">QGLViewer
mouse page</a> for details. */
void ManipulatedFrame::mousePressEvent(const VortexEngine::MouseEvent* const event, Camera* const camera)
{
    (void)camera;

  /* if (grabsMouse())
    keepsGrabbingMouse_ = true;
*/

    prevPos_ = pressPos_ = glm::ivec2(event->x, event->y);
}

/*! Modifies the ManipulatedFrame according to the mouse motion.

Actual behavior depends on mouse bindings. See the QGLViewer::MouseAction enum and the <a
href="../mouse.html">QGLViewer mouse page</a> for details.

The \p camera is used to fit the mouse motion with the display parameters (see
Camera::screenWidth(), Camera::screenHeight(), Camera::fieldOfView()).

Emits the manipulated() signal. */
bool ManipulatedFrame::mouseMoveEvent(const VortexEngine::MouseEvent* const event, Camera* const camera)
{
    std::cerr << "ManipulatedFrame::mouseMoveEvent (" << action_ << ")" << std::endl;
  switch (action_)
    {
    case VortexEngine::TRANSLATE:
      {
        const glm::ivec2 delta = glm::ivec2(event->x, event->y) - prevPos_;
        glm::vec3 trans(delta.x, -delta.y, 0.0);
        // Scale to fit the screen mouse displacement
        switch (camera->type())
          {
          case Camera::PERSPECTIVE :
            trans *= 2.0 * tan(camera->fieldOfView()/2.0) * fabs((camera->frame()->coordinatesOf(position())).z) / camera->screenHeight();
            break;
          case Camera::ORTHOGRAPHIC :
            {
              double w,h;
              camera->getOrthoWidthHeight(w, h);
              trans[0] *= 2.0 * w / camera->screenWidth();
              trans[1] *= 2.0 * h / camera->screenHeight();
              break;
            }
          }
        // Transform to world coordinate system.
        trans = camera->frame()->orientation().rotate(translationSensitivity()*trans);
        // And then down to frame
        if (referenceFrame()) trans = referenceFrame()->transformOf(trans);
        translate(trans);

        break;
      }

    case VortexEngine::ZOOM:
      {
        //#CONNECTION# wheelEvent ZOOM case
      //std::cerr << "camera position : " << camera->position() << " -- frame position " << position() << std::endl;
        glm::vec3 trans(0.0, 0.0, glm::length(camera->position()-position()) * (event->y - prevPos_.y) / camera->screenHeight());

        trans = camera->frame()->orientation().rotate(trans);
        if (referenceFrame())
          trans = referenceFrame()->transformOf(trans);
        translate(trans);

        break;
      }

    case VortexEngine::SCREEN_ROTATE:
      {
        glm::vec3 trans = camera->projectedCoordinatesOf(position());

        const double prev_angle = atan2(prevPos_.y-trans[1], prevPos_.x-trans[0]);
        const double      angle = atan2(event->y-trans[1], event->x-trans[0]);

        const glm::vec3 axis = transformOf(camera->frame()->inverseTransformOf(glm::vec3(0.0, 0.0, -1.0)));
        Quaternion rot(axis, (angle-prev_angle));
        //#CONNECTION# These two methods should go together (spinning detection and activation)
        computeMouseSpeed(event);
        setSpinningQuaternion(rot);
        spin();
        break;
      }

    case VortexEngine::SCREEN_TRANSLATE:
      {
        glm::vec3 trans;
        int dir = mouseOriginalDirection(event);
        if (dir == 1)
          trans = glm::vec3(event->x - prevPos_.x, 0.0, 0.0);
        else if (dir == -1)
          trans = glm::vec3(0.0, prevPos_.y - event->y, 0.0);

        switch (camera->type())
          {
          case Camera::PERSPECTIVE :
            trans *= 2.0 * tan(camera->fieldOfView()/2.0) * fabs((camera->frame()->coordinatesOf(position())).z) / camera->screenHeight();
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
        // Transform to world coordinate system.
        trans = camera->frame()->orientation().rotate(translationSensitivity()*trans);
        // And then down to frame
        if (referenceFrame())
          trans = referenceFrame()->transformOf(trans);

        translate(trans);
        break;
      }

    case VortexEngine::ROTATE:
      {
        glm::vec3 trans = camera->projectedCoordinatesOf(position());
        Quaternion rot = deformedBallQuaternion(event->x, event->y, trans[0], trans[1], camera);
        trans = glm::vec3(-rot[0], -rot[1], -rot[2]);
        trans = camera->frame()->orientation().rotate(trans);
        trans = transformOf(trans);
        rot[0] = trans[0];
        rot[1] = trans[1];
        rot[2] = trans[2];
        //#CONNECTION# These two methods should go together (spinning detection and activation)
        computeMouseSpeed(event);
        setSpinningQuaternion(rot);
        spin();
        break;
      }

    case VortexEngine::NO_MOUSE_ACTION:
      // Possible when the ManipulatedFrame is a MouseGrabber. This method is then called without startAction
      // because of mouseTracking.
      break;
    }

  if (action_ != VortexEngine::NO_MOUSE_ACTION) {
      prevPos_ = glm::ivec2(event->x, event->y);

      return true;
  } else
      return false;
}

/*! Stops the ManipulatedFrame mouse manipulation.

Overloading of MouseGrabber::mouseReleaseEvent().

If the action was a QGLViewer::ROTATE QGLViewer::MouseAction, a continuous spinning is possible if
the speed of the mouse cursor is larger than spinningSensitivity() when the button is released.
Press the rotate button again to stop spinning. See startSpinning() and isSpinning(). */
void ManipulatedFrame::mouseReleaseEvent(const VortexEngine::MouseEvent* const event, Camera* const camera)
{
  (void)event;
  (void)camera;

  //keepsGrabbingMouse_ = false;

  if (previousConstraint_)
    setConstraint(previousConstraint_);

  /*
  if (((action_ == VortexEngine::ROTATE) || (action_ == VortexEngine::SCREEN_ROTATE)) && (mouseSpeed_ >= spinningSensitivity()))
    startSpinning(delay_);
  */
  action_ = VortexEngine::NO_MOUSE_ACTION;
}

#if 0
/*! Overloading of MouseGrabber::mouseDoubleClickEvent().

Left button double click aligns the ManipulatedFrame with the \p camera axis (see alignWithFrame()
 and QGLViewer::ALIGN_FRAME). Right button projects the ManipulatedFrame on the \p camera view
 direction. */
void ManipulatedFrame::mouseDoubleClickEvent(const VortexEngine::MouseEvent* const event, Camera* const camera)
{
#if QT_VERSION >= 0x040000
  if (event->modifiers() == Qt::NoModifier)
#else
  if (event->state() == Qt::NoButton)
#endif
    switch (event->button())
      {
      case Qt::LeftButton:  alignWithFrame(camera->frame()); break;
      case Qt::RightButton: projectOnLine(camera->position(), camera->viewDirection()); break;
      default: break;
      }
}
#endif

/*! Overloading of MouseGrabber::wheelEvent().

Using the wheel is equivalent to a QGLViewer::ZOOM QGLViewer::MouseAction. See
 QGLViewer::setWheelBinding() and setWheelSensitivity(). */
bool ManipulatedFrame::wheelEvent(const VortexEngine::MouseEvent* const event, Camera* const camera)
{
  //#CONNECTION# QGLViewer::setWheelBinding
  if (action_ == VortexEngine::ZOOM)
    {
      const float wheelSensitivityCoef = 8E-4f;
      glm::vec3 trans(0.0, 0.0, -event->delta*wheelSensitivity()*wheelSensitivityCoef*glm::length(camera->position()-position()));

      //#CONNECTION# Cut-pasted from the mouseMoveEvent ZOOM case
      trans = camera->frame()->orientation().rotate(trans);
      if (referenceFrame())
        trans = referenceFrame()->transformOf(trans);
      translate(trans);
      return true;
    }

  // #CONNECTION# startAction should always be called before
  if (previousConstraint_)
    setConstraint(previousConstraint_);

   action_ = VortexEngine::NO_MOUSE_ACTION;
  return false;
}


////////////////////////////////////////////////////////////////////////////////

/*! Returns "pseudo-distance" from (x,y) to ball of radius size.
\arg for a point inside the ball, it is proportional to the euclidean distance to the ball
\arg for a point outside the ball, it is proportional to the inverse of this distance (tends tod
zero) on the ball, the function is continuous. */
static float projectOnBall(float x, float y)
{
  // If you change the size value, change angle computation in deformedBallQuaternion().
  const float size       = 1.0f;
  const float size2      = size*size;
  const float size_limit = size2*0.5;

  const float d = x*x + y*y;
  return d < size_limit ? sqrt(size2 - d) : size_limit/sqrt(d);
}


/*! Returns a quaternion computed according to the mouse motion. Mouse positions are projected on a
deformed ball, centered on (\p cx,\p cy). */
Quaternion ManipulatedFrame::deformedBallQuaternion(int x, int y, float cx, float cy, const Camera* const camera)
{
  // Points on the deformed ball
  float px = rotationSensitivity() * (float)(prevPos_.x  - cx) / (float)(camera->screenWidth());
  float py = rotationSensitivity() * (float)(cy - prevPos_.y)  / (float)(camera->screenHeight());
  float dx = rotationSensitivity() * (float)(x - cx)	    / (float)(camera->screenWidth());
  float dy = rotationSensitivity() * (float)(cy - y)	    / (float)(camera->screenHeight());

  const glm::vec3  p1(px, py, projectOnBall(px, py));
  const glm::vec3  p2(dx, dy, projectOnBall(dx, dy));
  // Approximation of rotation angle
  // Should be divided by the projectOnBall size, but it is 1.0
  const glm::vec3  axis = glm::cross(p2,p1);
  const float angle = 2.0 * asin(sqrt(glm::dot(axis, axis) / (glm::dot(p1, p1) * glm::dot(p2, p2))));
  return Quaternion(axis, angle);
}


} // namespace vortex
