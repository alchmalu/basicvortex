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

#include "camera.h"

namespace vortex {
using namespace util;
/*! Default constructor.

 sceneCenter() is set to (0,0,0) and sceneRadius() is set to 1.0. type() is Camera::PERSPECTIVE,
 with a \c M_PI/4 fieldOfView().

 See IODistance(), physicalDistanceToScreen(), physicalScreenWidth() and focusDistance()
 documentations for default stereo parameter values. */
Camera::Camera()
  : fieldOfView_(M_PI/4.0f)
{
  // #CONNECTION# Camera copy constructor
  //interpolationKfi_ = new KeyFrameInterpolator;
  // Requires the interpolationKfi_
  setFrame(new ManipulatedCameraFrame());

  // #CONNECTION# All these default values identical in initFromDOMElement.

  // Requires fieldOfView() to define focusDistance()
  setSceneRadius(1.0);

  // Initial value (only scaled after this)
  orthoCoef_ = tan(fieldOfView()/2.0);

  // Also defines the revolveAroundPoint(), which changes orthoCoef_. Requires a frame().
  setSceneCenter(glm::vec3(0.0, 0.0, 0.0));

  // Requires fieldOfView() when called with ORTHOGRAPHIC. Attention to projectionMatrix_ below.
  setType(PERSPECTIVE);

  // #CONNECTION# initFromDOMElement default values
  setZNearCoefficient(0.005f);
  setZClippingCoefficient(sqrt(3.0));

  // Dummy values
  setScreenWidthAndHeight(600, 400);

  // Stereo parameters
  setIODistance(0.062f);
  setPhysicalDistanceToScreen(0.5f);
  setPhysicalScreenWidth(0.4f);
  // focusDistance is set from setFieldOfView()

  // #CONNECTION# Camera copy constructor
  for (unsigned short j=0; j<16; ++j)
    {
      modelViewMatrix_[j] = ((j%5 == 0) ? 1.0 : 0.0);
      // #CONNECTION# computeProjectionMatrix() is lazy and assumes 0.0 almost everywhere.
      projectionMatrix_[j] = 0.0;
    }
  computeProjectionMatrix();
}

/*! Virtual destructor.

 The frame() is deleted, but the different keyFrameInterpolator() are \e not deleted (in case they
 are shared). */
Camera::~Camera()
{
  delete frame_;
  //delete interpolationKfi_;
}


/*! Copy constructor. Performs a deep copy using operator=(). */
Camera::Camera(const Camera& camera)
{
  // #CONNECTION# Camera constructor
  //interpolationKfi_ = new KeyFrameInterpolator;
  // Requires the interpolationKfi_
  setFrame(new ManipulatedCameraFrame());

  for (unsigned short j=0; j<16; ++j)
    {
      modelViewMatrix_[j] = ((j%5 == 0) ? 1.0 : 0.0);
      // #CONNECTION# computeProjectionMatrix() is lazy and assumes 0.0 almost everywhere.
      projectionMatrix_[j] = 0.0;
    }

  (*this)=camera;
}

/*! Equal operator.

 All the parameters of \p camera are copied. The frame() pointer is not modified, but its
 Frame::position() and Frame::orientation() are set to those of \p camera.

 \attention The Camera screenWidth() and screenHeight() are set to those of \p camera. If your
 Camera is associated with a QGLViewer, you should update these value after the call to this method:
 \code
 *(camera()) = otherCamera;
 camera()->setScreenWidthAndHeight(width(), height());
 \endcode
 The same applies to sceneCenter() and sceneRadius(), if needed. */
Camera& Camera::operator=(const Camera& camera)
{
  setScreenWidthAndHeight(camera.screenWidth(), camera.screenHeight());
  setFieldOfView(camera.fieldOfView());
  setSceneRadius(camera.sceneRadius());
  setSceneCenter(camera.sceneCenter());
  setZNearCoefficient(camera.zNearCoefficient());
  setZClippingCoefficient(camera.zClippingCoefficient());
  setType(camera.type());

  // Stereo parameters
  setIODistance(camera.IODistance());
  setFocusDistance(camera.focusDistance());
  setPhysicalScreenWidth(camera.physicalScreenWidth());
  setPhysicalDistanceToScreen(camera.physicalDistanceToScreen());

  orthoCoef_ = camera.orthoCoef_;

  // frame_ and interpolationKfi_ pointers are not shared.
  frame_->setReferenceFrame(NULL);
  frame_->setPosition(camera.position());
  frame_->setOrientation(camera.orientation());

  //interpolationKfi_->resetInterpolation();

  //kfi_ = camera.kfi_;

  computeProjectionMatrix();
  computeModelViewMatrix();

  return *this;
}

/*! Sets Camera screenWidth() and screenHeight() (expressed in pixels).

You should not call this method when the Camera is associated with a QGLViewer, since the
latter automatically updates these values when it is resized (hence overwritting your values).

Non-positive dimension are silently replaced by a 1 pixel value to ensure frustrum coherence.

If your Camera is used without a QGLViewer (offscreen rendering, shadow maps), use setAspectRatio()
instead to define the projection matrix. */
void Camera::setScreenWidthAndHeight(int width, int height)
{
  // Prevent negative and zero dimensions that would cause divisions by zero.
        screenWidth_  = width > 0 ? width : 1;
        screenHeight_ = height > 0 ? height : 1;
}

/*! Returns the near clipping plane distance used by the Camera projection matrix.

 The clipping planes' positions depend on the sceneRadius() and sceneCenter() rather than being fixed
 small-enough and large-enough values. A good scene dimension approximation will hence result in an
 optimal precision of the z-buffer.

 The near clipping plane is positioned at a distance equal to zClippingCoefficient() * sceneRadius()
 in front of the sceneCenter():
 \code
 zNear = distanceToSceneCenter() - zClippingCoefficient()*sceneRadius();
 \endcode

 In order to prevent negative or too small zNear() values (which would degrade the z precision),
 zNearCoefficient() is used when the Camera is inside the sceneRadius() sphere:
 \code
 const float zMin = zNearCoefficient() * zClippingCoefficient() * sceneRadius();
 if (zNear < zMin)
   zNear = zMin;
 // With an ORTHOGRAPHIC type, the value is simply clamped to 0.0
 \endcode

 See also the zFar(), zClippingCoefficient() and zNearCoefficient() documentations.

 If you need a completely different zNear computation, overload the zNear() and zFar() methods in a
 new class that publicly inherits from Camera and use QGLViewer::setCamera():
 \code
 class myCamera :: public qglviewer::Camera
 {
   virtual float Camera::zNear() const { return 0.001; };
   virtual float Camera::zFar() const { return 100.0; };
 }
 \endcode

 See the <a href="../examples/standardCamera.html">standardCamera example</a> for an application.

 \attention The value is always positive although the clipping plane is positioned at a negative z
 value in the Camera coordinate system. This follows the \c gluPerspective standard. */
float Camera::zNear() const
{
  float z = distanceToSceneCenter() - zClippingCoefficient()*sceneRadius();

  // Prevents negative or null zNear values.
  const float zMin = zNearCoefficient() * zClippingCoefficient() * sceneRadius();
  if (z < zMin)
    switch (type())
      {
      case Camera::PERSPECTIVE  : z = zMin; break;
      case Camera::ORTHOGRAPHIC : z = 0.0;  break;
      }
  return z;
}

/*! Returns the far clipping plane distance used by the Camera projection matrix.

The far clipping plane is positioned at a distance equal to zClippingCoefficient() * sceneRadius()
behind the sceneCenter():
\code
zFar = distanceToSceneCenter() + zClippingCoefficient()*sceneRadius();
\endcode

See the zNear() documentation for details. */
float Camera::zFar() const
{
  return distanceToSceneCenter() + zClippingCoefficient()*sceneRadius();
}

/*! Defines the Camera type().

Changing the camera Type alters the viewport and the objects' size can be changed. This method garantees that the two frustum match in a plane normal to viewDirection(), passing through the Revolve Around Point (RAP).

Prefix the type with \c Camera if needed, as in:
\code
camera()->setType(Camera::ORTHOGRAPHIC);
// or even qglviewer::Camera::ORTHOGRAPHIC if you do not use namespace
\endcode */
void Camera::setType(Type type)
{
  // make ORTHOGRAPHIC frustum fit PERSPECTIVE (at least in plane normal to viewDirection(), passing
  // through RAP). Done only when CHANGING type since orthoCoef_ may have been changed with a
  // setRevolveAroundPoint() in the meantime.
  if ( (type == Camera::ORTHOGRAPHIC) && (type_ == Camera::PERSPECTIVE) )
    orthoCoef_ = tan(fieldOfView()/2.0);
  type_ = type;
}

/*! Sets the Camera frame().

If you want to move the Camera, use setPosition() and setOrientation() or one of the Camera
positioning methods (lookAt(), fitSphere(), showEntireScene()...) instead.

If you want to save the Camera position(), there's no need to call this method either. Use
addKeyFrameToPath() and playPath() instead.

This method is actually mainly useful if you derive the ManipulatedCameraFrame class and want to
use an instance of your new class to move the Camera.

A \c NULL \p mcf pointer will silently be ignored. The calling method is responsible for
deleting the previous frame() pointer if needed in order to prevent memory leaks. */
void Camera::setFrame(ManipulatedCameraFrame* const mcf)
{
  if (!mcf)
    return;

  frame_ = mcf;
  //interpolationKfi_->setFrame(frame());
}

/*! Returns the distance from the Camera center to sceneCenter(), projected along the Camera Z axis.
  Used by zNear() and zFar() to optimize the Z range. */
float Camera::distanceToSceneCenter() const
{
  return fabs((frame()->coordinatesOf(sceneCenter())).z);
}


/*! Returns the \p halfWidth and \p halfHeight of the Camera orthographic frustum.

 These values are only valid and used when the Camera is of type() Camera::ORTHOGRAPHIC. They are
 expressed in OpenGL units and are used by loadProjectionMatrix() to define the projection matrix
 using:
 \code
 glOrtho( -halfWidth, halfWidth, -halfHeight, halfHeight, zNear(), zFar() )
 \endcode

 These values are proportional to the Camera (z projected) distance to the revolveAroundPoint().
 When zooming on the object, the Camera is translated forward \e and its frustum is narrowed, making
 the object appear bigger on screen, as intuitively expected.

 Overload this method to change this behavior if desired, as is done in the
 <a href="../examples/standardCamera.html">standardCamera example</a>. */
void Camera::getOrthoWidthHeight(GLdouble& halfWidth, GLdouble& halfHeight) const
{
  const float dist = orthoCoef_ * fabs(cameraCoordinatesOf(revolveAroundPoint()).z);
  //#CONNECTION# fitScreenRegion
  halfWidth  = dist * ((aspectRatio() < 1.0) ? 1.0 : aspectRatio());
  halfHeight = dist * ((aspectRatio() < 1.0) ? 1.0/aspectRatio() : 1.0);
}


/*! Computes the projection matrix associated with the Camera.

 If type() is Camera::PERSPECTIVE, defines a \c GL_PROJECTION matrix similar to what would \c
 gluPerspective() do using the fieldOfView(), window aspectRatio(), zNear() and zFar() parameters.

 If type() is Camera::ORTHOGRAPHIC, the projection matrix is as what \c glOrtho() would do.
 Frustum's width and height are set using getOrthoWidthHeight().

 Both types use zNear() and zFar() to place clipping planes. These values are determined from
 sceneRadius() and sceneCenter() so that they best fit the scene size.

 Use getProjectionMatrix() to retrieve this matrix. Overload loadProjectionMatrix() if you want your
 Camera to use an exotic projection matrix.

 \note You must call this method if your Camera is not associated with a QGLViewer and is used for
 offscreen computations (using (un)projectedCoordinatesOf() for instance). loadProjectionMatrix()
 does it otherwise. */
void Camera::computeProjectionMatrix() const
{
  const float ZNear = zNear();
  const float ZFar  = zFar();

  switch (type())
    {
    case Camera::PERSPECTIVE:
      {
        // #CONNECTION# all non null coefficients were set to 0.0 in constructor.
        const float f = 1.0/tan(fieldOfView()/2.0);
        projectionMatrix_[0]  = f/aspectRatio();
        projectionMatrix_[5]  = f;
        projectionMatrix_[10] = (ZNear + ZFar) / (ZNear - ZFar);
        projectionMatrix_[11] = -1.0;
        projectionMatrix_[14] = 2.0 * ZNear * ZFar / (ZNear - ZFar);
        projectionMatrix_[15] = 0.0;
        // same as gluPerspective( 180.0*fieldOfView()/M_PI, aspectRatio(), zNear(), zFar() );
        break;
      }
    case Camera::ORTHOGRAPHIC:
      {
        GLdouble w, h;
        getOrthoWidthHeight(w,h);
        projectionMatrix_[0]  = 1.0/w;
        projectionMatrix_[5]  = 1.0/h;
        projectionMatrix_[10] = -2.0/(ZFar - ZNear);
        projectionMatrix_[11] = 0.0;
        projectionMatrix_[14] = -(ZFar + ZNear)/(ZFar - ZNear);
        projectionMatrix_[15] = 1.0;
        // same as glOrtho( -w, w, -h, h, zNear(), zFar() );
        break;
      }
    }
}

/*! Computes the modelView matrix associated with the Camera's position() and orientation().

 This matrix converts from the world coordinates system to the Camera coordinates system, so that
 coordinates can then be projected on screen using the projection matrix (see computeProjectionMatrix()).

 Use getModelViewMatrix() to retrieve this matrix.

 \note You must call this method if your Camera is not associated with a QGLViewer and is used for
 offscreen computations (using (un)projectedCoordinatesOf() for instance). loadModelViewMatrix()
 does it otherwise. */
void Camera::computeModelViewMatrix() const
{
  const Quaternion q = frame()->orientation();

  const double q00 = 2.0l * q[0] * q[0];
  const double q11 = 2.0l * q[1] * q[1];
  const double q22 = 2.0l * q[2] * q[2];

  const double q01 = 2.0l * q[0] * q[1];
  const double q02 = 2.0l * q[0] * q[2];
  const double q03 = 2.0l * q[0] * q[3];

  const double q12 = 2.0l * q[1] * q[2];
  const double q13 = 2.0l * q[1] * q[3];

  const double q23 = 2.0l * q[2] * q[3];

  modelViewMatrix_[0] = 1.0l - q11 - q22;
  modelViewMatrix_[1] =        q01 - q23;
  modelViewMatrix_[2] =        q02 + q13;
  modelViewMatrix_[3] = 0.0l;

  modelViewMatrix_[4] =        q01 + q23;
  modelViewMatrix_[5] = 1.0l - q22 - q00;
  modelViewMatrix_[6] =        q12 - q03;
  modelViewMatrix_[7] = 0.0l;

  modelViewMatrix_[8] =        q02 - q13;
  modelViewMatrix_[9] =        q12 + q03;
  modelViewMatrix_[10] = 1.0l - q11 - q00;
  modelViewMatrix_[11] = 0.0l;

  const glm::vec3 t = q.inverseRotate(frame()->position());

  modelViewMatrix_[12] = -t.x;
  modelViewMatrix_[13] = -t.y;
  modelViewMatrix_[14] = -t.z;
  modelViewMatrix_[15] = 1.0l;
}



/*! Fills \p m with the Camera projection matrix values.

 Calls computeProjectionMatrix() to define the Camera projection matrix.

 This matrix only reflects the Camera's internal parameters and it may differ from the \c
 GL_PROJECTION matrix retrieved using \c glGetDoublev(GL_PROJECTION_MATRIX, m). It actually
 represents the state of the \c GL_PROJECTION after QGLViewer::preDraw(), at the beginning of
 QGLViewer::draw(). If you modified the \c GL_PROJECTION matrix (for instance using
 QGLViewer::startScreenCoordinatesSystem()), the two results differ.

 The result is an OpenGL 4x4 matrix, which is given in \e column-major order (see \c glMultMatrix
 man page for details).

 See also getModelViewMatrix() and setFromProjectionMatrix(). */
void Camera::getProjectionMatrix(double m[16]) const
{
  // May not be needed, but easier and more robust like this.
  computeProjectionMatrix();
  for (unsigned short i=0; i<16; ++i)
    m[i] = projectionMatrix_[i];
}

/*! Fills \p m with the Camera modelView matrix values.

 First calls computeModelViewMatrix() to define the Camera modelView matrix.

 Note that this matrix is usually \e not the one you would get from a \c
 glGetDoublev(GL_MODELVIEW_MATRIX, m). It actually represents the state of the \c
 GL_MODELVIEW after QGLViewer::preDraw(), at the beginning of QGLViewer::draw(). It converts from
 the world to the Camera coordinate system. As soon as you modify the \c GL_MODELVIEW in your
 QGLViewer::draw() method, the two matrices differ.

 The result is an OpenGL 4x4 matrix, which is given in \e column-major order (see \c glMultMatrix
 man page for details).

 See also getProjectionMatrix() and setFromModelViewMatrix(). */
void Camera::getModelViewMatrix(double m[16]) const
{
  // May not be needed, but easier like this.
  // Prevents from retrieving matrix in stereo mode -> overwrites shifted value.
  computeModelViewMatrix();
  for (unsigned short i=0; i<16; ++i)
    m[i] = modelViewMatrix_[i];
}

/*! Fills \p m with the product of the ModelView and Projection matrices.

  Calls getModelViewMatrix() and getProjectionMatrix() and then fills \p m with the product of these two matrices. */
void Camera::getModelViewProjectionMatrix(double m[16]) const
{
  double mv[16];
  double proj[16];
  getModelViewMatrix(mv);
  getProjectionMatrix(proj);

  for (unsigned short i=0; i<4; ++i)
  {
    for (unsigned short j=0; j<4; ++j)
    {
      double sum = 0.0;
      for (unsigned short k=0; k<4; ++k)
        sum += proj[i+4*k]*mv[k+4*j];
      m[i+4*j] = sum;
    }
  }
}



void Camera::getProjectionMatrix(float m[16]) const
{
  //qWarning("Warning : Camera::getProjectionMatrix requires a GLdouble matrix array");
  static double mat[16];
  getProjectionMatrix(mat);
  for (int i=0; i<16; ++i)
    m[i] = float(mat[i]);
}

void Camera::getModelViewMatrix(float m[16]) const
{
  //qWarning("Warning : Camera::getModelViewMatrix requires a GLdouble matrix array");
  static double mat[16];
  getModelViewMatrix(mat);
  for (int i=0; i<16; ++i)
    m[i] = float(mat[i]);
}


/*! Sets the sceneRadius() value. Negative values are ignored.

\attention This methods also sets focusDistance() to sceneRadius() / tan(fieldOfView()/2) and
flySpeed() to 1% of sceneRadius(). */
void Camera::setSceneRadius(float radius)
{
  if (radius <= 0.0)
    {
      //qWarning("Scene radius must be positive - Ignoring value");
      return;
    }

  sceneRadius_ = radius;

  setFocusDistance(sceneRadius() / tan(fieldOfView()/2.0));

  frame()->setFlySpeed(0.01*sceneRadius());
}

/*! Similar to setSceneRadius() and setSceneCenter(), but the scene limits are defined by a (world
  axis aligned) bounding box. */
void Camera::setSceneBoundingBox(const glm::vec3& min, const glm::vec3& max)
{
  setSceneCenter((min+max)*0.5f);
  setSceneRadius(0.5*glm::length(max-min));
}


/*! Sets the sceneCenter().

 \attention This method also sets the revolveAroundPoint() to sceneCenter(). */
void Camera::setSceneCenter(const glm::vec3& center)
{
  sceneCenter_ = center;
  setRevolveAroundPoint(sceneCenter());
}


/*! setSceneCenter() to the result of pointUnderPixel(\p pixel).

  Returns \c true if a pointUnderPixel() was found and sceneCenter() was actually changed.

  See also setRevolveAroundPointFromPixel(). See the pointUnderPixel() documentation. */
bool Camera::setSceneCenterFromPixel(const glm::ivec2& pixel)
{
  bool found;
  glm::vec3 point = pointUnderPixel(pixel, found);
  if (found)
    setSceneCenter(point);
  return found;
}

/*! Changes the revolveAroundPoint() to \p rap (defined in the world coordinate system). */
void Camera::setRevolveAroundPoint(const glm::vec3& rap)
{

  const float prevDist = fabs(cameraCoordinatesOf(revolveAroundPoint()).z);

  frame()->setRevolveAroundPoint(rap);

  // orthoCoef_ is used to compensate for changes of the revolveAroundPoint, so that the image does
  // not change when the revolveAroundPoint is changed in ORTHOGRAPHIC mode.
  const float newDist = fabs(cameraCoordinatesOf(revolveAroundPoint()).z);
  // Prevents division by zero when rap is set to camera position
  if ((prevDist > 1E-9) && (newDist > 1E-9))
    orthoCoef_ *= prevDist / newDist;
}

/*! The revolveAroundPoint() is set to the point located under \p pixel on screen.

Returns \c true if a pointUnderPixel() was found. If no point was found under \p pixel, the
revolveAroundPoint() is left unchanged.

\p pixel is expressed in Qt format (origin in the upper left corner of the window). See
pointUnderPixel().

See also setSceneCenterFromPixel(). */
bool Camera::setRevolveAroundPointFromPixel(const glm::ivec2& pixel)
{
  bool found;
  // get point in the world space
  glm::vec3 point = pointUnderPixel(pixel, found);
  if (found) {
    setRevolveAroundPoint(point);
  }
  return found;
}

/*! Returns the ratio between pixel and OpenGL units at \p position.

 A line of \c n * pixelGLRatio() OpenGL units, located at \p position in the world coordinates
 system, will be projected with a length of \c n pixels on screen.

 Use this method to scale objects so that they have a constant pixel size on screen. The following
 code will draw a 20 pixel line, starting at sceneCenter() and always directed along the screen
 vertical direction:
 \code
 glBegin(GL_LINES);
 glVertex3fv(sceneCenter());
 glVertex3fv(sceneCenter() + 20 * pixelGLRatio(sceneCenter()) * camera()->upVector());
 glEnd();
 \endcode */
float Camera::pixelGLRatio(const glm::vec3& position) const
{
  switch (type())
    {
    case Camera::PERSPECTIVE :
      return 2.0 * fabs((frame()->coordinatesOf(position)).z) * tan(fieldOfView()/2.0) / screenHeight();
    case Camera::ORTHOGRAPHIC :
      {
        GLdouble w, h;
        getOrthoWidthHeight(w,h);
        return 2.0 * h / screenHeight();
      }
    }
  // Bad compilers complain
  return 1.0;
}

/*! Changes the Camera fieldOfView() so that the entire scene (defined by QGLViewer::sceneCenter()
 and QGLViewer::sceneRadius()) is visible from the Camera position().

 The position() and orientation() of the Camera are not modified and you first have to orientate the
 Camera in order to actually see the scene (see lookAt(), showEntireScene() or fitSphere()).

 This method is especially useful for \e shadow \e maps computation. Use the Camera positioning
 tools (setPosition(), lookAt()) to position a Camera at the light position. Then use this method to
 define the fieldOfView() so that the shadow map resolution is optimally used:
 \code
 // The light camera needs size hints in order to optimize its fieldOfView
 lightCamera->setSceneRadius(sceneRadius());
 lightCamera->setSceneCenter(sceneCenter());

 // Place the light camera.
 lightCamera->setPosition(lightFrame->position());
 lightCamera->lookAt(sceneCenter());
 lightCamera->setFOVToFitScene();
 \endcode

 See the (soon available) shadowMap contribution example for a practical implementation.

 \attention The fieldOfView() is clamped to M_PI/2.0. This happens when the Camera is at a distance
 lower than sqrt(2.0) * sceneRadius() from the sceneCenter(). It optimizes the shadow map
 resolution, although it may miss some parts of the scene. */
void Camera::setFOVToFitScene()
{
  if (distanceToSceneCenter() > sqrt(2.0)*sceneRadius())
    setFieldOfView(2.0 * asin(sceneRadius() / distanceToSceneCenter()));
  else
    setFieldOfView(M_PI / 2.0f);
}
#if 0
/*! Makes the Camera smoothly zoom on the pointUnderPixel() \p pixel.

 Nothing happens if no pointUnderPixel() is found. Otherwise a KeyFrameInterpolator is created that
 animates the Camera on a one second path that brings the Camera closer to the point under \p pixel.

 See also interpolateToFitScene(). */
void Camera::interpolateToZoomOnPixel(const QPoint& pixel)
{
  const float coef = 0.1f;

  bool found;
  Vec target = pointUnderPixel(pixel, found);

  if (!found)
    return;

  if (interpolationKfi_->interpolationIsStarted())
    interpolationKfi_->stopInterpolation();

  interpolationKfi_->deletePath();
  interpolationKfi_->addKeyFrame(*(frame()));

  interpolationKfi_->addKeyFrame(Frame(0.3f*frame()->position() + 0.7f*target, frame()->orientation()), 0.4f);

  // Small hack: attach a temporary frame to take advantage of lookAt without modifying frame
  static ManipulatedCameraFrame* tempFrame = new ManipulatedCameraFrame();
  ManipulatedCameraFrame* const originalFrame = frame();
  tempFrame->setPosition(coef*frame()->position() + (1.0-coef)*target);
  tempFrame->setOrientation(frame()->orientation());
  setFrame(tempFrame);
  lookAt(target);
  setFrame(originalFrame);

  interpolationKfi_->addKeyFrame(*(tempFrame), 1.0);

  interpolationKfi_->startInterpolation();
}
#endif
/*! Interpolates the Camera on a one second KeyFrameInterpolator path so that the entire scene fits
 the screen at the end.

 The scene is defined by its sceneCenter() and its sceneRadius(). See showEntireScene().

 The orientation() of the Camera is not modified. See also interpolateToZoomOnPixel(). */
void Camera::interpolateToFitScene()
{
  /*
  if (interpolationKfi_->interpolationIsStarted())
    interpolationKfi_->stopInterpolation();

  interpolationKfi_->deletePath();
  interpolationKfi_->addKeyFrame(*(frame()));


  // Small hack:  attach a temporary frame to take advantage of lookAt without modifying frame
  static ManipulatedCameraFrame* tempFrame = new ManipulatedCameraFrame();
  ManipulatedCameraFrame* const originalFrame = frame();
  tempFrame->setPosition(frame()->position());
  tempFrame->setOrientation(frame()->orientation());
  setFrame(tempFrame);
  showEntireScene();
  setFrame(originalFrame);

  interpolationKfi_->addKeyFrame(*(tempFrame));

  interpolationKfi_->startInterpolation();
  */
}


/*! Smoothly interpolates the Camera on a KeyFrameInterpolator path so that it goes to \p fr.

  \p fr is expressed in world coordinates. \p duration tunes the interpolation speed (default is
  1 second).

  See also interpolateToFitScene() and interpolateToZoomOnPixel(). */
void Camera::interpolateTo(const Frame& fr, float duration)
{
    /*
  if (interpolationKfi_->interpolationIsStarted())
    interpolationKfi_->stopInterpolation();

  interpolationKfi_->deletePath();
  interpolationKfi_->addKeyFrame(*(frame()));
  interpolationKfi_->addKeyFrame(fr, duration);

  interpolationKfi_->startInterpolation();
  */
}


/*! Returns the coordinates of the 3D point located at pixel (x,y) on screen.

 Calls a \c glReadPixel to get the pixel depth and applies an unprojectedCoordinatesOf() to the
 result. \p found indicates whether a point was found or not (i.e. background pixel, result's depth
 is zFar() in that case).

 \p x and \p y are expressed in pixel units with an origin in the upper left corner. Use
 screenHeight() - y to convert to OpenGL standard.

 \attention This method assumes that a GL context is available, and that its content was drawn using
 the Camera (i.e. using its projection and modelview matrices). This method hence cannot be used for
 offscreen Camera computations. Use cameraCoordinatesOf() and worldCoordinatesOf() to perform
 similar operations in that case.

 \note The precision of the z-Buffer highly depends on how the zNear() and zFar() values are fitted
 to your scene. Loose boundaries will result in imprecision along the viewing direction. */
glm::vec3 Camera::pointUnderPixel(const glm::ivec2& pixel, bool& found) const
{
  // Qt uses upper corner for its origin while GL uses the lower corner.
  //glReadPixels(pixel.x(), screenHeight()-1-pixel.y(), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
  found = (pointClicked.z < 1.0f);
  return unprojectedCoordinatesOf(pointClicked);
}


/*! Moves the Camera so that the entire scene is visible.

 Simply calls fitSphere() on a sphere defined by sceneCenter() and sceneRadius().

 You will typically use this method in QGLViewer::init() after you defined a new sceneRadius(). */
void Camera::showEntireScene()
{
  fitSphere(sceneCenter(), sceneRadius());
}

/*! Moves the Camera so that its sceneCenter() is projected on the center of the window. The
 orientation() and fieldOfView() are unchanged.

 Simply projects the current position on a line passing through sceneCenter(). See also
 showEntireScene().*/
void Camera::centerScene()
{
  frame()->projectOnLine(sceneCenter(), viewDirection());
}

/*! Sets the Camera orientation(), so that it looks at point \p target (defined in the world
 coordinate system).

 The Camera position() is not modified. Simply setViewDirection().

 See also setUpVector(), setOrientation(), showEntireScene(), fitSphere() and fitBoundingBox(). */
void Camera::lookAt(const glm::vec3& target)
{
  setViewDirection(target - position());
}

/*! Moves the Camera so that the sphere defined by (\p center, \p radius) is visible and fits the window.

 The Camera is simply translated along its viewDirection() so that the sphere fits the screen. Its
 orientation() and its fieldOfView() are unchanged.

 You should therefore orientate the Camera before you call this method. See lookAt(),
 setOrientation() and setUpVector(). */
void Camera::fitSphere(const glm::vec3& center, float radius)
{
  float distance = 0.0f;
  switch (type())
    {
    case Camera::PERSPECTIVE :
      {
        const float yview = radius / sin(fieldOfView()/2.0);
        const float xview = radius / sin(horizontalFieldOfView()/2.0);
        distance = glm::max(xview,yview);
        break;
      }
    case Camera::ORTHOGRAPHIC :
      {
        distance = glm::dot((center-revolveAroundPoint()), viewDirection()) + (radius / orthoCoef_);
        break;
      }
    }
  glm::vec3 newPos(center - distance * viewDirection());
  frame()->setPositionWithConstraint(newPos);
}

/*! Moves the Camera so that the (world axis aligned) bounding box (\p min, \p max) is entirely
  visible, using fitSphere(). */
void Camera::fitBoundingBox(const glm::vec3& min, const glm::vec3& max)
{
  float diameter = glm::max(fabsf(max[1]-min[1]), fabsf(max[0]-min[0])) ;
  diameter = glm::max(fabsf(max[2]-min[2]), diameter);
  fitSphere(0.5f*(min+max), 0.5f*diameter);
}

#if 0
/*! Moves the Camera so that the rectangular screen region defined by \p rectangle (pixel units,
  with origin in the upper left corner) fits the screen.

  The Camera is translated (its orientation() is unchanged) so that \p rectangle is entirely
  visible. Since the pixel coordinates only define a \e frustum in 3D, it's the intersection of this
  frustum with a plane (orthogonal to the viewDirection() and passing through the sceneCenter())
  that is used to define the 3D rectangle that is eventually fitted. */
void Camera::fitScreenRegion(const QRect& rectangle)
{
  const Vec vd = viewDirection();
  const float distToPlane = distanceToSceneCenter();
  const QPoint center = rectangle.center();

  Vec orig, dir;
  convertClickToLine( center, orig, dir );
  Vec newCenter = orig + distToPlane / (dir*vd) * dir;

  convertClickToLine( QPoint(rectangle.x(), center.y()), orig, dir );
  const Vec pointX = orig + distToPlane / (dir*vd) * dir;

  convertClickToLine( QPoint(center.x(), rectangle.y()), orig, dir );
  const Vec pointY = orig + distToPlane / (dir*vd) * dir;

  float distance = 0.0f;
  switch (type())
    {
    case Camera::PERSPECTIVE :
      {
        const float distX = (pointX-newCenter).norm() / sin(horizontalFieldOfView()/2.0);
        const float distY = (pointY-newCenter).norm() / sin(fieldOfView()/2.0);
        distance = qMax(distX, distY);
        break;
      }
    case Camera::ORTHOGRAPHIC :
      {
        const float dist = ((newCenter-revolveAroundPoint()) * vd);
        //#CONNECTION# getOrthoWidthHeight
        const float distX = (pointX-newCenter).norm() / orthoCoef_ / ((aspectRatio() < 1.0) ? 1.0 : aspectRatio());
        const float distY = (pointY-newCenter).norm() / orthoCoef_ / ((aspectRatio() < 1.0) ? 1.0/aspectRatio() : 1.0);
        distance = dist + qMax(distX, distY);
        break;
      }
    }

  Vec newPos(newCenter - distance * vd);
  frame()->setPositionWithConstraint(newPos);
}
#endif

/*! Rotates the Camera so that its upVector() becomes \p up (defined in the world coordinate
 system).

 The Camera is rotated around an axis orthogonal to \p up and to the current upVector() direction.
 Use this method in order to define the Camera horizontal plane.

 When \p noMove is set to \c false, the orientation modification is compensated by a translation, so
 that the revolveAroundPoint() stays projected at the same position on screen. This is especially
 useful when the Camera is an observer of the scene (default mouse binding).

 When \p noMove is \c true (default), the Camera position() is left unchanged, which is an intuitive
 behavior when the Camera is in a walkthrough fly mode (see the QGLViewer::MOVE_FORWARD and
 QGLViewer::MOVE_BACKWARD QGLViewer::MouseAction).

 See also setViewDirection(), lookAt() and setOrientation(). */
void Camera::setUpVector(const glm::vec3& up, bool noMove)
{
  Quaternion q(glm::vec3(0.0, 1.0, 0.0), frame()->transformOf(up));

  if (!noMove)
    frame()->setPosition(revolveAroundPoint() - (frame()->orientation()*q).rotate(frame()->coordinatesOf(revolveAroundPoint())));

  frame()->rotate(q);

  // Useful in fly mode to keep the horizontal direction.
  frame()->updateFlyUpVector();
}

/*! Sets the orientation() of the Camera using polar coordinates.

 \p theta rotates the Camera around its Y axis, and \e then \p phi rotates it around its X axis.
 The polar coordinates are defined in the world coordinates system: \p theta = \p phi = 0 means
 that the Camera is directed towards the world Z axis. Both angles are expressed in radians.

 See also setUpVector(). The position() of the Camera is unchanged, you may want to call showEntireScene()
 after this method to move the Camera.

 This method can be useful to create Quicktime VR panoramic sequences, see the
 QGLViewer::saveSnapshot() documentation for details. */
void Camera::setOrientation(float theta, float phi)
{
  glm::vec3 axis(0.0, 1.0, 0.0);
  const Quaternion rot1(axis, theta);
  axis = glm::vec3(-cos(theta), 0., sin(theta));
  const Quaternion rot2(axis, phi);
  setOrientation(rot1 * rot2);
}

/*! Sets the Camera orientation(), defined in the world coordinate system. */
void Camera::setOrientation(const Quaternion& q)
{
  frame()->setOrientation(q);
  frame()->updateFlyUpVector();
}

/*! Rotates the Camera so that its viewDirection() is \p direction (defined in the world coordinate
 system).

 The Camera position() is not modified. The Camera is rotated so that the horizon (defined by its
 upVector()) is preserved. See also lookAt() and setUpVector(). */
void Camera::setViewDirection(const glm::vec3& direction)
{
  if (glm::dot(direction, direction) < 1E-10)
    return;

  glm::vec3 xAxis = glm::cross(direction, upVector());
  if (glm::dot(xAxis, xAxis) < 1E-10)
    {
      // target is aligned with upVector, this means a rotation around X axis
      // X axis is then unchanged, let's keep it !
      xAxis = frame()->inverseTransformOf(glm::vec3(1.0, 0.0, 0.0));
    }

  Quaternion q;
  q.setFromRotatedBasis(xAxis, glm::cross(xAxis, direction), -direction);
  frame()->setOrientationWithConstraint(q);
}

// Compute a 3 by 3 determinant.
static float det(float m00,float m01,float m02,
                 float m10,float m11,float m12,
                 float m20,float m21,float m22)
{
  return m00*m11*m22 + m01*m12*m20 + m02*m10*m21 - m20*m11*m02 - m10*m01*m22 - m00*m21*m12;
}

// Computes the index of element [i][j] in a \c float matrix[3][4].
static inline unsigned int ind(unsigned int i, unsigned int j)
{
  return (i*4+j);
}


/*! Sets the Camera's position() and orientation() from an OpenGL ModelView matrix.

This enables a Camera initialisation from an other OpenGL application. \p modelView is a 16 GLdouble
vector representing a valid OpenGL ModelView matrix, such as one can get using:
\code
GLdouble mvm[16];
glGetDoublev(GL_MODELVIEW_MATRIX, mvm);
myCamera->setFromModelViewMatrix(mvm);
\endcode

After this method has been called, getModelViewMatrix() returns a matrix equivalent to \p
modelView.

Only the orientation() and position() of the Camera are modified.

\note If you defined your matrix as \c GLdouble \c mvm[4][4], pass \c &(mvm[0][0]) as a
parameter. */
void Camera::setFromModelViewMatrix(const double* const modelViewMatrix)
{
  // Get upper left (rotation) matrix
  double upperLeft[3][3];
  for (int i=0; i<3; ++i)
    for (int j=0; j<3; ++j)
      upperLeft[i][j] = modelViewMatrix[i*4+j];

  // Transform upperLeft into the associated Quaternion
  Quaternion q;
  q.setFromRotationMatrix(upperLeft);

  setOrientation(q);
  setPosition(-q.rotate(glm::vec3(modelViewMatrix[12], modelViewMatrix[13], modelViewMatrix[14])));
}

/*! Defines the Camera position(), orientation() and fieldOfView() from a projection matrix.

 \p matrix has to be given in the format used by vision algorithm. It has 3 lines and 4 columns. It
 transforms a point from the world homogeneous coordinate system (4 coordinates: \c sx, \c sy, \c sz
 and \c s) into a point in the screen homogeneous coordinate system (3 coordinates: \c sx, \c sy,
 and \c s, where \c x and \c y are the pixel coordinates on the screen).

 Its three lines correspond to the homogeneous coordinates of the normals to the planes x=0, y=0 and
 z=0, defined in the Camera coordinate system.

 The elements of the matrix are ordered in line major order: you can call \c
 setFromProjectionMatrix(&(matrix[0][0])) if you defined your matrix as a \c float \c matrix[3][4].

 \attention Passing the result of getProjectionMatrix() or getModelViewMatrix() to this method is
 not possible (purposefully incompatible matrix dimensions). \p matrix is more likely to be the
 product of these two matrices, without the last line.

 Use setFromModelViewMatrix() to set position() and orientation() from a \c GL_MODELVIEW matrix.
 fieldOfView() can also be retrieved from a \e perspective \c GL_PROJECTION matrix using 2.0 *
 atan(1.0/projectionMatrix[5]).

 This code was written by Sylvain Paris. */
void Camera::setFromProjectionMatrix(const float matrix[12])
{
  // The 3 lines of the matrix are the normals to the planes x=0, y=0, z=0
  // in the camera CS. As we normalize them, we do not need the 4th coordinate.
  glm::vec3 line_0(matrix[ind(0,0)],matrix[ind(0,1)],matrix[ind(0,2)]);
  glm::vec3 line_1(matrix[ind(1,0)],matrix[ind(1,1)],matrix[ind(1,2)]);
  glm::vec3 line_2(matrix[ind(2,0)],matrix[ind(2,1)],matrix[ind(2,2)]);

  line_0 = glm::normalize(line_0);
  line_1 = glm::normalize(line_1);
  line_2 = glm::normalize(line_2);

  // The camera position is at (0,0,0) in the camera CS so it is the
  // intersection of the 3 planes. It can be seen as the kernel
  // of the 3x4 projection matrix. We calculate it through 4 dimensional
  // vectorial product. We go directly into 3D that is to say we directly
  // divide the first 3 coordinates by the 4th one.

  // We derive the 4 dimensional vectorial product formula from the
  // computation of a 4x4 determinant that is developped according to
  // its 4th column. This implies some 3x3 determinants.
  const glm::vec3 cam_pos = glm::vec3(det(matrix[ind(0,1)],matrix[ind(0,2)],matrix[ind(0,3)],
                              matrix[ind(1,1)],matrix[ind(1,2)],matrix[ind(1,3)],
                              matrix[ind(2,1)],matrix[ind(2,2)],matrix[ind(2,3)]),

                           -det(matrix[ind(0,0)],matrix[ind(0,2)],matrix[ind(0,3)],
                                matrix[ind(1,0)],matrix[ind(1,2)],matrix[ind(1,3)],
                                matrix[ind(2,0)],matrix[ind(2,2)],matrix[ind(2,3)]),

                           det(matrix[ind(0,0)],matrix[ind(0,1)],matrix[ind(0,3)],
                               matrix[ind(1,0)],matrix[ind(1,1)],matrix[ind(1,3)],
                               matrix[ind(2,0)],matrix[ind(2,1)],matrix[ind(2,3)])) /

    (-det(matrix[ind(0,0)],matrix[ind(0,1)],matrix[ind(0,2)],
          matrix[ind(1,0)],matrix[ind(1,1)],matrix[ind(1,2)],
          matrix[ind(2,0)],matrix[ind(2,1)],matrix[ind(2,2)]));

  // We compute the rotation matrix column by column.

  // GL Z axis is front facing.
  glm::vec3 column_2 = -line_2;

  // X-axis is almost like line_0 but should be orthogonal to the Z axis.
  glm::vec3 column_0 = glm::cross (glm::cross(column_2,line_0),column_2);
  column_0 = glm::normalize(column_0);

  // Y-axis is almost like line_1 but should be orthogonal to the Z axis.
  // Moreover line_1 is downward oriented as the screen CS.
  glm::vec3 column_1 = -1.f* (glm::cross (glm::cross(column_2,line_1),column_2));
  column_1 = glm::normalize(column_1);

  double rot[3][3];
  rot[0][0] = column_0[0];
  rot[1][0] = column_0[1];
  rot[2][0] = column_0[2];

  rot[0][1] = column_1[0];
  rot[1][1] = column_1[1];
  rot[2][1] = column_1[2];

  rot[0][2] = column_2[0];
  rot[1][2] = column_2[1];
  rot[2][2] = column_2[2];

  // We compute the field of view

  // line_1^column_0 -> vector of intersection line between
  // y_screen=0 and x_camera=0 plane.
  // column_2*(...)  -> cos of the angle between Z vector et y_screen=0 plane
  // * 2 -> field of view = 2 * half angle

  // We need some intermediate values.
  glm::vec3 dummy = glm::cross(line_1,column_0);
  dummy = glm::normalize(dummy);
  float fov = acos(glm::dot(column_2, dummy)) * 2.0;

  // We set the camera.
  Quaternion q;
  q.setFromRotationMatrix(rot);
  setOrientation(q);
  setPosition(cam_pos);
  setFieldOfView(fov);
}


/*
        // persp : projectionMatrix_[0]  = f/aspectRatio();
void Camera::setFromProjectionMatrix(const GLdouble* projectionMatrix)
{
  QString message;
  if ((fabs(projectionMatrix[1]) > 1E-3) ||
      (fabs(projectionMatrix[2]) > 1E-3) ||
      (fabs(projectionMatrix[3]) > 1E-3) ||
      (fabs(projectionMatrix[4]) > 1E-3) ||
      (fabs(projectionMatrix[6]) > 1E-3) ||
      (fabs(projectionMatrix[7]) > 1E-3) ||
      (fabs(projectionMatrix[8]) > 1E-3) ||
      (fabs(projectionMatrix[9]) > 1E-3))
    message = "Non null coefficient in projection matrix - Aborting";
  else
    if ((fabs(projectionMatrix[11]+1.0) < 1E-5) && (fabs(projectionMatrix[15]) < 1E-5))
      {
        if (projectionMatrix[5] < 1E-4)
          message="Negative field of view in Camera::setFromProjectionMatrix";
        else
          setType(Camera::PERSPECTIVE);
      }
    else
      if ((fabs(projectionMatrix[11]) < 1E-5) && (fabs(projectionMatrix[15]-1.0) < 1E-5))
        setType(Camera::ORTHOGRAPHIC);
      else
        message = "Unable to determine camera type in setFromProjectionMatrix - Aborting";

  if (!message.isEmpty())
    {
      qWarning(message);
      return;
    }

  switch (type())
    {
    case Camera::PERSPECTIVE:
      {
        setFieldOfView(2.0 * atan(1.0/projectionMatrix[5]));
        const float far = projectionMatrix[14] / (2.0 * (1.0 + projectionMatrix[10]));
        const float near = (projectionMatrix[10]+1.0) / (projectionMatrix[10]-1.0) * far;
        setSceneRadius((far-near)/2.0);
        setSceneCenter(position() + (near + sceneRadius())*viewDirection());
        break;
      }
    case Camera::ORTHOGRAPHIC:
      {
        GLdouble w, h;
        getOrthoWidthHeight(w,h);
        projectionMatrix_[0]  = 1.0/w;
        projectionMatrix_[5]  = 1.0/h;
        projectionMatrix_[10] = -2.0/(ZFar - ZNear);
        projectionMatrix_[11] = 0.0;
        projectionMatrix_[14] = -(ZFar + ZNear)/(ZFar - ZNear);
        projectionMatrix_[15] = 1.0;
        // same as glOrtho( -w, w, -h, h, zNear(), zFar() );
        break;
      }
    }
}
*/

///////////////////////// Camera to world transform ///////////////////////

/*! Same as cameraCoordinatesOf(), but with \c float[3] parameters (\p src and \p res may be identical pointers). */
void Camera::getCameraCoordinatesOf(const float src[3], float res[3]) const
{
  glm::vec3 r = cameraCoordinatesOf(glm::vec3(src[0],src[1],src[2]));
  for (int i=0; i<3; ++i)
    res[i] = r[i];
}

/*! Same as worldCoordinatesOf(), but with \c float[3] parameters (\p src and \p res may be identical pointers). */
void Camera::getWorldCoordinatesOf(const float src[3], float res[3]) const
{
  glm::vec3 r = worldCoordinatesOf(glm::vec3(src[0],src[1],src[2]));
  for (int i=0; i<3; ++i)
    res[i] = r[i];
}

/*! Fills \p viewport with the Camera OpenGL viewport.

This method is mainly used in conjunction with \c gluProject, which requires such a viewport.
Returned values are (0, screenHeight(), screenWidth(), - screenHeight()), so that the origin is
located in the \e upper left corner of the window (Qt style coordinate system). */
void Camera::getViewport(int viewport[4]) const
{
  viewport[0] = 0;
  viewport[1] = screenHeight();
  viewport[2] = screenWidth();
  viewport[3] = -screenHeight();
}

/*! Returns the screen projected coordinates of a point \p src defined in the \p frame coordinate
 system.

 When \p frame in \c NULL (default), \p src is expressed in the world coordinate system.

 The x and y coordinates of the returned Vec are expressed in pixel, (0,0) being the \e upper left
 corner of the window. The z coordinate ranges between 0.0 (near plane) and 1.0 (excluded, far
 plane). See the \c gluProject man page for details.

 unprojectedCoordinatesOf() performs the inverse transformation.

 See the <a href="../examples/screenCoordSystem.html">screenCoordSystem example</a>.

 This method only uses the intrinsic Camera parameters (see getModelViewMatrix(),
 getProjectionMatrix() and getViewport()) and is completely independent of the OpenGL \c
 GL_MODELVIEW, \c GL_PROJECTION and viewport matrices. You can hence define a virtual Camera and use
 this method to compute projections out of a classical rendering context.

 \attention However, if your Camera is not attached to a QGLViewer (used for offscreen computations
 for instance), make sure the Camera matrices are updated before calling this method. Call
 computeModelViewMatrix() and computeProjectionMatrix() to do so.

 If you call this method several times with no change in the matrices, consider precomputing the
 projection times modelview matrix to save computation time if required (\c P x \c M in the \c
 gluProject man page).

 Here is the code corresponding to what this method does (kindly submitted by Robert W. Kuhn) :
 \code
 Vec project(Vec point)
 {
        GLint    Viewport[4];
        GLdouble Projection[16], Modelview[16];
        GLdouble matrix[16];

        // Precomputation begin
        glGetIntegerv(GL_VIEWPORT         , Viewport);
        glGetDoublev (GL_MODELVIEW_MATRIX , Modelview);
        glGetDoublev (GL_PROJECTION_MATRIX, Projection);

        for (unsigned short m=0; m<4; ++m)
        {
                for (unsigned short l=0; l<4; ++l)
                {
                        double sum = 0.0;
                        for (unsigned short k=0; k<4; ++k)
                                sum += Projection[l+4*k]*Modelview[k+4*m];
                        matrix[l+4*m] = sum;
                }
        }
        // Precomputation end

        GLdouble v[4], vs[4];
        v[0]=point[0]; v[1]=point[1]; v[2]=point[2]; v[3]=1.0;

        vs[0]=matrix[0 ]*v[0] + matrix[4 ]*v[1] + matrix[8 ]*v[2] + matrix[12 ]*v[3];
        vs[1]=matrix[1 ]*v[0] + matrix[5 ]*v[1] + matrix[9 ]*v[2] + matrix[13 ]*v[3];
        vs[2]=matrix[2 ]*v[0] + matrix[6 ]*v[1] + matrix[10]*v[2] + matrix[14 ]*v[3];
        vs[3]=matrix[3 ]*v[0] + matrix[7 ]*v[1] + matrix[11]*v[2] + matrix[15 ]*v[3];

        vs[0] /= vs[3];
        vs[1] /= vs[3];
        vs[2] /= vs[3];

        vs[0] = vs[0] * 0.5 + 0.5;
        vs[1] = vs[1] * 0.5 + 0.5;
        vs[2] = vs[2] * 0.5 + 0.5;

        vs[0] = vs[0] * Viewport[2] + Viewport[0];
        vs[1] = vs[1] * Viewport[3] + Viewport[1];

        return Vec(vs[0], Viewport[3]-vs[1], vs[2]);
  }
 \endcode
 */
glm::vec3 Camera::projectedCoordinatesOf(const glm::vec3& src, const Frame* frame) const
{

  //double x,y,z;
  int viewport[4];
  getViewport(viewport);
  glm::mat4 mv = getModelViewMatrix();
  glm::mat4 proj = getProjectionMatrix();
  glm::mat4 mvp = proj*mv;

  if (frame)
    {
      glm::vec3 tmp = frame->inverseCoordinatesOf(src);
      //gluProject(tmp.x,tmp.y,tmp.z, modelViewMatrix_, projectionMatrix_, viewport,  &x,&y,&z);
      glm::vec4 res = mvp*glm::vec4(tmp, 1.0);
      res /= res.w;
      res = (res * 0.5) + 0.5;
      res.x = res.x*viewport[2]+viewport[0];
      res.y = res.y*viewport[3]+viewport[1];

      ///@todo, cleanup !!! david : I have changed vp - y to -vp -y
      return glm::vec3(res.x,-viewport[3]-res.y,res.z);
    }
  else {
    //gluProject(src.x,src.y,src.z, modelViewMatrix_, projectionMatrix_, viewport,  &x,&y,&z);
      glm::vec4 srcVec = glm::vec4(src.x, src.y, src.z, 1.0);
      glm::vec4 tmp = mvp*srcVec;
      using namespace util;
      tmp/=tmp.w;
      glm::vec3 res = glm::vec3(tmp.x, tmp.y, tmp.z);
      res = (res * 0.5f) + 0.5f;
      res.x = res.x*viewport[2]+viewport[0];
      res.y = res.y*viewport[3]+viewport[1];
      ///@todo, cleanup !!! david : I have changed vp - y to -vp -y
      return glm::vec3(res.x,-viewport[3]-res.y,res.z);
    }

}

/*! Returns the world unprojected coordinates of a point \p src defined in the screen coordinate
 system.

 The \p src.x and \p src.y input values are expressed in pixels, (0,0) being the \e upper left corner
 of the window. \p src.z is a depth value ranging in [0..1[ (near and far plane respectively). See
 the \c gluUnProject man page for details.

 The result is expressed in the \p frame coordinate system. When \p frame is \c NULL (default), the
 result is expressed in the world coordinates system. The possible \p frame Frame::referenceFrame()
 are taken into account.

 projectedCoordinatesOf() performs the inverse transformation.

 This method only uses the intrinsic Camera parameters (see getModelViewMatrix(),
 getProjectionMatrix() and getViewport()) and is completely independent of the OpenGL \c
 GL_MODELVIEW, \c GL_PROJECTION and viewport matrices. You can hence define a virtual Camera and use
 this method to compute un-projections out of a classical rendering context.

 \attention However, if your Camera is not attached to a QGLViewer (used for offscreen computations
 for instance), make sure the Camera matrices are updated before calling this method (use
 computeModelViewMatrix(), computeProjectionMatrix()). See also setScreenWidthAndHeight().

 This method is not computationally optimized. If you call it several times with no change in the
 matrices, you should buffer the entire inverse projection matrix (modelview, projection and then
 viewport) to speed-up the queries. See the \c gluUnProject man page for details. */
glm::vec3 Camera::unprojectedCoordinatesOf(const glm::vec3& src, const Frame* frame) const
{
    //double x,y,z;
    int viewport[4];
    getViewport(viewport);
    glm::mat4 mv = getModelViewMatrix();
    glm::mat4 proj = getProjectionMatrix();
    glm::mat4 mvp = proj*mv;
    glm::mat4 invmvp = glm::inverse(mvp);
    glm::vec4 from( ((src.x-viewport[0])/viewport[2]) *2.0f -1.0f,
                    ((src.y-viewport[1])/viewport[3]) *2.0f -1.0f,
                    2.0f*src.z-1.0f,
                    1.0f
                   );
    glm::vec4 res = invmvp*from;
    if (res.w != 0.f) {
        res /= res.w;
        if (frame) {
          return frame->coordinatesOf(glm::vec3(res.x, res.y, res.z));
        } else {
          return glm::vec3(res.x, res.y, res.z);
        }
    } else {
        return sceneCenter();
    }
}

/*! Same as projectedCoordinatesOf(), but with \c float parameters (\p src and \p res can be identical pointers). */
void Camera::getProjectedCoordinatesOf(const float src[3], float res[3], const Frame* frame) const
{
  glm::vec3 r = projectedCoordinatesOf(glm::vec3(src[0],src[1],src[2]), frame);
  for (int i=0; i<3; ++i)
    res[i] = r[i];
}

/*! Same as unprojectedCoordinatesOf(), but with \c float parameters (\p src and \p res can be identical pointers). */
void Camera::getUnprojectedCoordinatesOf(const float src[3], float res[3], const Frame* frame) const
{
  glm::vec3 r = unprojectedCoordinatesOf(glm::vec3(src[0],src[1],src[2]), frame);
  for (int i=0; i<3; ++i)
    res[i] = r[i];
}

#if 0
/////////////////////////////////////  KFI /////////////////////////////////////////

/*! Returns the KeyFrameInterpolator that defines the Camera path number \p i.

If path \p i is not defined for this index, the method returns a \c NULL pointer. */
KeyFrameInterpolator* Camera::keyFrameInterpolator(int i) const
{
  if (kfi_.contains(i))
    return kfi_[i];
  else
    return NULL;
}

/*! Sets the KeyFrameInterpolator that defines the Camera path of index \p i.

 The previous keyFrameInterpolator() is lost and should be deleted by the calling method if
 needed.

 The KeyFrameInterpolator::interpolated() signal of \p kfi probably needs to be connected to the
 Camera's associated QGLViewer::updateGL() slot, so that when the Camera position is interpolated
 using \p kfi, every interpolation step updates the display:
 \code
 myViewer.camera()->deletePath(3);
 myViewer.camera()->setKeyFrameInterpolator(3, myKeyFrameInterpolator);
 connect(myKeyFrameInterpolator, SIGNAL(interpolated()), myViewer, SLOT(updateGL());
 \endcode

 \note These connections are done automatically when a Camera is attached to a QGLViewer, or when a
 new KeyFrameInterpolator is defined using the QGLViewer::addKeyFrameKeyboardModifiers() and
 QGLViewer::pathKey() (default is Alt+F[1-12]). See the <a href="../keyboard.html">keyboard page</a>
 for details. */
void Camera::setKeyFrameInterpolator(int i, KeyFrameInterpolator* const kfi)
{
  if (kfi)
    kfi_[i] = kfi;
  else
    kfi_.remove(i);
}

/*! Adds the current Camera position() and orientation() as a keyFrame to the path number \p i.

This method can also be used if you simply want to save a Camera point of view (a path made of a
single keyFrame). Use playPath() to make the Camera play the keyFrame path (resp. restore
the point of view). Use deletePath() to clear the path.

The default keyboard shortcut for this method is Alt+F[1-12]. Set QGLViewer::pathKey() and
QGLViewer::addKeyFrameKeyboardModifiers().

If you use directly this method and the keyFrameInterpolator(i) does not exist, a new one is
created. Its KeyFrameInterpolator::interpolated() signal should then be connected to the
QGLViewer::updateGL() slot (see setKeyFrameInterpolator()). */
void Camera::addKeyFrameToPath(int i)
{
  if (!kfi_.contains(i))
    setKeyFrameInterpolator(i, new KeyFrameInterpolator(frame()));

  kfi_[i]->addKeyFrame(*(frame()));
}

/*! Makes the Camera follow the path of keyFrameInterpolator() number \p i.

 If the interpolation is started, it stops it instead.

 This method silently ignores undefined (empty) paths (see keyFrameInterpolator()).

 The default keyboard shortcut for this method is F[1-12]. Set QGLViewer::pathKey() and
 QGLViewer::playPathKeyboardModifiers(). */
void Camera::playPath(int i)
{
  if (kfi_.contains(i)) {
    if (kfi_[i]->interpolationIsStarted())
      kfi_[i]->stopInterpolation();
    else
      kfi_[i]->startInterpolation();
  }
}

/*! Resets the path of the keyFrameInterpolator() number \p i.

If this path is \e not being played (see playPath() and
KeyFrameInterpolator::interpolationIsStarted()), resets it to its starting position (see
KeyFrameInterpolator::resetInterpolation()). If the path is played, simply stops interpolation. */
void Camera::resetPath(int i)
{
  if (kfi_.contains(i)) {
    if ((kfi_[i]->interpolationIsStarted()))
      kfi_[i]->stopInterpolation();
    else
      {
                kfi_[i]->resetInterpolation();
                kfi_[i]->interpolateAtTime(kfi_[i]->interpolationTime());
      }
  }
}

/*! Deletes the keyFrameInterpolator() of index \p i.

Disconnect the keyFrameInterpolator() KeyFrameInterpolator::interpolated() signal before deleting the
keyFrameInterpolator() if needed:
\code
disconnect(camera()->keyFrameInterpolator(i), SIGNAL(interpolated()), this, SLOT(updateGL()));
camera()->deletePath(i);
\endcode */
void Camera::deletePath(int i)
{
  if (kfi_.contains(i))
    {
      kfi_[i]->stopInterpolation();
      delete kfi_[i];
      kfi_.remove(i);
    }
}

/*! Draws all the Camera paths defined by the keyFrameInterpolator().

 Simply calls KeyFrameInterpolator::drawPath() for all the defined paths. The path color is the
 current \c glColor().

 \attention The OpenGL state is modified by this method: see KeyFrameInterpolator::drawPath(). */
void Camera::drawAllPaths()
{
  for (QMap<int, KeyFrameInterpolator*>::ConstIterator it = kfi_.begin(), end=kfi_.end(); it != end; ++it)
#if QT_VERSION >= 0x040000
    (it.value())->drawPath(3, 5, sceneRadius());
#else
    (it.data())->drawPath(3, 5, sceneRadius());
#endif
}
#endif

#if 0
/*! Gives the coefficients of a 3D half-line passing through the Camera eye and pixel (x,y).

 The origin of the half line (eye position) is stored in \p orig, while \p dir contains the properly
 oriented and normalized direction of the half line.

 \p x and \p y are expressed in Qt format (origin in the upper left corner). Use screenHeight() - y
 to convert to OpenGL units.

 This method is useful for analytical intersection in a selection method.

 See the <a href="../examples/select.html">select example</a> for an illustration. */
void Camera::convertClickToLine(const QPoint& pixel, Vec& orig, Vec& dir) const
{
  switch (type())
    {
    case Camera::PERSPECTIVE:
      orig = position();
      dir = Vec( ((2.0 * pixel.x() / screenWidth()) - 1.0) * tan(fieldOfView()/2.0) * aspectRatio(),
                 ((2.0 * (screenHeight()-pixel.y()) / screenHeight()) - 1.0) * tan(fieldOfView()/2.0),
                 -1.0 );
      dir = worldCoordinatesOf(dir) - orig;
      dir.normalize();
      break;

    case Camera::ORTHOGRAPHIC:
      {
        GLdouble w,h;
        getOrthoWidthHeight(w,h);
        orig = Vec((2.0 * pixel.x() / screenWidth() - 1.0)*w, -(2.0 * pixel.y() / screenHeight() - 1.0)*h, 0.0);
        orig = worldCoordinatesOf(orig);
        dir = viewDirection();
        break;
      }
    }
}
#endif

#if 0
/*! This method has been deprecated in libQGLViewer version 2.2.0 */
void Camera::drawCamera(float, float, float)
{
  qWarning("drawCamera is deprecated. Use Camera::draw() instead.");
}


/*! Draws a representation of the Camera in the 3D world.

The near and far planes are drawn as quads, the frustum is drawn using lines and the camera up
vector is represented by an arrow to disambiguate the drawing. See the
<a href="../examples/standardCamera.html">standardCamera example</a> for an illustration.

Note that the current \c glColor and \c glPolygonMode are used to draw the near and far planes. See
the <a href="../examples/frustumCulling.html">frustumCulling example</a> for an example of
semi-transparent plane drawing. Similarly, the current \c glLineWidth and \c glColor is used to draw
the frustum outline.

When \p drawFarPlane is \c false, only the near plane is drawn. \p scale can be used to scale the
drawing: a value of 1.0 (default) will draw the Camera's frustum at its actual size.

This method assumes that the \c glMatrixMode is \c GL_MODELVIEW and that the current ModelView
matrix corresponds to the world coordinate system (as it is at the beginning of QGLViewer::draw()).
The Camera is then correctly positioned and orientated.

\note The drawing of a QGLViewer's own QGLViewer::camera() should not be visible, but may create
artefacts due to numerical imprecisions. */
void Camera::draw(bool drawFarPlane, float scale) const
{
  glPushMatrix();
  glMultMatrixd(frame()->worldMatrix());

  // 0 is the upper left coordinates of the near corner, 1 for the far one
  Vec points[2];

  points[0].z = scale * zNear();
  points[1].z = scale * zFar();

  switch (type())
    {
    case Camera::PERSPECTIVE:
      {
        points[0].y = points[0].z * tan(fieldOfView()/2.0);
        points[0].x = points[0].y * aspectRatio();

        const float ratio = points[1].z / points[0].z;

        points[1].y = ratio * points[0].y;
        points[1].x = ratio * points[0].x;
        break;
      }
    case Camera::ORTHOGRAPHIC:
      {
        GLdouble hw, hh;
        getOrthoWidthHeight(hw, hh);
        points[0].x = points[1].x = scale * float(hw);
        points[0].y = points[1].y = scale * float(hh);
        break;
      }
    }

  const int farIndex = drawFarPlane?1:0;

  // Near and (optionally) far plane(s)
  glBegin(GL_QUADS);
  for (int i=farIndex; i>=0; --i)
    {
      glNormal3f(0.0, 0.0, (i==0)?1.0:-1.0);
      glVertex3f( points[i].x,  points[i].y, -points[i].z);
      glVertex3f(-points[i].x,  points[i].y, -points[i].z);
      glVertex3f(-points[i].x, -points[i].y, -points[i].z);
      glVertex3f( points[i].x, -points[i].y, -points[i].z);
    }
  glEnd();

  // Up arrow
  const float arrowHeight    = 1.5f * points[0].y;
  const float baseHeight     = 1.2f * points[0].y;
  const float arrowHalfWidth = 0.5f * points[0].x;
  const float baseHalfWidth  = 0.3f * points[0].x;

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // Base
  glBegin(GL_QUADS);
  glVertex3f(-baseHalfWidth, points[0].y, -points[0].z);
  glVertex3f( baseHalfWidth, points[0].y, -points[0].z);
  glVertex3f( baseHalfWidth, baseHeight,  -points[0].z);
  glVertex3f(-baseHalfWidth, baseHeight,  -points[0].z);
  glEnd();

  // Arrow
  glBegin(GL_TRIANGLES);
  glVertex3f( 0.0f,           arrowHeight, -points[0].z);
  glVertex3f(-arrowHalfWidth, baseHeight,  -points[0].z);
  glVertex3f( arrowHalfWidth, baseHeight,  -points[0].z);
  glEnd();

  // Frustum lines
  switch (type())
    {
    case Camera::PERSPECTIVE :
      glBegin(GL_LINES);
      glVertex3f(0.0f, 0.0f, 0.0f);
      glVertex3f( points[farIndex].x,  points[farIndex].y, -points[farIndex].z);
      glVertex3f(0.0f, 0.0f, 0.0f);
      glVertex3f(-points[farIndex].x,  points[farIndex].y, -points[farIndex].z);
      glVertex3f(0.0f, 0.0f, 0.0f);
      glVertex3f(-points[farIndex].x, -points[farIndex].y, -points[farIndex].z);
      glVertex3f(0.0f, 0.0f, 0.0f);
      glVertex3f( points[farIndex].x, -points[farIndex].y, -points[farIndex].z);
      glEnd();
      break;
    case Camera::ORTHOGRAPHIC :
      if (drawFarPlane)
        {
          glBegin(GL_LINES);
          glVertex3f( points[0].x,  points[0].y, -points[0].z);
          glVertex3f( points[1].x,  points[1].y, -points[1].z);
          glVertex3f(-points[0].x,  points[0].y, -points[0].z);
          glVertex3f(-points[1].x,  points[1].y, -points[1].z);
          glVertex3f(-points[0].x, -points[0].y, -points[0].z);
          glVertex3f(-points[1].x, -points[1].y, -points[1].z);
          glVertex3f( points[0].x, -points[0].y, -points[0].z);
          glVertex3f( points[1].x, -points[1].y, -points[1].z);
          glEnd();
        }
    }

  glPopMatrix();
}

#endif

/*! Returns the 6 plane equations of the Camera frustum.

The six 4-component vectors of \p coef respectively correspond to the left, right, near, far, top
and bottom Camera frustum planes. Each vector holds a plane equation of the form:
\code
a*x + b*y + c*z + d = 0
\endcode
where \c a, \c b, \c c and \c d are the 4 components of each vector, in that order.

See the <a href="../examples/frustumCulling.html">frustumCulling example</a> for an application.

This format is compatible with the \c glClipPlane() function. One camera frustum plane can hence be
applied in an other viewer to visualize the culling results:
\code
 // Retrieve plance equations
 GLdouble coef[6][4];
 mainViewer->camera()->getFrustumPlanesCoefficients(coef);

 // These two additional clipping planes (which must have been enabled)
 // will reproduce the mainViewer's near and far clipping.
 glClipPlane(GL_CLIP_PLANE0, coef[2]);
 glClipPlane(GL_CLIP_PLANE1, coef[3]);
\endcode */
void Camera::getFrustumPlanesCoefficients(GLdouble coef[6][4]) const
{
  // Computed once and for all
  const glm::vec3 pos          = position();
  const glm::vec3 viewDir      = viewDirection();
  const glm::vec3 up           = upVector();
  const glm::vec3 right        = rightVector();
  const float posViewDir = glm::dot(pos, viewDir);

  static glm::vec3 normal[6];
  static GLdouble dist[6];

  switch (type())
    {
    case Camera::PERSPECTIVE :
      {
        const float hhfov = horizontalFieldOfView() / 2.0;
        const float chhfov = cos(hhfov);
        const float shhfov = sin(hhfov);
        normal[0] = - shhfov * viewDir;
        normal[1] = normal[0] + chhfov * right;
        normal[0] = normal[0] - chhfov * right;

        normal[2] = -viewDir;
        normal[3] =  viewDir;

        const float hfov = fieldOfView() / 2.0;
        const float chfov = cos(hfov);
        const float shfov = sin(hfov);
        normal[4] = - shfov * viewDir;
        normal[5] = normal[4] - chfov * up;
        normal[4] = normal[4] + chfov * up;

        for (int i=0; i<2; ++i)
          dist[i] = glm::dot(pos, normal[i]);
        for (int j=4; j<6; ++j)
          dist[j] = glm::dot(pos, normal[j]);

        // Natural equations are:
        // dist[0,1,4,5] = pos * normal[0,1,4,5];
        // dist[2] = (pos + zNear() * viewDir) * normal[2];
        // dist[3] = (pos + zFar()  * viewDir) * normal[3];

        // 2 times less computations using expanded/merged equations. Dir vectors are normalized.
        const float posRightCosHH = chhfov * glm::dot(pos, right);
        dist[0] = -shhfov * posViewDir;
        dist[1] = dist[0] + posRightCosHH;
        dist[0] = dist[0] - posRightCosHH;
        const float posUpCosH = chfov * glm::dot(pos, up);
        dist[4] = - shfov * posViewDir;
        dist[5] = dist[4] - posUpCosH;
        dist[4] = dist[4] + posUpCosH;

        break;
      }
    case Camera::ORTHOGRAPHIC :
      normal[0] = -right;
      normal[1] =  right;
      normal[4] =  up;
      normal[5] = -up;

      double hw, hh;
      getOrthoWidthHeight(hw, hh);
      dist[0] = glm::dot( (pos - (float)hw * right), normal[0] );
      dist[1] = glm::dot((pos + (float)hw * right), normal[1]);
      dist[4] = glm::dot((pos + (float)hh * up), normal[4]);
      dist[5] = glm::dot((pos - (float)hh * up), normal[5]);
      break;
    }

  // Front and far planes are identical for both camera types.
  normal[2] = -viewDir;
  normal[3] =  viewDir;
  dist[2] = -posViewDir - zNear();
  dist[3] =  posViewDir + zFar();

  for (int i=0; i<6; ++i)
    {
      coef[i][0] = GLdouble(normal[i].x);
      coef[i][1] = GLdouble(normal[i].y);
      coef[i][2] = GLdouble(normal[i].z);
      coef[i][3] = dist[i];
    }
}


glm::mat4 Camera::getProjectionMatrix() const{
    static float m[16];
    getProjectionMatrix(m);
    return glm::mat4( m[0], m[1], m[2], m[3],
                      m[4], m[5], m[6], m[7],
                      m[8], m[9], m[10], m[11],
                      m[12], m[13], m[14], m[15] );
}

glm::mat4 Camera::getModelViewMatrix() const{
    static float m[16];
    getModelViewMatrix(m);
    return glm::mat4( m[0], m[1], m[2], m[3],
                      m[4], m[5], m[6], m[7],
                      m[8], m[9], m[10], m[11],
                      m[12], m[13], m[14], m[15] );
}


} // namespace vortex
