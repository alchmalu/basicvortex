#include "renderer.h"
#include "visitors.h"

using namespace std;

namespace vortex {
#if 0
/*
 * Default solid renderer
 */
void DefaultSolidRenderer::setViewport(int width, int height)
{
    Renderer::setViewport(width, height);

    mFBO->setSize(width, height);
    if (mDepthMap->getId() != 0)
        mDepthMap->deleteGL();
    mDepthMap->initGL(GL_DEPTH_COMPONENT24, mWidth, mHeight, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    if (mColorTex->getId() != 0)
        mColorTex->deleteGL();
    mColorTex->initGL(GL_RGBA8, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);


    mFBO->bind();
    mFBO->attachTexture(GL_COLOR_ATTACHMENT0, mColorTex);
    mFBO->attachTexture(GL_DEPTH_ATTACHMENT, mDepthMap);
    mFBO->check();
    mFBO->unbind();

    //  mDepthMap->useMipMap(GL_NEAREST, GL_NEAREST);
}

void DefaultSolidRenderer::render(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
    if (mSceneManager->sceneGraph()) {
        //      glAssert(glEnable(GL_TEXTURE_RECTANGLE));
        //      glAssert( glEnable(GL_TEXTURE_2D) );

//        mFBO->bind();

        mFBO->useAsTarget(mWidth, mHeight);
        glClearColor(0., 1., 0., 1.);
        mFBO->clear(FBO::ALL);// to clear all attached texture
        glClearColor(0., 0., 0., 1.);

//transform light
        LightTransformVisitor transformLightAction(mSceneManager->sceneGraph());
        SceneGraph::PreOrderVisitor transformLight(mSceneManager->sceneGraph(), transformLightAction);
        transformLight.go(modelViewMatrix, projectionMatrix);

        // draw the scene
        ShaderLoop loop;
        DefaultLoopBuilder build(mSceneManager->sceneGraph(), &loop);
        SceneGraph::PostOrderVisitor builder(mSceneManager->sceneGraph(), build);
        builder.go(modelViewMatrix, projectionMatrix);
        loop.draw();
//        GLubyte pixels[4*4*4];
//        glReadPixels(mWidth/2, mHeight/2, 4, 4, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
//        for(int i =0; i<4*4*4; i+=4){
//            std::cout <<int(pixels[i]) << " ";
//            std::cout <<int(pixels[i]) << " ";
//            std::cout <<int(pixels[i]) << " ";
//            std::cout <<int(pixels[i]) << " " << std::endl;
//        }
        mFBO->unbind();

        ShaderProgram * shader = mSceneManager->getAsset()->getShaderProgram(mQuadShaderId);
        shader->bind();
        shader->setUniformTexture("color", mColorTex, 0);
        mScreenQuad->draw();
    }
}

/*
 * Default Wireframe renderer
 */

void DefaultWireframeRenderer::setViewport(int width, int height)
{
    Renderer::setViewport(width, height);
}

void DefaultWireframeRenderer::render(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
    if (mSceneManager->sceneGraph()) {
        //build render loop
        ShaderLoop loop;
        DefaultLoopBuilder loopBuilder(mSceneManager->sceneGraph(), &loop);
        SceneGraph::PostOrderVisitor builder(mSceneManager->sceneGraph(), loopBuilder);
        builder.go(modelViewMatrix, projectionMatrix);

        // setup wireframe mode : fill depth buffer
        glAssert(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
        glAssert(glEnable(GL_POLYGON_OFFSET_FILL));
        glAssert(glPolygonOffset(4.0f, 1.1f));

        // draw the scene to fill depth buffer
        loop.draw();

        // setup wireframe mode : line drawing
        glAssert(glDisable(GL_POLYGON_OFFSET_FILL));
        glAssert(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
        GLDepthFuncSave depthSave;
        glAssert(glDepthFunc(GL_LEQUAL));
        glAssert(glDepthMask(GL_FALSE));
        glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

        // draw the scene
        loop.draw();

        // cleanup wireframe mode
        glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        glAssert(glDepthMask(GL_TRUE));

        if (mShowBboxes) {
            BBoxRenderer bboxShower;
            SceneGraph::PostOrderVisitor render = SceneGraph::PostOrderVisitor(mSceneManager->sceneGraph(), bboxShower);
            glm::mat4x4 normal = glm::transpose(glm::inverse(modelViewMatrix));
            glm::mat4x4 MVP = projectionMatrix * modelViewMatrix;
            mBboxRenderState.bind(modelViewMatrix, projectionMatrix, MVP, normal);
            render.go(modelViewMatrix, projectionMatrix);
            mBboxRenderState.unbind();
        }
    }
}
#endif
/*
 * Picking renderer
 */

void SelectionRenderer::setViewport(int width, int height)
{
    Renderer::setViewport(width, height);

    mFBO.setSize(width, height);
    mVerticeIdFBO.setSize(width, height);

    if (mIdMap->getId() != 0)
        mIdMap->deleteGL();
    mIdMap->initGL(GL_RGBA8 , mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    mFBO.bind();
    //  mFBO.detachTexture( GL_COLOR_ATTACHMENT0 );
    mFBO.attachTexture(GL_COLOR_ATTACHMENT0, mIdMap);
    mFBO.attachRBO(GL_DEPTH_ATTACHMENT);
    mFBO.check();
    mFBO.unbind();

    if (mVerticeIdMap->getId() != 0)
        mVerticeIdMap->deleteGL();
    mVerticeIdMap->initGL(GL_RGBA8 , mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    mVerticeIdFBO.bind();
    //  mVerticeIdFBO.detachTexture( GL_COLOR_ATTACHMENT0 );
    mVerticeIdFBO.attachTexture(GL_COLOR_ATTACHMENT0, mVerticeIdMap);
    mVerticeIdFBO.attachRBO(GL_DEPTH_ATTACHMENT);
    mVerticeIdFBO.check();
    mVerticeIdFBO.unbind();
}

void SelectionRenderer::pick(int x, int y)
{

    // activate fbo
    mFBO.useAsTarget(mWidth, mHeight);
    mFBO.clear(FBO::ALL);// to clear all attached texture
    // draw ids
    if (!mMode) {
        //      mGroup->drawMeshId();
    } else {
        //      mGroup->drawFaceId();
        if (!mPrimitive) {
            mVerticeIdFBO.useAsTarget(mWidth, mHeight);
            mVerticeIdFBO.clear(FBO::ALL);

            //          mGroup->drawVerticeId();

            mVerticeIdFBO.unbind();
            mFBO.bind();
        }
    }
    // read texel (x,y)
    GLubyte data[4];
    glAssert(glReadBuffer(GL_COLOR_ATTACHMENT0));
//#ifndef __APPLE__
    glAssert(glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE));
//#endif
    glAssert(glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data));
    mFBO.unbind();
    std::cerr << "Texel = " << int(data[0]) << " " << int(data[1]) << " " << int(data[2]) << std::endl;
    int id = data[0] * 255 * 255 + data[1] * 255 + data[2];
    // pick mesh/vertices/faces
    if (!mMode) {  // object mode => select a mesh
        //      std::cerr << "Mesh picked (" << id << ") : " << mGroup->getMeshName( id ) << std::endl;
    } else if (!mPrimitive) {  // edit mode vertices => select vertices
        if (id) {
            mVerticeIdFBO.bind();
            glAssert(glReadBuffer(GL_COLOR_ATTACHMENT0));
            glAssert(glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE));
            glAssert(glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data));
            mVerticeIdFBO.unbind();
            std::cerr << "Texel = " << int(data[0]) << " " << int(data[1]) << " " << int(data[2]) << std::endl;
            //          int vertice = data[0];
            //          mGroup->setSelectedVertice( id, vertice );
        } else {
            std::cerr << "No vertice picked" << std::endl;
        }
    } else { // edit mode faces => select faces
        if (id) {
            std::cerr << "Face picked : " << id << std::endl;
            //          mGroup->setSelectedFace( id );
        } else {
            std::cerr << "No face picked" << std::endl;
        }
    }
}

void SelectionRenderer::render(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
    if (mSceneManager->sceneGraph()) {
        mModelView = modelViewMatrix;
        mProjection = projectionMatrix;

        //      mGroup->clear();
        //      mGroup->setProjectionMatrix( mProjection );
        //      DefaultShaderGroupBuilder build( mGroup );
        //      SceneGraph::PostOrderVisitor builder( mSceneManager->sceneGraph(), build );
        //      builder.go( modelViewMatrix, projectionMatrix );

        //glAssert(glEnable(GL_TEXTURE_RECTANGLE));

        if (!mMode) {  // object mode : select and/or /scale/rotate meshes => here draw meshes id and the whole scene as in solid renderer
            // @todo render mesh silhouette

            // draw the scene
            //          mGroup->draw();
            mSceneManager->sceneGraph()->draw(modelViewMatrix, projectionMatrix);

        } else if (!mPrimitive) {  // edit mode vertice : select and/or move/rotate vertices => here draw selected mesh vertices id and the whole scene with the selected mesh wireframed
            // TODO
            // @todo draw selected mesh vertices id

            // draw the scene without the selected mesh
            //          mGroup->drawAllButSelected();

            // draw the selected mesh wireframed
            glAssert(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
            glAssert(glEnable(GL_POLYGON_OFFSET_FILL));
            glAssert(glPolygonOffset(4.0f, 1.1f));
            //          mGroup->drawSelectedMesh( mModelView, mProjection );
            glAssert(glDisable(GL_POLYGON_OFFSET_FILL));
            glAssert(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

            GLDepthFuncSave depthSave;
            glAssert(glDepthFunc(GL_LEQUAL));
            glAssert(glDepthMask(GL_FALSE));
            glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            //          mGroup->drawSelectedMesh( mModelView, mProjection );
            glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            glAssert(glDepthMask(GL_TRUE));

            //glAssert(glEnable(GL_POINT_SMOOTH));
            glAssert(glPointSize(1.1));
            glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_POINT));
            //          mGroup->drawSelectedMesh( mModelView, mProjection, true );
            glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

            // draw the selected vertice if there is one
            //          mGroup->drawSelectedVertice( mModelView, mProjection );


        } else { // edit mode face: select and/or move/scale/rotate faces => here draw selected mesh faces id and the whole scene with the selected mesh half-wireframed
            // TODO
            // @todo draw selected mesh faces id

            // draw the scene without the selected mesh
            //          mGroup->drawAllButSelected();

            // draw the selected mesh half-wireframed
            glAssert(glEnable(GL_POLYGON_OFFSET_FILL));
            glAssert(glPolygonOffset(4.0f, 1.1f));
            //          mGroup->drawSelectedMesh( mModelView, mProjection, true );
            glAssert(glDisable(GL_POLYGON_OFFSET_FILL));

            GLDepthFuncSave depthSave;
            glAssert(glDepthFunc(GL_LEQUAL));
            glAssert(glDepthMask(GL_FALSE));
            glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            //          mGroup->drawSelectedMesh( mModelView, mProjection );
            glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            glAssert(glDepthMask(GL_TRUE));

            // draw the selected face if there is one
            //          mGroup->drawSelectedFace( mModelView, mProjection );
        }

        /*
        if (mShowBboxes) { // can be called with the 2 modes
            BBoxRenderer bboxShower;
            SceneGraph::PostOrderVisitor render = SceneGraph::PostOrderVisitor(mSceneManager->sceneGraph(), bboxShower);
            glm::mat4x4 normal = glm::transpose(glm::inverse(modelViewMatrix));
            glm::mat4x4 MVP = projectionMatrix * modelViewMatrix;
            mBboxRenderState.bind(modelViewMatrix, projectionMatrix, MVP, normal);
            render.go(modelViewMatrix, projectionMatrix);
            mBboxRenderState.unbind();
        }
        */
    }

}

} // vortex
