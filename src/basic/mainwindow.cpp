/*
 *   Copyright (C) 2008-2013 by Mathias Paulin, David Vanderhaeghe
 *   Mathias.Paulin@irit.fr
 *   vdh@irit.fr
 */

#include "myrenderer.h"
#include "mainwindow.h"

#include <QtGui>

#include <QGLFormat>
#include <openglwidget.h>

#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>

#include <iostream>

#include "opengl.h"


using namespace vortex::util;
MainWindow::MainWindow()  {
    fileMenu=NULL;
    renderMenu=NULL;
    QGLFormat fmt;

    fmt.setDoubleBuffer ( true );
    fmt.setDirectRendering ( true );
    fmt.setRgba ( true );
    fmt.setStencil ( true );
    fmt.setDepth ( true );
    fmt.setAlpha ( true );
    fmt.setVersion ( 3,2 );
    fmt.setProfile ( QGLFormat::CoreProfile  );

    QGLFormat::setDefaultFormat ( fmt );

    openGLWidget = new OpenGLWidget(this);

    if ( QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_3_2 ) {        
    } else {
        std::cerr << "error context OpenGL" << std::endl;
    }
    setCentralWidget ( openGLWidget );

    createActions();
    createMenus();
    createStatusBar();

    readSettings();

    setCurrentFile ( "" );

    connect ( openGLWidget, SIGNAL ( fpsChanged ( const QString & ) ), this, SLOT ( statusChanged ( const QString & ) ) );
}


MainWindow::~MainWindow() {
    delete openGLWidget;
    delete fileMenu;
    delete renderMenu;
}


void MainWindow::closeEvent ( QCloseEvent *event ) {
        writeSettings();
        event->accept();
}

void MainWindow::open() {
    static QString path("");
    QString fileName = QFileDialog::getOpenFileName ( this, "open 3d scene", path );

    if ( !fileName.isEmpty() ){
        loadFile ( fileName );
        // keep track of last valid dir
        path = QFileInfo(fileName).absolutePath();
    }
}

void MainWindow::createActions() {
    openAct = new QAction ( QIcon ( ":/fileopen.xpm" ), tr ( "&Open..." ), this );
    openAct->setShortcut ( tr ( "Ctrl+O" ) );
    openAct->setStatusTip ( tr ( "Open an existing file" ) );
    connect ( openAct, SIGNAL ( triggered() ), this, SLOT ( open() ) );

    exitAct = new QAction ( tr ( "E&xit" ), this );
    exitAct->setShortcut ( tr ( "Ctrl+Q" ) );
    exitAct->setStatusTip ( tr ( "Exit the application" ) );
    connect ( exitAct, SIGNAL ( triggered() ), this, SLOT ( close() ) );

    checkResetCamera = new QAction ( tr ( "Reset &Camera" ), this );
    checkResetCamera->setShortcut ( tr ( "Ctrl+C" ) );
    checkResetCamera->setStatusTip ( tr( "Reset Camera attributes" ) );
    connect ( checkResetCamera, SIGNAL ( triggered() ), this, SLOT ( resetCamera() ) );

    checkReloadShaders = new QAction ( tr ( "&Reload Shaders" ), this );
    checkReloadShaders->setShortcut ( tr ( "Ctrl+R" ) );
    checkReloadShaders->setStatusTip ( tr ( "Reload Shaders" ) );
    connect ( checkReloadShaders, SIGNAL ( triggered() ), this, SLOT ( reloadShaders() ) );

    renderingMode = new QAction( tr( "&Fill/Wireframe"), this );
    renderingMode->setCheckable(true);
    renderingMode->setChecked(true);
    renderingMode->setShortcut( tr( "Ctrl+F") );
    renderingMode->setStatusTip( tr( "Switch Fill/Wireframe rendering") );
    connect( renderingMode, SIGNAL( triggered(bool) ), this, SLOT( switchRenderingMode(bool) ) );
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu ( tr ( "&File" ) );
    fileMenu->addAction ( openAct );
    fileMenu->addSeparator();
    fileMenu->addAction ( exitAct );

    renderMenu = menuBar()->addMenu ( tr ( "&Rendering" ) );
    renderMenu->addAction (checkResetCamera );
    renderMenu->addSeparator();
    renderMenu->addAction ( checkReloadShaders );
    renderMenu->addSeparator();
    renderMenu->addAction ( renderingMode );
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage ( tr ( "Ready" ) );
}

void MainWindow::readSettings() {
    QSettings settings ( "VortexRenderer", "VortexRenderer" );
    QPoint pos = settings.value ( "pos", QPoint ( 200, 200 ) ).toPoint();
    QSize size = settings.value ( "size", QSize ( 800, 690 ) ).toSize();
    std::cerr <<"read settings" <<size.width()<<std::endl ;
    resize ( size );
    move ( pos );
}

void MainWindow::writeSettings() {
    std::cerr <<"write settings" <<size().width()<<std::endl ;

    QSettings settings ( "VortexRenderer", "VortexRenderer" );
    settings.setValue ( "pos", pos() );
    settings.setValue ( "size", size() );
}

void MainWindow::loadFile ( const QString &fileName ) {
    QFile file ( fileName );
    QString reason;
    if ( !file.open ( QFile::ReadOnly | QFile::Text ) ) {
        QMessageBox::warning ( this, tr ( "Application" ),
                              tr ( "Cannot read file %1:\n%2." )
                              .arg ( fileName )
                              .arg ( file.errorString() ) );
        return;
    }

    QApplication::setOverrideCursor ( Qt::WaitCursor );
    openGLWidget->makeCurrent();


    bool succeed = openGLWidget->loadScene(fileName.toStdString());

    QApplication::restoreOverrideCursor();

    if ( succeed ) {
        reset();

        setCurrentFile ( fileName );
        statusBar()->showMessage ( tr ( "File loaded" ), 2000 );

        openGLWidget->updateGL();
    } else {
        QMessageBox::warning ( this, tr ( "Application" ), tr ( "Cannot read file %1\n%2" ) .arg ( fileName ) .arg ( openGLWidget->sceneManager()->getLastErrorString() ) );
        statusBar()->showMessage ( tr ( "Error loading file" ), 2000 );
    }
}

void MainWindow::setCurrentFile ( const QString &fileName ) {
    curFile = fileName;

    QString shownName;
    if ( curFile.isEmpty() )
        shownName = "untitled.obj";
    else
        shownName = strippedName ( curFile );
    /*save the name of the file without extension*/
    mNameFile = shownName;
    mNameFile.truncate(shownName.indexOf(".", 0));
    setWindowTitle ( tr ( "%1[*] - %2" ).arg ( shownName ).arg ( tr ( "Application" ) ) );
}

QString MainWindow::strippedName ( const QString &fullFileName ) {
    return QFileInfo ( fullFileName ).fileName();
}

void MainWindow::reset(){
    resetCamera();
}

void MainWindow::statusChanged ( const QString &message ) {
    statusBar()->showMessage ( message );
}

void MainWindow::reloadShaders(){
    openGLWidget->reloadShaders();
    openGLWidget->updateGL();
}

void MainWindow::resetCamera(){
    openGLWidget->resetCamera();
    openGLWidget->updateGL();
}

void MainWindow::switchRenderingMode(bool on) {
    openGLWidget->switchRenderingMode(!on);
}
