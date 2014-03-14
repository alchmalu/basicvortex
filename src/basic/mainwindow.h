/*
 *   Copyright (C) 2008-2013 by Mathias Paulin, David Vanderhaeghe
 *   Mathias.Paulin@irit.fr
 *   vdh@irit.fr
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QWidgetAction>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>

#include "openglwidget.h"

class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    void closeEvent ( QCloseEvent *event );

private slots:
    void open();
    void statusChanged ( const QString &message );

    void resetCamera();
    void reloadShaders();   
    void switchRenderingMode(bool on);

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void loadFile ( const QString &fileName );
    void setCurrentFile ( const QString &fileName );
    QString strippedName ( const QString &fullFileName );
    void reset();

    OpenGLWidget *openGLWidget;
    QString curFile;

    QMenu *fileMenu;
    QMenu *renderMenu;
    QAction *openAct;
    QAction *exitAct;
    QAction *checkResetCamera;
    QAction *checkReloadShaders;
    QAction *renderingMode;

    QSize getSize();
    QString mNameFile;
};

#endif
