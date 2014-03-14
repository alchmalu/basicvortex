#include <iostream>
#include <fstream>

#include "vortexengine.h"


#include "visitors.h"

#include "shaderobject.h"

#include "renderer.h"
#include "camera.h"

#include "controller.h"

using namespace std;

namespace vortex {

VortexEngine::VortexEngine() : mActiveRenderer(NULL)
{
    mAssetManager = new AssetManager();
    mSceneManager = new SceneManager(mAssetManager);
    mCamera = NULL;
}

VortexEngine::VortexEngine(AssetManager *assetManager, SceneManager *sceneManager) : mActiveRenderer(NULL)
{
    mAssetManager = assetManager;
    mSceneManager = sceneManager;
    mCamera = NULL;
}

VortexEngine::~VortexEngine()
{
    delete mAssetManager;
    delete mSceneManager;
}


bool VortexEngine::loadScene(std::string fileName){
    std::cerr << "*** VortexEngine::loadScene ***" << std::endl;    


         mSceneManager->newScene();
         delete mAssetManager;
         mAssetManager = new AssetManager();
         mSceneManager->setAssetManager(mAssetManager);
         bool ret = mSceneManager->loadScene(fileName);

         if(ret) {
             BBox sceneBox = mSceneManager->getBBox();
             glm::vec3 ext=sceneBox.getExtend();

             std::cerr << "Bbox : " << sceneBox <<  std::endl;
             std::cerr << "Extend : " << ext[0] << " " << ext[1] << " " << ext[2] <<  std::endl;
             resetCamera();
             mActiveRenderer->initRessources(mAssetManager);
         }
         return ret;


}

//
void VortexEngine::init(Renderer *activeRenderer)
{    


    /* initialise openGL */
    glAssert(glEnable(GL_DEPTH_TEST));
    glAssert(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    // WARNING : uncoment the next line gives segfault on GeForce 8800 GTX :(
    glAssert(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));

    /* build default renderers */
    mActiveRenderer = activeRenderer;
    /* this must be done only after loading a scene*/
    mActiveRenderer->initRessources(mAssetManager);

    resetCamera();
    mActiveRenderer->setCamera(mCamera);
}

void VortexEngine::setViewportSize(int width, int height)
{
    mWidth  = width;
    mHeight = height;    
    mActiveRenderer->setViewport(mWidth, mHeight);
};

/**
 * @todo use here pass abstraction mechanism
 */
void VortexEngine::draw()
{
    mCamera->setScreenWidthAndHeight(mWidth, mHeight);
    mCamera->computeModelViewMatrix();
    mCamera->computeProjectionMatrix();
    glm::mat4x4 modelViewMatrix = mCamera->getModelViewMatrix(); //mCamera->sceneToViewMatrix();
    glm::mat4x4 projectionMatrix = mCamera->getProjectionMatrix(); //mCamera->projectionMatrix();

    // only one of them should be implemented in a given renderer
    mActiveRenderer->render(modelViewMatrix, projectionMatrix);
    mActiveRenderer->render(mCamera);
}

void VortexEngine::setRenderer(Renderer *renderer){
    mActiveRenderer = renderer;
    mActiveRenderer->setCamera(mCamera);
}

Renderer * VortexEngine::renderer(){
    return mActiveRenderer;
}

bool VortexEngine::handleKeyEvent(char key)
{
    if (mActiveRenderer->handleKeyEvent(key))
        return true;

    switch (key) {
    case 'r':
        resetCamera();
        return true;
        break;
    }
    return false;
}

bool VortexEngine::handleMouseEvent(const MouseEvent& event)
{

    static int button = 0;
    static int modifiers = 0;

    if (event.click) {

        // mouse presse event : start an action
        MouseAction action;
        button = event.button;
        modifiers = event.modifiers;
        switch (button) {
        case MouseEvent::LEFT :
            if (modifiers & MouseEvent::CONTROL)
                action = VortexEngine::LOOK_AROUND;
            else
                action = VortexEngine::ROTATE;
            break;
        case MouseEvent::RIGHT :
            action = VortexEngine::TRANSLATE;
            break;
        case MouseEvent::MIDDLE :
            action = VortexEngine::ZOOM;
            break;
        default:
           action = VortexEngine::NO_MOUSE_ACTION;
        }

        // beware : opengl ref is at lower left corner, qt at top left
        //float depthAtPixel = mActiveRenderer->readDepthAt(event.x, mHeight - event.y);
        //mCamera->setDepthAtClick(glm::vec3(event.x, event.y, depthAtPixel));
        //mCamera->setRevolveAroundPointFromPixel(glm::ivec2(event.x, event.y));
        mCamera->frame()->startAction(action);
        mCamera->frame()->mousePressEvent(&event, mCamera);
        return false;
    } else {
        bool modified = mCamera->frame()->mouseMoveEvent(&event, mCamera);
        return modified;
    }
#if 0
    static int modifiers = 0;
    static int x = 0;
    static int y = 0;

    static bool moved = false;
    /*
        if (activeRenderer->handleMouseEvent(event))
            return;
    */
    if (event.click) {
        x = event.x;
        y = event.y;
        button = event.button;
        modifiers = event.modifiers;
    }

    if (event.button == MouseEvent::MOVE) {
//       std::cerr << "Move button event" << std::endl;
//        float dx = (float)(event.x - x) / (float) mWidth;
//        float dy = - ((float)(event.y - y) / (float) mHeight);

        float dx = (float)(event.x - x) / (float) mWidth;
        float dy = (float)(event.y - y) / (float) mHeight;

        switch (button) {
        case MouseEvent::LEFT :
            //mCamera.localRotate ( dx*M_PI, dy*M_PI );
            //mCamera.niceRotate(event.x / float(mWidth), event.y / float(mHeight), x / float(mWidth), y / float(mHeight));
            //mCamera->sweetRotate(dx*360.0f, dy*360.f);
            break;
        case MouseEvent::RIGHT :
            //mCamera.localTranslate(glm::vec3(-dx * 5.f, -dy * 5.f, 0.f));
            //mCamera->sweetPan(dx/**10.f*/, dy/**10.f*/);
            break;
        case MouseEvent::MIDDLE :
            //mCamera.localTranslate(glm::vec3(0.0f, 0.0f, float(y - event.y) * 5 / (float) mHeight));
            //mCamera->sweetZoom(dy/**10.f*/);
            break;
        }
        x = event.x;
        y = event.y;

        moved = true;

        return true;
    }

    if (event.button == MouseEvent::WHEEL) {
//       std::cerr << "Weel button event : " << event.delta << std::endl;
        //mCamera.localTranslate(glm::vec3(0.0f, 0.0f, float(y - event.y) * 5 / (float) mHeight));
        //mCamera.localTranslate(glm::vec3(0.0f, 0.0f, (event.delta < 0) ? -0.75f : 0.75f));
        //mCamera.globalZoom((event.delta < 0) ? 0.75f : 1.25f);
        float dy = (event.delta < 0) ? -0.1f : 0.1f;
        //mCamera->sweetZoom(dy);
        return true;
    }

    if (button == MouseEvent::LEFT) {
//     std::cerr << "Left button event" << std::endl;
        moved = false;
        return false;
    }

    if (button == MouseEvent::MIDDLE) {
//       std::cerr << "Middle button event" << std::endl;
        return false;
    }

    if (button == MouseEvent::RIGHT) {
//       std::cerr << "Right button event" << std::endl;
        return false;
    }

    if (!event.click) {
        button = 0;
        modifiers = 0;
    }
    return false;
#endif
}

/** todo manage here configuration change and renderstate definition for the scenegraph
 */
void VortexEngine::setConfiguration(const EngineConfiguration& configuration)
{
    //mEngineConfiguration = configuration;
}

//const VortexEngine::EngineConfiguration& VortexEngine::getConfiguration() const
//{
//    //return mEngineConfiguration;
//    return VortexEngine::EngineConfiguration();
//}

VortexEngine::EngineConfiguration::EngineConfiguration(const EngineConfiguration& configuration)
{
    mRenderMode = configuration.mRenderMode;
    mAlgorithm = configuration.mAlgorithm;
}

void VortexEngine::reloadShaders()
{
    mSceneManager->getAsset()->reloadShaders();

    if(renderer()) renderer()->reloadShaders();
}

void VortexEngine::resetCamera() {
    if (mCamera)
        delete mCamera;
    mCamera = new Camera;
    mCamera->setScreenWidthAndHeight(mWidth, mHeight);

    // BEWARE OF SENSITIVITIES !!!
    //mCamera->frame()->setRotationSensitivity(10.0); // default : 1.0
    //mCamera->frame()->setTranslationSensitivity(10.0); // default : 1.0

    if (mSceneManager && mSceneManager->sceneGraph()) {
        mCamera->setSceneBoundingBox(mSceneManager->getBBox().getMin(),
                                     mSceneManager->getBBox().getMax());
    }
    mCamera->showEntireScene();

    mCamera->computeModelViewMatrix();
    mCamera->computeProjectionMatrix();

    mActiveRenderer->setCamera(mCamera);
}

bool VortexEngine::updateAnimations(double time){
    bool modified = mAssetManager->updateAnimations(time);   

    if (modified) {
        mActiveRenderer->updateAnimations(time);
    }
    return modified;
}

bool VortexEngine::updateAnimations(int percent){
 double theTime = mAssetManager->setAnimationsAt(percent);
 mActiveRenderer->updateAnimations(theTime);
 return true;
}

} // namespace vortex



