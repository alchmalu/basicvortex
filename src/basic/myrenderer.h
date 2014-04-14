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
    enum TextureId {COLOR_TEXTURE, DEPTH_TEXTURE, NORMAL_TEXTURE, PICKING_TEXTURE, NUM_TEXTURE};
    std::string getTextureName(int i){
        static const std::string textureNames[]={"COLOR_TEXTURE", "DEPTH_TEXTURE", "NORMAL_TEXTURE", "PICKING_TEXTURE"};
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

    void showTexture(int which);

    vortex::Texture *getTexture(TextureId sourceTexture){
        return mTextures[sourceTexture];
    }

    vortex::Mesh::MeshPtr getMeshPicked() {
        if (mNodePicked)
            return (*mNodePicked)[mMeshI];
        else
            return NULL;
    }

    vortex::Material* getMaterialPicked() {
        if (mNodePicked)
            return mNodePicked->getRenderState(mMeshI)->getMaterial();
        else
            return NULL;
    }

    void drawScreenQuad() { mScreenQuad->draw(); }
    void reloadShaders();

<<<<<<< HEAD
    void pick(int x, int y);
=======

    vortex::Mesh::MeshPtr pick(int x, int y);
>>>>>>> 0a49773ec341312329eebc1d2aff39cd6d5a0756
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
    void renderPicking(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix);
<<<<<<< HEAD
    void drawSelection(const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix);
=======
>>>>>>> 0a49773ec341312329eebc1d2aff39cd6d5a0756

    void drawSkyBox(int shaderId, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix);
    void ambientPass(vortex::ShaderLoop &theRenderingLoop, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix, const glm::mat4x4 &viewToWorldMatrix);
    void lightsPass(vortex::ShaderLoop &theRenderingLoop, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix, const glm::mat4x4 &viewToWorldMatrix);
    void displayTexture(vortex::Texture * theTexture);

    vortex::FBO *mFbo;
    vortex::Mesh *mScreenQuad;

    /* Image processing shaders */
    int mDisplayShaderId;
    int mPickingShaderId;

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

<<<<<<< HEAD
=======

>>>>>>> 0a49773ec341312329eebc1d2aff39cd6d5a0756
    /*
     ***********************************
     * PICKING
     ***********************************
     */

<<<<<<< HEAD
    typedef std::map<vortex::PickingState, vortex::Mesh::MeshPtr> PickingLoop;
    PickingLoop mPickingLoop;

    class PickingLoopBuilder : public vortex::SceneGraph::VisitorOperation {
    public:
        PickingLoopBuilder(PickingLoop *loop, vortex::ShaderProgram *shader, int *id);
        void operator()(vortex::SceneGraph::Node *theNode, const glm::mat4x4 &modelViewMatrix, const glm::mat4x4 &projectionMatrix);
    private:
        PickingLoop *mLoop;
        vortex::ShaderProgram *mShader;
        int *mId;
    };


    class GetLeafMeshNodeSelected : public vortex::SceneGraph::VisitorOperation {
    public:
        GetLeafMeshNodeSelected(vortex::SceneGraph::LeafMeshNode **node, int *meshI);
        void operator()(vortex::SceneGraph::Node *theNode);
    private:
        vortex::SceneGraph::LeafMeshNode **mNode;
        int *mMeshI;
    };

    /* On conserve les pointeurs permettant d'accéder aux informations sur l'objet cliqué
     * ce qui permet de ne plus avoir à parcourir le graphe */
    vortex::SceneGraph::LeafMeshNode *mNodePicked;
    int mMeshI;
=======
    class MeshVectorBuilder : public vortex::SceneGraph::VisitorOperation {
    public:
        typedef std::vector<vortex::Mesh::MeshPtr> MeshVector;

        MeshVectorBuilder(vortex::SceneGraph *sceneGraph, MeshVector *meshes);
        void operator()(vortex::SceneGraph::Node *theNode);
    private:
        vortex::SceneGraph *mSceneGraph;
        MeshVector *mMeshes;
    };

    MeshVectorBuilder::MeshVector mPickingMeshes;

    glm::vec3 idToColor(int id);
    int colorToId(glm::vec3 color);

>>>>>>> 0a49773ec341312329eebc1d2aff39cd6d5a0756
};

#endif // MATRENDERER_H
