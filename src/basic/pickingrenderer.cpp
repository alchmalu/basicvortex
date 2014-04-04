#include "pickingrenderer.h"

#include "fbo.h"

using namespace vortex;
using namespace vortex::util;

PickingRenderer::PickingRenderer(SceneManager *sceneManager, int width, int height) : Renderer(sceneManager) {
    glCheckError();

    mFbo = new FBO(FBO::Components(FBO::DEPTH | FBO::COLOR), width, height);

    glCheckError();

    // Restore the default framebuffer
    FBO::bindDefault();
    glAssert( glReadBuffer(GL_NONE) );

    glCheckError();
}
