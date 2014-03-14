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

#include "constraint.h"
#include "frame.h"
#include "camera.h"

 namespace vortex {




////////////////////////////////////////////////////////////////////////////////
//                                  Constraint                                //
////////////////////////////////////////////////////////////////////////////////

/*! Default constructor.

translationConstraintType() and rotationConstraintType() are set to AxisPlaneConstraint::FREE.
translationConstraintDirection() and rotationConstraintDirection() are set to (0,0,0). */
AxisPlaneConstraint::AxisPlaneConstraint()
  : translationConstraintType_(FREE), rotationConstraintType_(FREE)
{
  // Do not use set since setRotationConstraintType needs a read.
}

/*! Simply calls setTranslationConstraintType() and setTranslationConstraintDirection(). */
void AxisPlaneConstraint::setTranslationConstraint(Type type, const glm::vec3& direction)
{
  setTranslationConstraintType(type);
  setTranslationConstraintDirection(direction);
}

/*! Defines the translationConstraintDirection(). The coordinate system where \p direction is expressed depends on your class implementation. */
void AxisPlaneConstraint::setTranslationConstraintDirection(const glm::vec3& direction)
{
  if ((translationConstraintType()!=AxisPlaneConstraint::FREE) && (translationConstraintType()!=AxisPlaneConstraint::FORBIDDEN))
    {
      const float norm = glm::length(direction);
      if (norm < 1E-8)
        {
          //qWarning("AxisPlaneConstraint::setTranslationConstraintDir: null vector for translation constraint");
          translationConstraintType_ = AxisPlaneConstraint::FREE;
        }
      else
        translationConstraintDir_ = direction/norm;
    }
}

/*! Simply calls setRotationConstraintType() and setRotationConstraintDirection(). */
void AxisPlaneConstraint::setRotationConstraint(Type type, const glm::vec3& direction)
{
  setRotationConstraintType(type);
  setRotationConstraintDirection(direction);
}

/*! Defines the rotationConstraintDirection(). The coordinate system where \p direction is expressed depends on your class implementation. */
void AxisPlaneConstraint::setRotationConstraintDirection(const glm::vec3& direction)
{
  if ((rotationConstraintType()!=AxisPlaneConstraint::FREE) && (rotationConstraintType()!=AxisPlaneConstraint::FORBIDDEN))
    {
      float norm = glm::length(direction);
      if (norm < 1E-8)
        {
          //qWarning("AxisPlaneConstraint::setRotationConstraintDir: null vector for rotation constraint");
          rotationConstraintType_ = AxisPlaneConstraint::FREE;
        }
      else
        rotationConstraintDir_ = direction/norm;
    }
}

/*! Set the Type() of the rotationConstraintType(). Default is AxisPlaneConstraint::FREE.

 Depending on this value, the Frame will freely rotate (AxisPlaneConstraint::FREE), will only be able
 to rotate around an axis (AxisPlaneConstraint::AXIS), or will not able to rotate at all
 (AxisPlaneConstraint::FORBIDDEN).

 Use Frame::setOrientation() to define the orientation of the constrained Frame before it gets
 constrained.

 \attention An AxisPlaneConstraint::PLANE Type() is not meaningful for rotational constraints and
 will be ignored. */
void AxisPlaneConstraint::setRotationConstraintType(Type type)
{
  if (rotationConstraintType() == AxisPlaneConstraint::PLANE)
    {
      //qWarning("AxisPlaneConstraint::setRotationConstraintType: the PLANE type cannot be used for a rotation constraints");
      return;
    }

  rotationConstraintType_ = type;
}


////////////////////////////////////////////////////////////////////////////////
//                               LocalConstraint                              //
////////////////////////////////////////////////////////////////////////////////

/*! Depending on translationConstraintType(), constrain \p translation to be along an axis or
  limited to a plane defined in the Frame local coordinate system by
  translationConstraintDirection(). */
void LocalConstraint::constrainTranslation(glm::vec3& translation, Frame* const frame)
{
  glm::vec3 proj;
  switch (translationConstraintType())
    {
    case AxisPlaneConstraint::FREE:
      break;
    case AxisPlaneConstraint::PLANE:
      proj = frame->rotation().rotate(translationConstraintDirection());
      projectOnPlane(translation, proj);
      break;
    case AxisPlaneConstraint::AXIS:
      proj = frame->rotation().rotate(translationConstraintDirection());
      projectOnAxis(translation, proj);
      break;
    case AxisPlaneConstraint::FORBIDDEN:
      translation = glm::vec3(0.0, 0.0, 0.0);
      break;
    }
}

/*! When rotationConstraintType() is AxisPlaneConstraint::AXIS, constrain \p rotation to be a rotation
  around an axis whose direction is defined in the Frame local coordinate system by
  rotationConstraintDirection(). */
void LocalConstraint::constrainRotation(Quaternion& rotation, Frame* const)
{
  switch (rotationConstraintType())
    {
    case AxisPlaneConstraint::FREE:
      break;
    case AxisPlaneConstraint::PLANE:
      break;
    case AxisPlaneConstraint::AXIS:
      {
        glm::vec3 axis = rotationConstraintDirection();
        glm::vec3 quat = glm::vec3(rotation[0], rotation[1], rotation[2]);
        projectOnAxis(quat, axis);
        rotation = Quaternion(quat, 2.0*acos(rotation[3]));
      }
      break;
    case AxisPlaneConstraint::FORBIDDEN:
      rotation = Quaternion(); // identity
      break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//                               WorldConstraint                              //
////////////////////////////////////////////////////////////////////////////////

/*! Depending on translationConstraintType(), constrain \p translation to be along an axis or
  limited to a plane defined in the world coordinate system by
  translationConstraintDirection(). */
void WorldConstraint::constrainTranslation(glm::vec3& translation, Frame* const frame)
{
  glm::vec3 proj;
  switch (translationConstraintType())
    {
    case AxisPlaneConstraint::FREE:
      break;
    case AxisPlaneConstraint::PLANE:
      if (frame->referenceFrame())
        {
          proj = frame->referenceFrame()->transformOf(translationConstraintDirection());
          projectOnPlane(translation, proj);
        }
      else
        projectOnPlane(translation, translationConstraintDirection());
      break;
    case AxisPlaneConstraint::AXIS:
      if (frame->referenceFrame())
        {
          proj = frame->referenceFrame()->transformOf(translationConstraintDirection());
          projectOnAxis(translation, proj);
        }
      else
        projectOnAxis(translation, translationConstraintDirection());
      break;
    case AxisPlaneConstraint::FORBIDDEN:
      translation = glm::vec3(0.0, 0.0, 0.0);
      break;
    }
}

/*! When rotationConstraintType() is AxisPlaneConstraint::AXIS, constrain \p rotation to be a rotation
  around an axis whose direction is defined in the world coordinate system by
  rotationConstraintDirection(). */
void WorldConstraint::constrainRotation(Quaternion& rotation, Frame* const frame)
{
  switch (rotationConstraintType())
    {
    case AxisPlaneConstraint::FREE:
      break;
    case AxisPlaneConstraint::PLANE:
      break;
    case AxisPlaneConstraint::AXIS:
      {
        glm::vec3 quat(rotation[0], rotation[1], rotation[2]);
        glm::vec3 axis = frame->transformOf(rotationConstraintDirection());
        projectOnAxis(quat, axis);
        rotation = Quaternion(quat, 2.0*acos(rotation[3]));
        break;
      }
    case AxisPlaneConstraint::FORBIDDEN:
      rotation = Quaternion(); // identity
      break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//                               CameraConstraint                              //
////////////////////////////////////////////////////////////////////////////////

/*! Creates a CameraConstraint, whose constrained directions are defined in the \p camera coordinate
  system. */
CameraConstraint::CameraConstraint(const Camera* const camera)
  : AxisPlaneConstraint(), camera_(camera)
{}

/*! Depending on translationConstraintType(), constrain \p translation to be along an axis or
  limited to a plane defined in the camera() coordinate system by
  translationConstraintDirection(). */
void CameraConstraint::constrainTranslation(glm::vec3& translation, Frame* const frame)
{
  glm::vec3 proj;
  switch (translationConstraintType())
    {
    case AxisPlaneConstraint::FREE:
      break;
    case AxisPlaneConstraint::PLANE:
      proj = camera()->frame()->inverseTransformOf(translationConstraintDirection());
      if (frame->referenceFrame())
        proj = frame->referenceFrame()->transformOf(proj);
      projectOnPlane(translation, proj);
      break;
    case AxisPlaneConstraint::AXIS:
      proj = camera()->frame()->inverseTransformOf(translationConstraintDirection());
      if (frame->referenceFrame())
        proj = frame->referenceFrame()->transformOf(proj);
      projectOnAxis(translation, proj);
      break;
    case AxisPlaneConstraint::FORBIDDEN:
      translation = glm::vec3(0.0, 0.0, 0.0);
      break;
    }
}

/*! When rotationConstraintType() is AxisPlaneConstraint::AXIS, constrain \p rotation to be a rotation
  around an axis whose direction is defined in the camera() coordinate system by
  rotationConstraintDirection(). */
void CameraConstraint::constrainRotation(Quaternion& rotation, Frame* const frame)
{
  switch (rotationConstraintType())
    {
    case AxisPlaneConstraint::FREE:
      break;
    case AxisPlaneConstraint::PLANE:
      break;
    case AxisPlaneConstraint::AXIS:
      {
        glm::vec3 axis = frame->transformOf(camera()->frame()->inverseTransformOf(rotationConstraintDirection()));
        glm::vec3 quat = glm::vec3(rotation[0], rotation[1], rotation[2]);
        projectOnAxis(quat, axis);
        rotation = Quaternion(quat, 2.0*acos(rotation[3]));
      }
      break;
    case AxisPlaneConstraint::FORBIDDEN:
      rotation = Quaternion(); // identity
      break;
    }
}

} // namsepace vortex
