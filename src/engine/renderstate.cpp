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

#include "renderstate.h"
#include "assetmanager.h"

namespace vortex {

/**
 * @todo optimize this using a uniform class ... read Uniform Buffer Object specificatio
 */
void RenderState::bind(const glm::mat4x4 &modelviewmatrix, const glm::mat4x4 &projectionmatrix, const glm::mat4x4 &modelviewprojectionmatrix, const glm::mat4x4 &normalMatrix)
{
    glAssert(glUseProgram(mShaderProgram->id()));
    GLint loc;
    glAssert(loc = glGetUniformLocation(mShaderProgram->id(), "modelViewMatrix"));
    glAssert(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelviewmatrix)));

    glAssert(glUniformMatrix4fv(glGetUniformLocation(mShaderProgram->id(), "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionmatrix)));
    glAssert(glUniformMatrix4fv(glGetUniformLocation(mShaderProgram->id(), "MVP"), 1, GL_FALSE, glm::value_ptr(modelviewprojectionmatrix)));
    glAssert(glUniformMatrix4fv(glGetUniformLocation(mShaderProgram->id(), "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix)));

    if (mMaterial) {
        Texture * tex = mMaterial->getTexture(Material::TEXTURE_DIFFUSE);
        if (tex) {
            tex->bind(GL_TEXTURE0);
            glAssert(glUniform1i(glGetUniformLocation(mShaderProgram->id(), "map_diffuse"), 0));
        }
    }
}

/**
 * @todo do we realy need to do something here ?
 */
void RenderState::unbind()
{
}


}
