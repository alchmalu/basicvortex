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

#include "visitors.h"

namespace vortex {


void PrintNodeInfo::operator()(SceneGraph::Node* theNode)
{
    std::cerr << theNode->name() << " " << theNode->boundingBox() << std::endl;
}

/*
 * -------------------------------------------------------------------------------
 */
void BBoxBuilder::operator()(SceneGraph::Node *theNode)
{
    if (theNode->isLeaf()) {
       SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
       for (int i = 0; i < leafNode->nMeshes(); ++i){
           if ((*leafNode)[i]){
                if (! (*leafNode)[i]->boundingBox().isEmpty()){
                    leafNode->updateBbox((*leafNode)[i]->boundingBox().getTransformedBBox(leafNode->transformMatrix()));
                }
            }
       }
   } else {
       SceneGraph::InnerNode *innerNode = static_cast<SceneGraph::InnerNode *>(theNode);
       for (int i = 0; i < innerNode->nChilds(); ++i){
           if (! (*innerNode)[i]->boundingBox().isEmpty()){
                innerNode->updateBbox((*innerNode)[i]->boundingBox().getTransformedBBox(innerNode->transformMatrix()));
           }
       }
   }
}

void BBoxRenderer::operator()(SceneGraph::Node *theNode, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
//          leafNode->operator[] ( i )->drawBbox ( modelViewMatrix, projectionMatrix );
            leafNode->drawBbox(modelViewMatrix, projectionMatrix);
        }
    }
}

/*
 * -------------------------------------------------------------------------------
 */
/*ResourcesSetter::ResourcesSetter(AssetManager *resourcesManager) : mResourcesManager(resourcesManager)
{
}

void ResourcesSetter::operator()(SceneGraph::Node *theNode)
{
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            RenderState::RenderStatePtr stateObject = leafNode->getRenderState(i);
            stateObject->setAssetManager(mResourcesManager);
            // build a shaderConfiguration object from material
            ShaderConfiguration nodeShaderConfiguration;
            Material * nodeMaterial = stateObject->getMaterial();
            for (int j = 0; j < nodeMaterial->numTexture(); ++j) {
                nodeShaderConfiguration.addProperty(nodeMaterial->getTextureSemantic(j));
            }
            // get the shader associated with this configuration
            ShaderProgram *theNodeProgram = mResourcesManager->getShaderProgram(nodeShaderConfiguration);
            // associate the shader with the state object
            stateObject->setShaderProgram(theNodeProgram);
        }
    }
}
*/
/*
 * -------------------------------------------------------------------------------
 */
WireframeNodeRenderer::WireframeNodeRenderer()
{
}

void WireframeNodeRenderer::operator()(SceneGraph::Node *theNode, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
    if (theNode->isLeaf()) {
        glAssert(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
        glAssert(glEnable(GL_POLYGON_OFFSET_FILL));
        glAssert(glPolygonOffset(4.0f, 1.1f));
        theNode->draw(modelViewMatrix, projectionMatrix);
        glAssert(glDisable(GL_POLYGON_OFFSET_FILL));
        glAssert(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

        GLDepthFuncSave depthSave;
        glAssert(glDepthFunc(GL_LEQUAL));
        glAssert(glDepthMask(GL_FALSE));
        glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        theNode->draw(modelViewMatrix, projectionMatrix);
        glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        glAssert(glDepthMask(GL_TRUE));
    }
}

/*
 * -------------------------------------------------------------------------------
 */
DefaultLoopBuilder::DefaultLoopBuilder(SceneGraph *sceneGraph, ShaderLoop *loop) : mSceneGraph(sceneGraph), mLoop(loop)
{
}

void DefaultLoopBuilder::operator()(SceneGraph::Node *theNode, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
//    std::cerr << " -- DefaultLoopBuilder::operator()" << std::endl;
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        RenderState *state = NULL;
        ShaderProgram *prog = NULL;
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if ((*leafNode)[i]) {
            state = leafNode->getRenderState(i);
            prog = state->getShaderProgram();
            //@TODO check not needed data duplication, allocation ... !!! *prog is copied instead of linked ...
            (*mLoop)[ *prog ][ TransformState(modelViewMatrix, projectionMatrix, prog)]
            [ MaterialState(state->getMaterial(), prog)].push_back((*leafNode)[i]);
}
        }

    }
}
/*
 * -------------------------------------------------------------------------------
 */
ShaderLoopBuilder::ShaderLoopBuilder(AssetManager *resourcesManager, SceneGraph *sceneGraph, ShaderLoop *loop, std::string shaderName)
    : mResourcesManager(resourcesManager), mSceneGraph(sceneGraph), mLoop(loop), mPropertiesFilter(NULL)
{
    mShaderName = mResourcesManager->getShaderBasePath() +  shaderName;
}

void ShaderLoopBuilder::operator()(SceneGraph::Node *theNode, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
//    std::cerr << " -- ShaderLoopBuilder::operator()" << std::endl;
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        RenderState *state = NULL;
        ShaderProgram *prog = NULL;
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if ((*leafNode)[i]){
                state = leafNode->getRenderState(i);

            // build a shaderConfiguration object from material
            ShaderConfiguration nodeShaderConfiguration(ShaderConfiguration::DEFAULT, mShaderName);
            Material * nodeMaterial = state->getMaterial();
            for (int j = 0; j < nodeMaterial->numTexture(); ++j) {
                if ( !mPropertiesFilter || mPropertiesFilter->match(nodeMaterial->getTextureSemantic(j)) )
                    nodeShaderConfiguration.addProperty(nodeMaterial->getTextureSemanticString(j));
            }

            // get or generate the shader associated with this configuration
            prog = mResourcesManager->getShaderProgram(nodeShaderConfiguration);
            prog->setConfiguration(nodeShaderConfiguration);
            (*mLoop)[ *prog ][ TransformState(modelViewMatrix, projectionMatrix, prog)]
            [ MaterialState(nodeMaterial, prog)].push_back((*leafNode)[i]);
            }
        }
    }
}

/*
 * -------------------------------------------------------------------------------
 */

TransformLoopBuilder::TransformLoopBuilder(SceneGraph *sceneGraph, TransformLoop *loop, ShaderProgram *shader) : mSceneGraph(sceneGraph), mLoop(loop),
    mShader(shader)
{
}

void TransformLoopBuilder::operator()(SceneGraph::Node *theNode, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        RenderState *state = NULL;
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
if ((*leafNode)[i]) {
            state = leafNode->getRenderState(i);
            (*mLoop)[ TransformState(modelViewMatrix, projectionMatrix, mShader)]
            [ MaterialState(state->getMaterial(), mShader)].push_back((*leafNode)[i]);
}
        }

    }
}

/*
 * -------------------------------------------------------------------------------
 */

SetOfLoopBuilder::SetOfLoopBuilder(): SceneGraph::VisitorOperation() {
}

void SetOfLoopBuilder::operator()(SceneGraph::Node *theNode, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix){
//    std::cerr << "SetOfLoopBuilder::operator() -- (" << mBuilderSet.size() << ")" <<std::endl;
    for (std::deque<SceneGraph::VisitorOperation *>::iterator it=mBuilderSet.begin(); it!= mBuilderSet.end(); ++it)
        (*(*it))(theNode, modelViewMatrix, projectionMatrix);
}

void SetOfLoopBuilder::addLoopBuilder(SceneGraph::VisitorOperation *builder){
    mBuilderSet.push_back(builder);
}

/*
 * -------------------------------------------------------------------------------
 */

void LightTransformVisitor::operator()(SceneGraph::Node *theNode, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
    if (theNode->isLeaf()) {
        if(mSceneGraph->mLights.size()>0) {
            for(unsigned int i=0;i<mSceneGraph->mLights.size(); ++i){
                if(0 == mSceneGraph->mLights[i].mName.compare(theNode->name())){
                    mSceneGraph->mLights[i].mTransform = modelViewMatrix;
                }
            }

        }
    }
}



/*
 * -------------------------------------------------------------------------------
 */
// ShaderCreatorVisitor

ShaderBuilder::ShaderBuilder(vortex::AssetManager *resourcesManager, std::string shaderName, bool setDefault) :
    mResourcesManager(resourcesManager),
    mShaderName(shaderName),
    mSetAsDefault(setDefault), mPropertiesFilter(NULL){
    mShaderName = mResourcesManager->getShaderBasePath() + shaderName;
    //std::cerr << "Shader builder visitor : " << mShaderName << std::endl;
}

void ShaderBuilder::operator()(SceneGraph::Node *theNode)
{
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if ((*leafNode)[i]) {
                RenderState::RenderStatePtr stateObject = leafNode->getRenderState(i);
                stateObject->setAssetManager(mResourcesManager);

            // build a shaderConfiguration object from material
            ///@todo make ShaderConfiguration::DEFAULT configurable

                ShaderConfiguration nodeShaderConfiguration(ShaderConfiguration::DEFAULT, mShaderName);
                Material * nodeMaterial = stateObject->getMaterial();
                for (int j = 0; j < nodeMaterial->numTexture(); ++j) {
                    if ( mPropertiesFilter ) {
                        if (mPropertiesFilter->match(nodeMaterial->getTextureSemantic(j)) )
                            nodeShaderConfiguration.addProperty(nodeMaterial->getTextureSemanticString(j));
                    } else {
                        nodeShaderConfiguration.addProperty(nodeMaterial->getTextureSemanticString(j));
                    }
                }
                // get or generate the shader associated with this configuration
                ShaderProgram *theNodeProgram = mResourcesManager->getShaderProgram(nodeShaderConfiguration);
                if (mSetAsDefault){
                    // associate the shader with the state object
                    stateObject->setShaderProgram(theNodeProgram);
                }
            }
        }
    }
}

/*
 * -------------------------------------------------------------------------------
 */
ShaderSetter::ShaderSetter(ShaderProgram *prog) : mProg(prog)
{
}

void ShaderSetter::operator()(SceneGraph::Node *theNode)
{
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if ((*leafNode)[i]) {
                leafNode->getRenderState(i)->setShaderProgram(mProg);
            }
        }
    }
}
ShaderSaver::ShaderSaver(bool save) : mSave(save)
{
}

void ShaderSaver::operator()(SceneGraph::Node *theNode)
{
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if ((*leafNode)[i]) {
                leafNode->getRenderState(i)->saveProgram(mSave);
            }
        }
    }
}

/*
 * -------------------------------------------------------------------------------
 */
GraphStatistic::GraphStatistic() : leafCount(0), innerCount(0), meshCount(0), faceCount(0), verticesCount(0){

}

void GraphStatistic::operator()(SceneGraph::Node *theNode){
    if (theNode->isLeaf()) {
        leafCount+=1;
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        meshCount += leafNode->nMeshes();
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if ((*leafNode)[i]) {
                Mesh::MeshPtr theMesh= (*leafNode)[i];
                faceCount += theMesh->numIndices()/3;
                verticesCount += theMesh->numvertices();
            }
        }
    } else {
       innerCount+=1;
    }
}

std::ostream & operator << (std::ostream &out, const GraphStatistic &stat){
    out << "Inner nodes : " << stat.innerCount << " / leaf nodes : " << stat.leafCount;
    out << " --- Meshes : " << stat.meshCount << " - Faces : " << stat.faceCount << " - Vertex : " << stat.verticesCount;
    return out;
}


// MeshUpdater

/**
 * Update if the mesh is animated
 * @author Alexandre Bonhomme
 * @date 02/05/2012
 */
void MeshUpdater::operator()(SceneGraph::Node *theNode) {

    if (theNode->isLeaf()) {

        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);

        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if (mTime >=0){
                // update the mesh
                (*leafNode)[i]->update();
            } else {
               (*leafNode)[i]->reset();
            }
        }
    }
}

} // namespace vortex
