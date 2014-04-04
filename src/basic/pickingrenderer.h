#ifndef PICKINGRENDERER_H
#define PICKINGRENDERER_H

#include "renderer.h"

class PickingRenderer : public vortex::Renderer
{
public:
    PickingRenderer(vortex::SceneManager *sceneManager, int width, int height);

private:
    vortex::FBO *mFbo;
};

#endif // PICKINGRENDERER_H
