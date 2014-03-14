/*
 *   Copyright (C) 2008-2013 by Mathias Paulin, David Vanderhaeghe
 *   Mathias.Paulin@irit.fr
 *   vdh@irit.fr
 */


#include <QApplication>
#include "myrenderer.h"
#include "mainwindow.h"


int main ( int argc, char *argv[] ) {
    QApplication app ( argc, argv );
    MainWindow * mw = new MainWindow();
    mw->show();
    int result = app.exec();
    delete mw;
    return result;
}

