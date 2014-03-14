/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include "material.h"
#include "shaderobject.h"

namespace vortex {


class AssetManager;

/**
 * A renderstate object is associated with each leaf node Mesh.
 * As a leaf node may have several meshes, we will store a vector of RenderState object in the node, indexed the same way as the Meshes.
 */
class RenderState {
public:
    typedef RenderState* RenderStatePtr;

    /**
     * Default constructor
     */
    RenderState() : mMaterial(NULL), mShaderProgram(NULL) {}
    virtual ~RenderState() {}

    /**
     * Set the Material to be used for drawing the Mesh the RenderState is associated to.
     *
     * @param material The Material to be used.
     */
    void setMaterial(Material *material) {
        mMaterial = material;
    }

    /**
     * @return the Material used to draw the Mesh.
     */
    Material * getMaterial() {
        return mMaterial;
    }

    /**
     * Set the ShaderProgram to be used to draw the Mesh.
     *
     * @param shaderProgram The ShaderProgram to be used.
     */
    void setShaderProgram(ShaderProgram *shaderProgram) {
        mShaderProgram = shaderProgram;
    }

    /**
     * @return The ShaderProgram used to draw the Mesh the RenderState is associated to.
     */
    ShaderProgram * getShaderProgram() {
        return mShaderProgram;
    }

    /**
     * Bind the RenderState : specify the ShaderProgram to the OpenGL API and pass all uniform variables to it.
     *
     * @param modelviewmatrix The ModelView transformation matrix to be used to draw the Mesh the RenderState is associated to.
     * @param projectionmatrix The Projection transformation matrix to be used to draw the Mesh the RenderState is associated to.
     * @param modelviewprojectionmatrix The ModelViewProjection transformation matrix to be used to draw the Mesh the RenderState is associated to.
     * @param normalMatrix The Normal transformation matrix to be used to draw the Mesh the RenderState is associated to.
     *
     * This method is called when drawing the scene from the SceneGraph.
     */
    void bind(const glm::mat4x4 &modelviewmatrix, const glm::mat4x4 &projectionmatrix, const glm::mat4x4 &modelviewprojectionmatrix, const glm::mat4x4 &normalMatrix);

    /**
     * Unbind the RenderState
     */
    void unbind();

    /**
     * Set the AssetManager to used for resources setting.
     *
     * @param manager The AssetManager of the scene containing the Mesh the RenderState is associated to.
     */
    void setAssetManager(AssetManager *manager) {
        mResourcesManager = manager;
    }

    /**
     * Save/restore the ShaderProgram in/to use.
     *
     * @param save Specify whether to save ( if true ) or restore ( if false ) the ShaderProgram.
     */
    void saveProgram(bool save) {
        if (save) { // save
            mOldShaderProgram = mShaderProgram;
        } else {// restore
            mShaderProgram = mOldShaderProgram;
        }
    }

private:
    AssetManager *mResourcesManager;

    Material *mMaterial;
    ShaderProgram *mShaderProgram;

    ShaderProgram *mOldShaderProgram;
};

}
#endif // RENDERSTATE_H
