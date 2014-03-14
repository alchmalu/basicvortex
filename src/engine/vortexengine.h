/***************************************************************************
 *   Copyright (C) 2011 by Mathias Paulin   *
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

#ifndef VORTEXENGINE_H
#define VORTEXENGINE_H

#include "opengl.h"
#include "scenegraph.h"
#include "scenemanager.h"
//#include "camera.h"

namespace vortex {
/**
 * Forward declaration of renderer  classes
 */
class Renderer;

class Camera;

class VortexEngine {
public:
    VortexEngine();
    VortexEngine(AssetManager *assetManager, SceneManager *sceneManager);
    virtual ~VortexEngine();

    bool loadScene(std::string fileName);
    SceneManager * sceneManager(){ return mSceneManager; }

    void init(Renderer*);

    void setViewportSize(int width, int height);

    void draw();

    /**
     * Set the renderer to use.
     * This renderer should be one of the already implemented in vortex renderer or one user
     * define by subclassing Renderer.
     * @see Renderer
     */
    void setRenderer(Renderer *renderer);

    /**
     * Get the actual renderer for the engine.
     * The engine does not manage switching between renderer other tha defaults.
     */
    Renderer * renderer();

    /**
     * Mouse event structure
     */
    class MouseEvent {
    public :
        enum Modifiers {NONE = 0x00, CONTROL = 0x01, SHIFT = 0x02, ALT = 0x04};
        enum Buttons {LEFT = 0x01, MIDDLE = 0x02, RIGHT = 0x04, WHEEL = 0x08, MOVE = 0x10};
        int button;
        bool click; // true -> click, false -> release
        int modifiers;
        int x;
        int y;
        int delta;
    };
    /**
     * Mouse event action
     */
    enum MouseAction { NO_MOUSE_ACTION, ROTATE, ZOOM, TRANSLATE, MOVE_FORWARD, LOOK_AROUND,
                       MOVE_BACKWARD, SCREEN_ROTATE, ROLL, DRIVE, SCREEN_TRANSLATE, ZOOM_ON_REGION
      };

    /** Handle mouse event. */
    bool handleMouseEvent(const MouseEvent& event);
    /** Handle keyboard event. */
    bool handleKeyEvent(char key);
    /** Set the actual scene manager. */
    void setSceneManager(vortex::SceneManager* manager) {
        mSceneManager = manager;
        //mCamera->buildMatrices(mWidth, mHeight);
    }

    /**
     * Engine configuration structure
     *
     */
    class EngineConfiguration {
    public :
        /// Rendering modes
        enum RenderingModes {RENDER_FILL, RENDER_WIREFRAME, RENDER_SELECTION};
        /// Rendering algorithms
        enum RenderingAlgorithm {FORWARD_RENDERING, DEFERRED_RENDERING};
        /// Shadow algorithms
        enum ShadowAlgorithm {NO_SHADOW, SHADOW_MAPPING, SHADOW_PCSS};

        /**
        * Default configuration
        */
        EngineConfiguration() : mRenderMode(RENDER_FILL), mAlgorithm(FORWARD_RENDERING), mShadow(NO_SHADOW) {}
        EngineConfiguration(const EngineConfiguration& configuration);
        RenderingModes mRenderMode;
        RenderingAlgorithm mAlgorithm;
        ShadowAlgorithm mShadow;
    };
    /**
     * @todo finalize and developp configuration mechanism and pass manager
     */
    void setConfiguration(const EngineConfiguration& configuration);

    /**
     * @return the actual engine configuration
     */
//    const EngineConfiguration &getConfiguration() const;

    /**
     * Specify the active renderer whether to show or not Meshes Boxes.
     */
    void showBboxes(bool show);

    /**
     * Specify the scene AssetManager to reload ShaderPrograms.
     */
    void reloadShaders();

    /**
     * Clear the SelectionRenderer.
     */
    void clearSelection();

    /**
     * Specify the SelectionRenderer the rendering mode to use.
     *
     * @param mode The rendering mode the SelectionRenderer must switch to.
     *
     * @return True if the SelectionRenderer has been able to switch rendering mode, false otherwise.
     */
    bool setPickingMode(bool mode);

    /**
     * Specify the SelectionRenderer which type of primitives are to be selected.
     *
     * @param primitive The type of primitives to be selected.
     */
    void selectPrimitive(bool primitive);

    /**
     * Reset the camera.
     */
    void resetCamera();

    /**
     * Display or hide the skeleton of an animation scene
     */
    void switchSkeleton() {
        mAssetManager->getCurrentAnimation()->getSkeleton()->setSkeletonDisplay(
            !(mAssetManager->getCurrentAnimation()->getSkeleton()->isSkeletonDisplay())
        );
    }

    /**
     * Update all animations
     * @param time Current time
     */
    bool updateAnimations(double time);

    /**
     * Update all animations
     * @param percent Current relative frame
     */
    bool updateAnimations(int percent);

protected:
    Camera *mCamera;
    int mWidth;
    int mHeight;
    SceneManager* mSceneManager;
    AssetManager* mAssetManager;
   // EngineConfiguration mEngineConfiguration;

private:
    /**
     * Renderer used by the engine.
     *
     * The rendere may be changed either by configuring the engine with its internal possibilities
     * or be subclassing the Renderer class.
     * @see Renderer
     */
    Renderer *mActiveRenderer;

};

}


#endif
