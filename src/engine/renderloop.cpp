#include "renderloop.h"

namespace vortex {


//-------------------------------------------------------------------------------
// MaterialState
//-------------------------------------------------------------------------------


MaterialState:: MaterialState(Material *mat, ShaderProgram *shaderId) : Bindable(), mMaterial(mat), mShader(shaderId) {}


void MaterialState::bind() const{ // to be completed
    if (mMaterial) {
        bind(mMaterial, mShader);
    }

}

void MaterialState::bind(Material *mat, ShaderProgram *shader) {
    glAssert(glUniform1f(glGetUniformLocation(shader->id() , "opacityLevel"), (-1.f)));

    Texture * tex = mat->getTexture(Material::TEXTURE_DIFFUSE);
    if (tex) {
        shader->setUniformTexture("map_diffuse", tex);
        shader->setUniformTexture("map_ambient", tex);
        shader->setUniform("texturedAmbient", 1);
    } else {
        shader->setUniform("texturedAmbient", 0);
    }

    tex = mat->getTexture(Material::TEXTURE_HEIGHT);
    if (tex) {
        shader->setUniformTexture("map_normals", tex);
    }
    tex = mat->getTexture(Material::TEXTURE_NORMALS);
    if (tex) {
        shader->setUniformTexture("map_normals", tex);
    }
    tex = mat->getTexture(Material::TEXTURE_SPECULAR);
    if (tex) {
        shader->setUniformTexture("map_specular", tex);
    }
    tex = mat->getTexture(Material::TEXTURE_AMBIENT);
    if (tex) {
        shader->setUniformTexture("map_ambient", tex);
        shader->setUniform("texturedAmbient", 1);
    }
    tex = mat->getTexture(Material::TEXTURE_OPACITY);
    if (tex) {
        shader->setUniform("opacityLevel", 0.01f);
        shader->setUniformTexture("map_opacity", tex);
    }

    shader->setUniform("Kd", mat->getDiffuseColor());
    shader->setUniform("Ka", mat->getAmbientColor());
    shader->setUniform("Ks", mat->getSpecularColor());
    shader->setUniform("Ns", mat->getShininess());


}

bool MaterialState::operator< (const MaterialState &other) const {
    // todo find a better comparison
    return (mMaterial->getName() < other.mMaterial->getName());
}


//-------------------------------------------------------------------------------
// TransformState
//-------------------------------------------------------------------------------

TransformState::TransformState(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix, ShaderProgram *prog) : Bindable(),
        mModelView(modelviewMatrix), mProjection(projectionMatrix), mProgram(prog) {
        mMVP = mProjection * mModelView;
        mNormal = glm::transpose(glm::inverse(mModelView));
    }

void TransformState::bind() const {

    /**********     METHOD WITH UBO     **********/
    mProgram->setTransform(mModelView, mProjection, mMVP, mNormal);
    /*********************************************/

}

void TransformState::bind(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix) const {
    glm::mat4x4 mv = modelviewMatrix*mModelView;
    glm::mat4x4 mvp = projectionMatrix * mv;
    glm::mat4x4 mnormal = glm::transpose(glm::inverse(mv));
    /**********     METHOD WITH UBO     **********/
    mProgram->setTransform(mv, projectionMatrix, mvp, mnormal);
    /*********************************************/

}

bool TransformState::operator< (const TransformState &other) const {
    // todo find a better comparison
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (mModelView[i][j] < other.mModelView[i][j]) {
                return true;
            } else if (mModelView[i][j] > other.mModelView[i][j]) {
                return false;
            }
        }
    }
    return false;
}

//-------------------------------------------------------------------------------
// PickingState
//-------------------------------------------------------------------------------

PickingState::PickingState(int id) :
    Bindable(), mId(id), mModelView(NULL), mProjection(NULL), mProgram(NULL) {
}

PickingState::PickingState(int id, const glm::mat4x4 &modelView, const glm::mat4x4 &projection, ShaderProgram *prog) :
    Bindable(), mId(id), mModelView(modelView), mProjection(projection), mProgram(prog) {
}

void PickingState::bind() const {
    bind(mId, mModelView*mProjection, mProgram);
}

void PickingState::bind(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix) const {
    bind(mId, projectionMatrix * (modelviewMatrix * mModelView), mProgram);
}

void PickingState::bind(int id, const glm::mat4x4 &mvp, ShaderProgram *shader) {
    glm::vec4 color(idToColor(id), 1);
    shader->setUniform("MVP", mvp);
    shader->setUniform("color", color);
}

bool PickingState::operator< (const PickingState &other) const {
    return mId < other.mId;
}

bool PickingState::operator ==(const PickingState &other) const {
    return mId == other.mId;
}

glm::vec3 PickingState::idToColor(int id) {
    float r = ((id / 65536) % 256) / 255.0;
    float g = ((id / 256) % 256) / 255.0;
    float b = (id % 256) / 255.0;

    //std::cout << r*255 << " " << g*255 << " " << b*255 << std::endl;
    return glm::vec3(r, g, b);
}

int PickingState::colorToId(glm::vec3 color) {
    return color.x * 65536 + color.y * 256 + color.z;
}

#ifdef LAZYTRANSFORM

template<>
void DrawableMap<TransformState, MaterialLoop>::draw(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix) {

    for (typename DrawableMap< TransformState, MaterialLoop >::iterator it = this->begin(); it != this->end(); ++it) {
        it->first.bind(modelviewMatrix, projectionMatrix);
        it->second.draw();
    }
}

template<>
void DrawableMap<TransformState, DrawableVector<Mesh::MeshPtr> >::draw(const glm::mat4x4 &modelviewMatrix, const glm::mat4x4 &projectionMatrix) {

    for (typename DrawableMap< TransformState, DrawableVector<Mesh::MeshPtr> >::iterator it = this->begin(); it != this->end(); ++it) {
        glCheckError();
        it->first.bind(modelviewMatrix, projectionMatrix);
        it->second.draw();
    }
}

#endif

} // vortex


