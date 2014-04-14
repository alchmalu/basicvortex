/***************************************************************************
 *   Copyright (C) 2008 by Mathias Paulin   *
 *   Mathias.Paulin@irit.fr   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "openglwidget.h"

#include "timer.h"
#include <QTimer>
#include <QMenu>

#include <QWheelEvent>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TAILLE_MAX_IMAGES_TAB 1500

using namespace vortex;

//-----------------------------------
void cerrInfoGL(OpenGLWidget *w) {
    const GLubyte *str;
    glAssert( str = glGetString(GL_RENDERER) );
    std::cerr << "Renderer : " << str << std::endl;
    glAssert( str = glGetString(GL_VENDOR) );
    std::cerr << "Vendor : " << str << std::endl;
    glAssert( str = glGetString(GL_VERSION) );
    std::cerr << "Version : " << str << std::endl;
    glAssert( str = glGetString(GL_SHADING_LANGUAGE_VERSION) );
    std::cerr << "GLSL Version : " << str << std::endl;

    if (!w->isValid()){
        std::cerr << " Invalid (sic) "<< std::endl;
    }
}

#ifdef __APPLE__
//-----------------------------------
// To get OpenGL 3.2 core on Mac Os X [mountain]lion with QT 4.8.4


void* select_3_2_mac_visual(GDHandle handle);


struct Core3_2_context : public QGLContext
{
    Core3_2_context(const QGLFormat& format, QPaintDevice* device) : QGLContext(format,device) {}
    Core3_2_context(const QGLFormat& format) : QGLContext(format) {}
    virtual void* chooseMacVisual(GDHandle handle)
    {
        return select_3_2_mac_visual(handle);
    }
};


OpenGLWidget::OpenGLWidget ( QWidget *parent) : QGLWidget (new Core3_2_context(QGLFormat::defaultFormat()) ), m_isClick ( false ), m_mouseX ( 0 ), m_mouseY ( 0 ), m_trackMode ( TRACK_NO_TRACK ) {
    setFocusPolicy ( Qt::StrongFocus );
    makeCurrent();
    cerrInfoGL(this);
    mAnimationTimer=new QTimer( this );
    connect( mAnimationTimer, SIGNAL(timeout()), this, SLOT(animate()) );
    mRendercontrol = NULL;
}


#else

OpenGLWidget::OpenGLWidget ( QWidget *parent ) : QGLWidget ( parent ),mCamera(NULL), m_isClick ( false ), m_mouseX ( 0 ), m_mouseY ( 0 ), m_trackMode ( TRACK_NO_TRACK ) {
    setFocusPolicy ( Qt::StrongFocus );
    makeCurrent();
    cerrInfoGL(this);
    mAnimationTimer=new QTimer( this );
    connect( mAnimationTimer, SIGNAL(timeout()), this, SLOT(animate()) );
    mRendercontrol = NULL;
}

#endif

OpenGLWidget::~OpenGLWidget() {
    delete mRenderer;
}

void OpenGLWidget::clear() {
    updateGL();
}

void OpenGLWidget::initializeGL() {
    //    std::cerr << "OpenGLWidget::initializeGL" << std::endl;

    mAssetManager = new AssetManager();
    mSceneManager = new SceneManager(mAssetManager);
    mRenderer = new MyRenderer(mSceneManager);

    /* initialise openGL */
    glAssert(glEnable(GL_DEPTH_TEST));
    glAssert(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    // WARNING : uncoment the next line gives segfault on GeForce 8800 GTX :(
    glAssert(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));

    mRenderer->initRessources(mAssetManager);

    resetCamera();
    mRenderer->setCamera(mCamera);

    mRendercontrol = new RendererControl(mRenderer , this);
    mRendercontrol->setGlWidget(this);

    mMaterialEditor = new MaterialEditor(mRenderer , this);
    mMaterialEditor->setGlWidget(this);
    mMaterialEditor->setModal(false);

    glCheckError();
}

void OpenGLWidget::animate(){
    mTime_anim = Timer::getTime()-mTime_start;
    //std::cerr << "The time : " << time_anim << std::endl;

    bool modified = mAssetManager->updateAnimations(mTime_anim);

    if (modified) {
        mRenderer->updateAnimations(mTime_anim);
        updateGL();
    }
}

void OpenGLWidget::setAnimationAt(int percent){
    double theTime = mAssetManager->setAnimationsAt(percent);
    mRenderer->updateAnimations(theTime);
    updateGL();
}


void OpenGLWidget::resizeGL ( int w, int h ) {
    mWidth=w;
    mHeight=h;
    mRenderer->setViewport(mWidth, mHeight);
    glCheckError();
}

void OpenGLWidget::paintGL() {
    glCheckError();

    static int frames = 0;
    static float time = 0.0;
    Timer t;
    t.start();

    mCamera->setScreenWidthAndHeight(mWidth, mHeight);
    mCamera->computeModelViewMatrix();
    mCamera->computeProjectionMatrix();
    glm::mat4x4 modelViewMatrix = mCamera->getModelViewMatrix(); //mCamera->sceneToViewMatrix();
    glm::mat4x4 projectionMatrix = mCamera->getProjectionMatrix(); //mCamera->projectionMatrix();

    mRenderer->render(modelViewMatrix, projectionMatrix);

    t.stop();
    time += t.value();

    if ( frames % 25 == 0 ) {
        float fps = ( ( float ) frames ) / time;
        QString thetext = QString ( "FPS : %1" ).arg ( fps );
        emit fpsChanged ( thetext );
        time = 0.0;
        frames = 0;
    }
    frames += 1;
}

void OpenGLWidget::keyPressEvent ( QKeyEvent * e ) {

    QString text=e->text();

    if ( !text.isEmpty() ) {
        char key = text.at ( 0 ).toAscii();
        if (!mRenderer->handleKeyEvent(key))
        {
            switch(key) {
            case 't':
                showRenderControl();
                break;

            case 'r':
                resetCamera();
                break;
            }
            updateGL();
        }
    }
}

void OpenGLWidget::wheelEvent ( QWheelEvent * e ) {
    vortex::VortexEngine::MouseEvent event;

    event.button=vortex::VortexEngine::MouseEvent::WHEEL;

    event.modifiers=vortex::VortexEngine::MouseEvent::NONE;
    if ( e->modifiers() & Qt::ShiftModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::SHIFT;
    if ( e->modifiers() & Qt::ControlModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::CONTROL;
    if ( e->modifiers() & Qt::AltModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::ALT;

    event.click=false;
    event.x = e->x();
    event.y = e->y();
    event.delta = e->delta();
    if(handleMouseEvent(event))
        updateGL();
}

void OpenGLWidget::mouseMoveEvent ( QMouseEvent * e ) {
    vortex::VortexEngine::MouseEvent event;

    // dont need to know which button
    event.button=vortex::VortexEngine::MouseEvent::MOVE;

    event.modifiers=vortex::VortexEngine::MouseEvent::NONE;
    if ( e->modifiers() & Qt::ShiftModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::SHIFT;
    if ( e->modifiers() & Qt::ControlModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::CONTROL;
    if ( e->modifiers() & Qt::AltModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::ALT;

    event.click=false;
    event.x = e->x();
    event.y = e->y();
    event.delta = 0;

    if(handleMouseEvent(event))
        updateGL();
}

bool OpenGLWidget::handleMouseEvent(const vortex::VortexEngine::MouseEvent& event){

    static int button = 0;
    static int modifiers = 0;

    typedef vortex::VortexEngine::MouseAction MouseAction;
    typedef vortex::VortexEngine::MouseEvent MouseEvent;

    if (event.click) {

        // mouse presse event : start an action
        vortex::VortexEngine::MouseAction action;
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
        if(event.button == MouseEvent::WHEEL){
            mCamera->frame()->startAction(VortexEngine::ZOOM);
            return mCamera->frame()->wheelEvent(&event, mCamera);
        }
        return mCamera->frame()->mouseMoveEvent(&event, mCamera);
    }

    return true;
}

void OpenGLWidget::mouseReleaseEvent ( QMouseEvent *e ) {
    vortex::VortexEngine::MouseEvent event;

    if ( e->button() &Qt::LeftButton )
        event.button=vortex::VortexEngine::MouseEvent::LEFT;
    if ( e->button() &Qt::MidButton )
        event.button=vortex::VortexEngine::MouseEvent::MIDDLE;
    if ( e->button() &Qt::RightButton )
        event.button=vortex::VortexEngine::MouseEvent::RIGHT;

    event.modifiers=vortex::VortexEngine::MouseEvent::NONE;
    if ( e->modifiers() & Qt::ShiftModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::SHIFT;
    if ( e->modifiers() & Qt::ControlModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::CONTROL;
    if ( e->modifiers() & Qt::AltModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::ALT;

#ifdef __APPLE__
    if ( (e->modifiers() & Qt::ShiftModifier) && (e->button() &Qt::LeftButton) )
        event.button=vortex::VortexEngine::MouseEvent::MIDDLE;
#endif

    event.click=false;
    event.x = e->x();
    event.y = e->y();
    event.delta = 0;

    if (handleMouseEvent ( event ))
        updateGL();
}

void OpenGLWidget::mousePressEvent ( QMouseEvent * e ) {
    vortex::VortexEngine::MouseEvent event;

    if ( e->button() &Qt::LeftButton )
        event.button=vortex::VortexEngine::MouseEvent::LEFT;
    if ( e->button() &Qt::MidButton )
        event.button=vortex::VortexEngine::MouseEvent::MIDDLE;
    if ( e->button() &Qt::RightButton )
        event.button=vortex::VortexEngine::MouseEvent::RIGHT;

    event.modifiers=vortex::VortexEngine::MouseEvent::NONE;
    if ( e->modifiers() & Qt::ShiftModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::SHIFT;
    if ( e->modifiers() & Qt::ControlModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::CONTROL;
    if ( e->modifiers() & Qt::AltModifier )
        event.modifiers|=vortex::VortexEngine::MouseEvent::ALT;

#ifdef __APPLE__
    if ( (e->modifiers() & Qt::ShiftModifier) && (e->button() &Qt::LeftButton) )
        event.button=vortex::VortexEngine::MouseEvent::MIDDLE;
#endif

    event.click=true;
    event.x = e->x();
    event.y = e->y();
    event.delta = 0;

    if (handleMouseEvent ( event ))
        updateGL();

<<<<<<< HEAD
    if( (e->button() & Qt::LeftButton) && (e->modifiers() & Qt::ControlModifier) ) {
        mRenderer->pick(e->x(), e->y());
        mMaterialEditor->updateView();
    }
=======
    this->mRenderer->pick(e->x(), e->y());
    mMaterialEditor->updateView();
>>>>>>> 0a49773ec341312329eebc1d2aff39cd6d5a0756
}

void OpenGLWidget::switchAnimation(bool on){
    mTime_start = Timer::getTime();
    if (on)
        mAnimationTimer->start( 25 );
    else {
        mAnimationTimer->stop();
        bool modified = mAssetManager->updateAnimations(-1);

        if (modified) {
            mRenderer->updateAnimations(-1);
        }

        updateGL();
    }
}

void OpenGLWidget::switchRenderingMode(bool on){
    mRenderer->setRenderingMode((int)on);
    updateGL();
}

void OpenGLWidget::showRenderControl(){
    mRendercontrol->setRenderMode(mRenderer->getRenderMode());
    mRendercontrol->show();
}

void OpenGLWidget::toggleMaterialEditor() {
    if (!mMaterialEditor->isVisible()) {
        mMaterialEditor->updateView();
        mMaterialEditor->show();
    } else
        mMaterialEditor->hide();
}


void OpenGLWidget::resetCamera(){
    if (mCamera)
        delete mCamera;
    mCamera = new Camera;
    mCamera->setScreenWidthAndHeight(mWidth, mHeight);
    if (mSceneManager && mSceneManager->sceneGraph()) {
        mCamera->setSceneBoundingBox(mSceneManager->getBBox().getMin(),
                                     mSceneManager->getBBox().getMax());
    }
    mCamera->showEntireScene();

    mCamera->computeModelViewMatrix();
    mCamera->computeProjectionMatrix();
    mCamera->frame()->setRotationSensitivity(2);
    mCamera->setRevolveAroundPoint(mCamera->sceneCenter());

    mRenderer->setCamera(mCamera);
}

bool OpenGLWidget::loadScene(std::string fileName){
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
        mRenderer->initRessources(mAssetManager);
    }
    return ret;
}

void OpenGLWidget::reloadShaders(){
    mAssetManager->reloadShaders();
    if(mRenderer){
        mRenderer->reloadShaders();
    }
}

