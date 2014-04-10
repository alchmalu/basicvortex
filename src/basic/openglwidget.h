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
#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include "vortexengine.h"
#include "scenemanager.h"
#include "camera.h"
#include "assetmanager.h"
#include "scenemanager.h"
#include <QGLWidget>
#include <QImage>

#include "myrenderer.h"

#include "renderercontrol.h"
#include "materialeditor.h"


/**
   @author Mathias Paulin <Mathias.Paulin@irit.fr>
   @author David Vanderhaeghe <vdh@irit.fr>
*/

class OpenGLWidget : public QGLWidget {
    Q_OBJECT
public:
    OpenGLWidget ( QWidget *parent );
    virtual ~OpenGLWidget();

    void clear();
    void switchAnimation(bool on);
    void switchRenderingMode(bool on);
    void setAnimationAt(int percent);
    bool loadScene(std::string fileName);

    void resetCamera();
    vortex::SceneManager *sceneManager(){ return mSceneManager; }

    void reloadShaders();
signals:
    void fpsChanged ( const QString & );
    void needRedraw ( void );

public slots:
    void animate();
    void showRenderControl();
    void toggleMaterialEditor();
    bool handleMouseEvent(const vortex::VortexEngine::MouseEvent& event);

protected:
    void initializeGL();
    void resizeGL ( int w, int h );
    void paintGL();
    virtual void mousePressEvent ( QMouseEvent * e );
    virtual void mouseReleaseEvent ( QMouseEvent * e );
    virtual void mouseMoveEvent ( QMouseEvent * e );
    virtual void wheelEvent ( QWheelEvent * e );
    virtual void keyPressEvent ( QKeyEvent * e );

private:
    enum TrackMode {TRACK_NO_TRACK = 0, TRACK_POLAR_ROTATE, TRACK_POLAR_ZOOM, TRACK_QUAKE_ROTATE, TRACK_QUAKE_WALK, TRACK_QUAKE_MOVE_XY};

    MyRenderer *mRenderer;
    vortex::Camera *mCamera;
    vortex::SceneManager* mSceneManager;
    vortex::AssetManager* mAssetManager;

    int mWidth;
    int mHeight;

    // Mouse events attributes
    bool m_isClick;
    int m_mouseX;
    int m_mouseY;
    TrackMode m_trackMode;

    void make_test();

    QTimer *mAnimationTimer;
    double mTime_start;
    double mTime_anim;

    RendererControl * mRendercontrol;
    MaterialEditor * mMaterialEditor;
};

#endif
