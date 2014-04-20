#include "myrenderer.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include "timer.h"

#include "camera.h"

using namespace vortex;
using namespace vortex::util;

static GLenum bufs[]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};

MyRenderer::MyRenderer(SceneManager *sceneManager, int width, int height) : Renderer(sceneManager), mRenderMode(0), mDisplayTextureId(0) {
    glCheckError();

    mFbo = new FBO(FBO::Components(FBO::DEPTH | FBO::COLOR), width, height);

    glCheckError();

    mTextures[DEPTH_TEXTURE] = new Texture("depth", GL_TEXTURE_2D/*GL_TEXTURE_RECTANGLE*/);
    mTextures[NORMAL_TEXTURE] = new Texture("normal", GL_TEXTURE_2D);
    mTextures[COLOR_TEXTURE] = new Texture("color", GL_TEXTURE_2D );
    mTextures[PICKING_TEXTURE] = new Texture("picking", GL_TEXTURE_2D);

    glCheckError();

    mScreenQuad = ScreenQuadBuilder().build("ScreenQuad");
    mScreenQuad->init();

    glCheckError();

    FBO::bindDefault();
    glAssert( glDrawBuffer(GL_BACK) );
    glAssert( glReadBuffer(GL_BACK) );

    mNodePicked = NULL;
    mMeshI = -1;

    glCheckError();
}

MyRenderer::~MyRenderer() {
    delete mFbo;

    // Add delete of all textures

    for (int i=0; i<NUM_TEXTURE; i++)
        delete mTextures[i];

    delete mScreenQuad;
}

void MyRenderer::displayTexture(Texture * theTexture){
    FBO::bindDefault();
    glAssert( glDrawBuffer(GL_BACK); );
    glAssert( glDepthFunc(GL_ALWAYS) );
    glAssert(glViewport(0, 0, width(), height()));
    ShaderProgram *shader = mSceneManager->getAsset()->getShaderProgram(mDisplayShaderId);
    shader->bind();
    shader->setUniformTexture("color", theTexture);
    mScreenQuad->draw();
    glAssert( glDepthFunc(GL_LESS) );
}

void MyRenderer::ambientPass(vortex::ShaderLoop &theRenderingLoop, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix, const glm::mat4x4 &viewToWorldMatrix){
    // render ambient and normal
    ShadersGlobalParameters  ambientAndNormalParamameters;
    ambientAndNormalParamameters.addParameter("view2worldMatrix", viewToWorldMatrix);
    ambientAndNormalParamameters.addParameter("ambientIntensity", 1.f );
    theRenderingLoop.draw(ambientAndNormalParamameters, modelViewMatrix, projectionMatrix);
}

void MyRenderer::lightsPass(vortex::ShaderLoop &theRenderingLoop, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix, const glm::mat4x4 &viewToWorldMatrix){
    if(mSceneManager->sceneGraph()->mLights.size()>0){
        for(unsigned int i = 0; i<mSceneManager->sceneGraph()->mLights.size(); ++i ){

            glm::mat4x4 lightMatrix = modelViewMatrix*mSceneManager->sceneGraph()->mLights[i].mTransform;
            Light l = Light(mSceneManager->sceneGraph()->mLights[i], lightMatrix);
            // tex unit 5 seems to be free ;)
            LightGlobalParameters lightParamameters(l);
            lightParamameters.addParameter("inverseViewMatrix", viewToWorldMatrix);
            theRenderingLoop.draw(lightParamameters, modelViewMatrix, projectionMatrix);
        }
    } else { // no lights in scene, set up a headlight
        Light l;
        l.mDiffuse=glm::vec3(4.0,4.0,4.0);
        LightGlobalParameters lightParamameters(l);
        lightParamameters.addParameter("inverseViewMatrix", viewToWorldMatrix);
        lightParamameters.addParameter("computeShadow", 0);
        theRenderingLoop.draw(lightParamameters, modelViewMatrix, projectionMatrix);
    }
}

void MyRenderer::renderFilled(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix){

#ifdef STATISTIQUES
    static int nbframes=0;
    static double renderTime = 0.;
//    static double postprocessTime = 0.;
//    GLuint64 stopRenderTime=0;
//    GLuint64 stopPostprocessTime=0;

    Timer graphTimer;
    Timer GPUTimer;
    static float timeCPU = 0.f;
    static float timeGPU = 0.f;

    GLuint64 globalStartTime, globalMiddleTime, globalStopTime;

#endif
    //
    // Important note before modifying this method :
    // see MyRenderer::setViewport for FBO configurations
    //
    glm::mat4x4 viewToWorldMatrix = glm::inverse(modelViewMatrix);

#ifdef STATISTIQUES
    graphTimer.start();
#endif

#ifdef STATISTIQUES
    graphTimer.stop();
    GPUTimer.start();
#endif

    // draw the scene

#ifdef STATISTIQUES
    glAssert( glQueryCounter(queryRenderID[0], GL_TIMESTAMP) );
#endif
    mFbo->useAsTarget(mWidth, mHeight);
    glAssert(glDrawBuffers(2, bufs));
    glAssert(glClearColor(0.1, 0.1, 0.1, 1.));
    glAssert(glClearDepth(1.0));
    glAssert(glDepthFunc(GL_LESS));
    glAssert(glDisable(GL_BLEND));

    mFbo->clear(FBO::ALL);// to clear all attached texture

    // render ambient and normal
    ambientPass(mAmbientAndNormalLoop, modelViewMatrix, projectionMatrix, viewToWorldMatrix);

    // setup per light rendering : blend each pass onto the previous one
    glAssert(glDrawBuffers(1, bufs));
    glAssert(glDepthFunc(GL_LEQUAL));
    glAssert(glEnable(GL_BLEND));
    glAssert(glBlendFunc(GL_ONE, GL_ONE));
    glDepthMask(GL_FALSE);

    // render for each light
    lightsPass(mMainDrawLoop, modelViewMatrix, projectionMatrix, viewToWorldMatrix);



    // restore parameter
    glAssert( glDisable(GL_BLEND) );
    glAssert( glDepthFunc(GL_LESS) );
    glDepthMask(GL_TRUE);

    drawSelection(modelViewMatrix, projectionMatrix);


#ifdef STATISTIQUES
    glAssert( glQueryCounter(queryRenderID[1], GL_TIMESTAMP) );
#endif
    glDepthFunc(GL_LESS);

#ifdef STATISTIQUES
    glAssert( glQueryCounter(queryRenderID[2], GL_TIMESTAMP) );

    GLint stopTimerAvailable = 0;
    while (!stopTimerAvailable) {
        glAssert( glGetQueryObjectiv(queryRenderID[2],
                                   GL_QUERY_RESULT_AVAILABLE,
                                   &stopTimerAvailable) );
    }

    // get query results
    glAssert( glGetQueryObjectui64v(queryRenderID[0], GL_QUERY_RESULT, &globalStartTime) );
    glAssert( glGetQueryObjectui64v(queryRenderID[1], GL_QUERY_RESULT, &globalMiddleTime) );
    glAssert( glGetQueryObjectui64v(queryRenderID[2], GL_QUERY_RESULT, &globalStopTime) );

/*
    printf("Time spent on the GPU: %f ms\n", (globalStopTime - globalStartTime) / 1000000.0);
    printf("Render time : %f ms\n", (globalMiddleTime - globalStartTime) / 1000000.0);
    printf("Post-process time : %f ms\n", (globalStopTime - globalMiddleTime) / 1000000.0);
*/

    GPUTimer.stop();
    /*
    std::cout << "Loop statistics" << std::endl;
    std::cout << "---" << "Loop size : predraw(" << ambientAndNormalLoop.size() << ") -- main(" << mainDrawLoop.size() << ")" << std::endl;
    for (ShaderLoop::iterator it = mainDrawLoop.begin(); it != mainDrawLoop.end(); ++it) {
        std::cout << it->first.getConfiguration() << std::endl;
        std::cout << "\tTransformations :" << it->second.size() << std::endl;
    } */
    double gpurender = (double)(globalMiddleTime - globalStartTime)/1000000000.0;
    double gpupproc = (double)(globalStopTime - globalMiddleTime)/1000000000.0;
    double gpufull = (double)(globalStopTime - globalStartTime)/1000000000.0;

    std::cout << "GPU Render : "  << gpurender << "\tGPU PostProcess : " << gpupproc << "\tGPU Frame time : " << gpufull << std::endl;

    std::cout << "CPU time : " << graphTimer.value() << " -- GPU time : " << GPUTimer.value() << std::endl;


    timeCPU+=graphTimer.value();
    timeGPU+=GPUTimer.value();

//    if (nbframes!=0) {
    if (gpufull < 100.0){ // parfois ... erreur de timer :(
            renderTime += (gpufull);

           double fps = ((double)nbframes)/(renderTime);
           std::cout << "fps : " << fps << "("<<renderTime<<") -- " << ((double)nbframes)/(timeCPU+timeGPU) << std::endl;
    }
    nbframes++;
#endif
}

void MyRenderer::renderWireframe(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix){

    mFbo->useAsTarget(mWidth, mHeight);
    glAssert(glDrawBuffers(1, bufs));
    glAssert(glClearColor(0.1, 0.1, 0.1, 1.));
    glAssert(glClearDepth(1.0));
    glAssert(glDepthFunc(GL_LESS));
    glAssert(glDisable(GL_BLEND));

    mFbo->clear(FBO::ALL);// to clear all attached texture

    glm::mat4x4 viewToWorldMatrix = glm::inverse(modelViewMatrix);

    glAssert(glEnable(GL_POLYGON_OFFSET_FILL));
    glAssert(glPolygonOffset(1.0, 5));

    ShadersGlobalParameters  ambientAndNormalParamameters;
    ambientAndNormalParamameters.addParameter("color", glm::vec4(0.5,0.5,0.5,1));
    mAmbientAndNormalLoop.draw(ambientAndNormalParamameters, modelViewMatrix, projectionMatrix);

    glAssert(glDisable(GL_POLYGON_OFFSET_FILL));
    glAssert(glPolygonOffset(1.0, 0));

    glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    glAssert(glEnable(GL_LINE_SMOOTH));
    glAssert(glLineWidth(1.f) );

    glAssert(glDrawBuffers(1, bufs));
    glAssert(glDepthFunc(GL_LEQUAL));
    glAssert(glEnable(GL_BLEND));
    glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    glAssert(glDepthMask(GL_FALSE));

    ambientAndNormalParamameters = ShadersGlobalParameters();
    ambientAndNormalParamameters.addParameter("color", glm::vec4(0.7,0.7,1.0,1));
    mAmbientAndNormalLoop.draw(ambientAndNormalParamameters, modelViewMatrix, projectionMatrix);

    glAssert( glDisable(GL_BLEND) );
    glAssert( glDepthFunc(GL_LESS) );
    glAssert( glDepthMask(GL_TRUE) );

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// --------------------------------------------
void MyRenderer::showTexture(int which){
    displayTexture(mTextures[which]);
}

// --------------------------------------------
void MyRenderer::render(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix){

    glAssert ( glClearColor ( 0.3f, 0.3f, 0.3f, 0.f ) );
    glAssert ( glClear ( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT ) );

    if ( ! mSceneManager->sceneGraph())
        return;
    {
        (*(mRenderOperators[mRenderMode]))(modelViewMatrix, projectionMatrix);
        renderPicking(modelViewMatrix, projectionMatrix);
        showTexture(mDisplayTextureId);
    }
}

void MyRenderer::setViewport(int width, int height)
{
    Renderer::setViewport(width, height);

    mFbo->setSize(width, height);

    if (mTextures[DEPTH_TEXTURE]->getId() != 0)
        mTextures[DEPTH_TEXTURE]->deleteGL();
    mTextures[DEPTH_TEXTURE]->initGL(GL_DEPTH_COMPONENT24, mWidth, mHeight, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    if (mTextures[NORMAL_TEXTURE]->getId() != 0)
        mTextures[NORMAL_TEXTURE]->deleteGL();
    if (mTextures[COLOR_TEXTURE]->getId() != 0)
        mTextures[COLOR_TEXTURE]->deleteGL();
    if (mTextures[PICKING_TEXTURE]->getId() != 0)
        mTextures[PICKING_TEXTURE]->deleteGL();

    mTextures[NORMAL_TEXTURE]->initGL(GL_RGBA32F, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    mTextures[COLOR_TEXTURE]->initGL(GL_RGBA32F, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    mTextures[PICKING_TEXTURE]->initGL(GL_RGBA32F, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    mFbo->bind();
    mFbo->attachTexture(GL_COLOR_ATTACHMENT0, mTextures[COLOR_TEXTURE]);
    mFbo->attachTexture(GL_COLOR_ATTACHMENT1, mTextures[NORMAL_TEXTURE]);
    mFbo->attachTexture(GL_COLOR_ATTACHMENT2, mTextures[PICKING_TEXTURE]);
    mFbo->attachTexture(GL_DEPTH_ATTACHMENT, mTextures[DEPTH_TEXTURE]);
    mFbo->check();
    mFbo->unbind();

    FBO::bindDefault();
    glAssert (glDrawBuffer(GL_BACK); );
    glAssert (glReadBuffer(GL_BACK); );
}


void MyRenderer::initRessources(AssetManager *assetManager){

    assetManager->setShaderFileExtentions(std::string(".vert.glsl"), std::string(".frag.glsl"));
    assetManager->setShaderBasePath(std::string("../src/basic/shaders/"));

    mDisplayShaderId = assetManager->addShaderProgram("display");
    mPickingShaderId = assetManager->addShaderProgram("fill");

#ifdef STATISTIQUES
    glAssert( glGenQueries(4, queryRenderID) );
#endif

    /*
     * Image Computation shaders
     */
    if(mSceneManager->sceneGraph()){
        // Ambient and normal computation
        {
            ShaderBuilder shaderCreatorVisitor(assetManager, "ambient", false);
            mAmbientAndNormalFilter = new MaterialPropertiesSelectorFilter;
            mAmbientAndNormalFilter->addProperty(Material::TEXTURE_OPACITY);
            mAmbientAndNormalFilter->addProperty(Material::TEXTURE_HEIGHT);
            mAmbientAndNormalFilter->addProperty(Material::TEXTURE_NORMALS);
            mAmbientAndNormalFilter->addProperty(Material::TEXTURE_AMBIENT);
            mAmbientAndNormalFilter->addProperty(Material::TEXTURE_DIFFUSE);
            mAmbientAndNormalFilter->addProperty(Material::TEXTURE_SPECULAR);
            shaderCreatorVisitor.setFilter(mAmbientAndNormalFilter);
            SceneGraph::PreOrderVisitor visit(mSceneManager->sceneGraph(), shaderCreatorVisitor);
            visit.go();
        }

        // Lighting computation
        {
            ShaderBuilder shaderCreatorVisitor(assetManager, "phong");
            SceneGraph::PreOrderVisitor visit(mSceneManager->sceneGraph(), shaderCreatorVisitor);
            visit.go();
        }

        buildRenderingLoops();
    }

    mRenderOperators.push_back(new FilledRenderOperator(this));
    mRenderOperators.push_back(new WireRenderOperator(this));

    if(mSceneManager->sceneGraph()){
        assetManager->statistics();
        GraphStatistic statistic;
        SceneGraph::PreOrderVisitor statisticVisitor(mSceneManager->sceneGraph(), statistic);
        statisticVisitor.go();
        std::cerr << statistic << std::endl;
    }

}

void MyRenderer::buildRenderingLoops(){
    mMainDrawLoop.clear();
    mAmbientAndNormalLoop.clear();
    mPickingLoop.clear();

    if (mRenderMode == 0) { // Fill mode
        // Light loop builder
        DefaultLoopBuilder mainDrawLoopBuilder(mSceneManager->sceneGraph(), &mMainDrawLoop);

        // Ambient and normal loop builder
        ShaderLoopBuilder ambientAndNormalLoopBuilder(mSceneManager->getAsset(), mSceneManager->sceneGraph(), &mAmbientAndNormalLoop, "ambient");
        ambientAndNormalLoopBuilder.setFilter(mAmbientAndNormalFilter);

        // Setup loop builder
        SetOfLoopBuilder renderPassesBuilder;
        renderPassesBuilder.addLoopBuilder(&mainDrawLoopBuilder);
        renderPassesBuilder.addLoopBuilder(&ambientAndNormalLoopBuilder);
        SceneGraph::PostOrderVisitor renderPassesVisitor(mSceneManager->sceneGraph(), renderPassesBuilder);
        renderPassesVisitor.go(glm::mat4(1.0), glm::mat4(1.0));

    } else { // WireMode
        ShaderLoopBuilder ambientAndNormalLoopBuilder(mSceneManager->getAsset(), mSceneManager->sceneGraph(), &mAmbientAndNormalLoop, "fill");
        ambientAndNormalLoopBuilder.setFilter(mAmbientAndNormalFilter);
        // Setup loop builder
        SceneGraph::PostOrderVisitor renderPassesVisitor(mSceneManager->sceneGraph(), ambientAndNormalLoopBuilder);
        renderPassesVisitor.go(glm::mat4(1.0), glm::mat4(1.0));
    }

    {
        /* Get all meshes, each mesh have an unique ID witch is his position in the vector */
        int id = 1;
        ShaderProgram *pickingShader = mSceneManager->getAsset()->getShaderProgram(mPickingShaderId);
        PickingLoopBuilder pickingPassesVisitor(&mPickingLoop, pickingShader, &id);
        SceneGraph::PostOrderVisitor renderPassesVisitor(mSceneManager->sceneGraph(), pickingPassesVisitor);
        renderPassesVisitor.go(glm::mat4(1.0), glm::mat4(1.0));
    }
}

void MyRenderer::renderPicking(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix) {
    ShaderProgram *pickingShader = mSceneManager->getAsset()->getShaderProgram(mPickingShaderId);

    // Bind the FBO
    mFbo->useAsTarget(mWidth, mHeight);
    GLuint attachments[1] = {GL_COLOR_ATTACHMENT2};
    glAssert(glDrawBuffers(1, attachments));
    glAssert(glClearColor(0., 0., 0., 1.));
    glAssert(glClearDepth(1.0));
    glAssert(glDepthFunc(GL_LESS));
    glAssert(glDisable(GL_BLEND));
    mFbo->clear(FBO::ALL);// to clear all attached texture

    pickingShader->bind();
    for(PickingLoop::iterator it = mPickingLoop.begin() ; it != mPickingLoop.end() ; ++it) {
        it->first.bind(modelViewMatrix, projectionMatrix);
        it->second->draw();
    }

    glDepthMask(GL_TRUE);
}

void MyRenderer::drawSelection(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix) {
    if (mNodePicked) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glLineWidth(3);

        ShaderProgram *pickingShader = mSceneManager->getAsset()->getShaderProgram(mPickingShaderId);
        pickingShader->bind();

        BBoxRenderer bboxPassesVisitor(getMeshPicked(), glm::vec4(0,0,0,1), pickingShader);
        SceneGraph::PostOrderVisitor renderPassesVisitor(mSceneManager->sceneGraph(), bboxPassesVisitor);
        renderPassesVisitor.go(modelViewMatrix, projectionMatrix);
    }
}

void MyRenderer::pick(int x, int y){
    GLubyte data[4];
    GLint viewport[4];

    mFbo->bind();
    glReadBuffer(GL_COLOR_ATTACHMENT2);

    // Transformer les coordonnées souris en coordonnées framebuffer
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Lecture de la couleur du pixel
    glAssert(glReadPixels(x, viewport[3]-y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data));
    glm::vec3 color(data[0], data[1], data[2]);

    // On déselectionne l'ancien object si c'est nécessaire
    if (mNodePicked)
        this->getMeshPicked()->setSelected(false);

    if (color.x == 0 && color.y == 0 && color.z == 0) {
        mNodePicked = NULL;
        mMeshI = -1;
    }
    else {
        // On cherche le mesh cliqué pour mettre son attribut selected à true
        PickingState picked = PickingState(PickingState::colorToId(color));
        PickingLoop::iterator it = mPickingLoop.find(picked);
        if (it != mPickingLoop.end())
            it->second->setSelected(true);

        // Permet de récupérer des informations sur l'object cliqué plus rapidement
        GetLeafMeshNodeSelected get(&mNodePicked, &mMeshI);
        SceneGraph::PostOrderVisitor visitor(getScene()->sceneGraph(), get);
        visitor.go();
    }
}

void MyRenderer::reloadShaders(){
    buildRenderingLoops();
}

MyRenderer::PickingLoopBuilder::PickingLoopBuilder(PickingLoop *loop, ShaderProgram *shader, int *id) : mLoop(loop), mShader(shader), mId(id) {
}

void MyRenderer::PickingLoopBuilder::operator()(SceneGraph::Node *theNode, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix)
{
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if ((*leafNode)[i]) {;
                (*mLoop)[PickingState((*mId), modelViewMatrix, projectionMatrix, mShader)] = (*leafNode)[i];
                (*mId)++;
            }
        }
    }
}

MyRenderer::GetLeafMeshNodeSelected::GetLeafMeshNodeSelected(vortex::SceneGraph::LeafMeshNode **node, int *meshI) : mNode(node), mMeshI(meshI) {
}

void MyRenderer::GetLeafMeshNodeSelected::operator()(SceneGraph::Node *theNode) {
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if ((*leafNode)[i]->isSelected()) {
                *mNode = leafNode;
                *mMeshI = i;
            }
        }
    }
}

glm::vec3 MyRenderer::idToColor(int id) {
    float r = ((id / 65536) % 256) / 255.0;
    float g = ((id / 256) % 256) / 255.0;
    float b = (id % 256) / 255.0;

    return glm::vec3(r, g, b);
}

int MyRenderer::colorToId(glm::vec3 color) {
    return color.x * 65536 + color.y * 256 + color.z;
}

