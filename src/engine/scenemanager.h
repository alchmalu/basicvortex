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

#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <string>

#include "assetmanager.h"
#include "scenegraph.h"
#include "assimploader.h"

#include "visitors.h"
#include "loader.h"

namespace vortex {
/**
 * Class which stores the information about scene.
 *
 * @see AssetManager
 * @see SceneGraph
 */

class SceneManager {
public:
    SceneManager(AssetManager *);
    virtual ~SceneManager();

    virtual void newScene();
    /**
     * Load the scene
     *
     * @param filename absolute path of a file to load
     * @return true if scene was load was successful, false otherwise
     */
    virtual bool loadScene(const std::string &filename);

    /**
     * Gets textual information about the last error that occurred
     */
    const char *getLastErrorString() const {
        return mLoader->getLastErrorString();
    }
    /**
     * Gets numerical information about the last error that occurred
     */
    int lastError() const {
        return mLoader->lastError();
    }

    /**
     * SceneGraph getter
     */
    SceneGraph *sceneGraph() const {
        return mSceneGraph;
    }

    /**
     * AssetManager getter
     */
    AssetManager *getAsset() {
        return mAssetManager;
    }

    void setAssetManager(AssetManager *assetManager ){mAssetManager = assetManager;}


    virtual BBox getBBox() {
        if (mSceneGraph)
            return mSceneGraph->getBBox();
        else
            return BBox();
    }
protected:

    Loader *mLoader;
    AssetManager *mAssetManager;
    SceneGraph *mSceneGraph;
};

} // namespace vortex
#endif
