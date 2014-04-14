#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H
#include <map>

#include "drawable.h"
#include "bindable.h"
#include "mesh.h"
#include "shaderobject.h"
#include "material.h"

#define LAZYTRANSFORM
namespace vortex {
/**
 *  @defgroup RenderingLoops Rendering loop abstraction.
 *  A rendering loop is an abstraction that allows to minimize state changes whe drawing a scene.
 *  A rendering loop is a map that associate keys (bindable rendering state) to draw calls. A draw call may be itself a rendering loop.
 *  @todo Write full documentation of this abstraction
 */

/**
 *  @defgroup RenderingLoopsKeys Keys for sorting draw calls in renderloops.
 *  @ingroup RenderingLoops
 *  @todo Write full documentation of this abstraction
 */

/**
 *  This class allows to sort draw call by material
 *  @ingroup RenderingLoopsKeys
 *  @todo Write full documentation of this abstraction
 */
class MaterialState : public Bindable {
public:

    MaterialState(Material *mat, ShaderProgram *shader);


    /**
      * Bind the MaterialState to the current rendering pipeline.
      */
    void bind() const;

    /**
      * Bind a Material to the shader shaderId.
      * @todo optimize this by binding only the needed subset of material parameters ...
      */
    static void bind(Material *mat, ShaderProgram * shader) ;

    /**
      * Strict order on material.
      */
    bool operator< (const MaterialState &other) const;

private:
    Material *mMaterial;
    ShaderProgram *mShader;

};


/**
 *  This class allows to sort draw call by Transformation matrices
 *  @ingroup RenderingLoopsKeys
 *  @todo Write full documentation of this abstraction
 */
class TransformState : public Bindable {
public:

    TransformState(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix, ShaderProgram *prog);

    /**
      * Bind the TransformState to the current rendering pipeline.
      */
    void bind() const;

    /**
      * Bind the TransformState to the current rendering pipeline.
      */
    void bind(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix) const;

    /**
      * Strict order on matrices.
      */
    bool operator< (const TransformState &other) const;

private:
    glm::mat4x4 mModelView, mProjection, mMVP, mNormal;
    ShaderProgram *mProgram;
};

<<<<<<< HEAD
/**
 *  @ingroup RenderingLoopsKeys
 *  @todo Write full documentation of this abstraction
 */
class PickingState : public Bindable {
public:
    PickingState(int id);
    PickingState(int id, const glm::mat4x4 &modelView, const glm::mat4x4 &projection, ShaderProgram *prog);

    void bind() const;
    void bind(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix) const;
    static void bind(int id, const glm::mat4x4 &mvp, ShaderProgram *shader);

    bool operator< (const PickingState &other) const;
    bool operator== (const PickingState &other) const;

    static glm::vec3 idToColor(int id);
    static int colorToId(glm::vec3 color);

private:
    int mId;
    glm::mat4x4 mModelView, mProjection;
    ShaderProgram *mProgram;
};

=======
>>>>>>> 0a49773ec341312329eebc1d2aff39cd6d5a0756
/**
 *  Generic vector (based on std::vector) with a draw method. Drawing a vector results in drawing all its elements.
 *  @ingroup RenderingLoops
 */
template< class E >
class DrawableVector : public Drawable, public std::vector< E > {
public:

    void draw() {
        for (typename DrawableVector< E >::iterator it = this->begin(); it != this->end(); ++it) {
            (*it).draw();
        }
    }
};

/**
 *  Specialisation of a DrawableVector to Mesh::MeshPtr
 *  @ingroup RenderingLoops
 */
template<>
class DrawableVector < Mesh::MeshPtr > : public Drawable, public std::vector< Mesh::MeshPtr > {
public:

    void draw() {
        for (DrawableVector< Mesh::MeshPtr >::iterator it = this->begin(); it != this->end(); ++it) {
            (*it)->draw();
        }
    }
};

/**
 *  Specialisation of Drawable pointer
 *  @ingroup RenderingLoops
 */
class DrawableMeshPtr : public Drawable {
public:
    DrawableMeshPtr() : Drawable(), mMesh(NULL) {}
    DrawableMeshPtr(Mesh::MeshPtr mesh) : Drawable(), mMesh(mesh) {}

    void draw() {
        mMesh->draw();
    }

    Mesh::MeshPtr mMesh;
};

/**
 *  Generic map (based on std::map) with a draw method. Drawing a map results in drawing all its elements.
 *  A map may be drawn with or without parameters.
 *  ACTUAL LIMITATION : when rendering loops are nested, only the outer loop may have parameters
 *  @ingroup RenderingLoops
 *  @todo generalize the paramaters to all loops.
 */
template< typename Key, typename Child >
class DrawableMap : public Drawable, public std::map< Key, Child > {
public:

    void draw() {
        for (typename DrawableMap< Key, Child >::iterator it = this->begin(); it != this->end(); ++it) {
            it->first.bind();
            it->second.draw();
        }
    }

    void draw(const GlobalParameter& parameters) {
        for (typename DrawableMap< Key, Child >::iterator it = this->begin(); it != this->end(); ++it) {
            it->first.bind(parameters);
            it->second.draw();
        }
    }

#ifdef LAZYTRANSFORM
    /** Draw with matrix completion : must multiply the matrices by modelview and projection before binding parameters.
     *      This method must be restricted to drawable maps with
     *
     **/
    void draw(const GlobalParameter& parameters,const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix) {
        for (typename DrawableMap< Key, Child >::iterator it = this->begin(); it != this->end(); ++it) {
            it->first.bind(parameters);
            it->second.draw(modelviewMatrix, projectionMatrix);
        }
    }

    void draw(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix) {
        for (typename DrawableMap< Key, Child >::iterator it = this->begin(); it != this->end(); ++it) {
            it->first.bind();
            it->second.draw(modelviewMatrix, projectionMatrix);
        }
    }
#endif

};


/**
  *  Draw meshes ordered by materials
  *  @ingroup RenderingLoops
  */
typedef DrawableMap< MaterialState, DrawableVector<Mesh::MeshPtr> > MaterialLoop;

/**
  *  Draw MaterialLoop ordered by transformations
  *  @ingroup RenderingLoops
  */
typedef DrawableMap< TransformState, MaterialLoop > TransformLoop;
/**
  *  Draw TransformLoop ordered by shaders
  *  @ingroup RenderingLoops
  */
typedef DrawableMap< ShaderProgram, TransformLoop > ShaderLoop;

/**
  *  Draw meshes ordered by transformations (without materials)
  *  @ingroup RenderingLoops
  */
typedef DrawableMap< TransformState, DrawableVector<Mesh::MeshPtr> > LightTransformLoop;

/**
  * Draw LightTransformLoop ordered by shaders
  *  @ingroup RenderingLoops
  */
typedef DrawableMap< ShaderProgram, LightTransformLoop > LightShaderLoop;

/**
  *  Draw meshes for picking
  *  @ingroup RenderingLoops
  */
typedef DrawableMap<PickingState, Mesh::MeshPtr> PickingLoop;


#ifdef LAZYTRANSFORM
template<>
void DrawableMap<TransformState, MaterialLoop>::draw(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix);

template<>
void DrawableMap<TransformState, DrawableVector<Mesh::MeshPtr> >::draw(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix);
#endif

} // vortex

#endif // GROUP_H
