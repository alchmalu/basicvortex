#ifndef MYRENDERER_H
#define MYRENDERER_H

#include "renderer.h"
#include "skybox.h"

#include <string>

//#define STATISTIQUES


/**
  * Sample (but not simple) full-featured renderer ...
  */

class MyRenderer : public vortex::Renderer{
    friend class RaytracingRenderOperator;
public:
    // public enums
    enum TextureId {COLOR_TEXTURE, DEPTH_TEXTURE, NORMAL_TEXTURE, NUM_TEXTURE};
    std::string getTextureName(int i){
        static const std::string textureNames[]={"COLOR_TEXTURE", "DEPTH_TEXTURE", "NORMAL_TEXTURE"};
        return textureNames[i];
    }

    // Constructors -- Destructors
    MyRenderer(vortex::SceneManager *sceneManager, int width = 1, int height = 1);
    ~MyRenderer();

    // Internal classes
    class RenderOperator{
    protected:
        MyRenderer *mAssociatedRenderer;
        RenderOperator() : mAssociatedRenderer(NULL) {}
    public:
        RenderOperator(MyRenderer *theRenderer) : mAssociatedRenderer(theRenderer) {}
        virtual ~RenderOperator(){}
        virtual void operator()(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix) = 0;
    };

    // Public methods
    void buildRenderingLoops();
    void render(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix);

    void setViewport(int width, int height);

    void initRessources(vortex::AssetManager *assetManager);

    void setRenderingMode(int mode) {
        bool needLoopRebuild = (mRenderMode != mode);
        mRenderMode=mode;
        if (needLoopRebuild)
            buildRenderingLoops();
    }
    bool getRenderMode() const {
        return (mRenderMode==0);
    }
    void setDisplayTextureId(int id) {
        std::cerr << "display texture id " << id<< std::endl;
        mDisplayTextureId=id;
    }

    float readDepthAt(int x, int y);

    void showTexture(int which);

    vortex::Texture *getTexture(TextureId sourceTexture){
        return mTextures[sourceTexture];
    }

    void drawScreenQuad() { mScreenQuad->draw(); }
    void reloadShaders();

private:
    /// Textures for all the rendering steps
    vortex::Texture * mTextures[NUM_TEXTURE];

    class FilledRenderOperator : public RenderOperator{
        FilledRenderOperator() : RenderOperator(){}
    public:
        FilledRenderOperator(MyRenderer *theRenderer) : RenderOperator(theRenderer) {}
        ~FilledRenderOperator(){}
        void operator()(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix){
            mAssociatedRenderer->renderFilled(modelViewMatrix, projectionMatrix);
        }
    };

    class WireRenderOperator : public RenderOperator{
        WireRenderOperator() :  RenderOperator(){}
    public:
        WireRenderOperator(MyRenderer *theRenderer) : RenderOperator(theRenderer) {}
        ~WireRenderOperator(){}
        void operator()(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix){
            mAssociatedRenderer->renderWireframe(modelViewMatrix, projectionMatrix);
        }
    };

    std::vector<RenderOperator*> mRenderOperators;

    void renderFilled(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix);
    void renderWireframe(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix);

    void drawSkyBox(int shaderId, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix);
    void ambientPass(vortex::ShaderLoop &theRenderingLoop, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix, const glm::mat4x4 &viewToWorldMatrix);
    void lightsPass(vortex::ShaderLoop &theRenderingLoop, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix, const glm::mat4x4 &viewToWorldMatrix);
    void displayTexture(vortex::Texture * theTexture);

    vortex::FBO *mFbo;
    vortex::Mesh *mScreenQuad;

    /* Image processing shaders */
    int mDisplayShaderId;

    vortex::MaterialPropertyFilter *mAmbientAndNormalFilter;
    vortex::MaterialPropertyFilter *mDepthFilter;

#ifdef STATISTIQUES
    unsigned int queryRenderID[4];
#endif
    int mRenderMode;
    int mDisplayTextureId;

    // render loops
    vortex::ShaderLoop mMainDrawLoop;
    vortex::ShaderLoop mAmbientAndNormalLoop;
};

#endif // MATRENDERER_H
