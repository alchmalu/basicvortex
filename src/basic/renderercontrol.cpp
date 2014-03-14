#include "renderercontrol.h"
#include "ui_renderercontrol.h"

#include "openglwidget.h"

#include <QFileDialog>

RendererControl::RendererControl(MyRenderer *renderer, QWidget *parent) :
    QDialog(parent), theRenderer(renderer),
    ui(new Ui::RendererControl)
{
    ui->setupUi(this);
    for(int i=0; i<theRenderer->NUM_TEXTURE; ++i){
        ui->displayTextureComboBox->addItem(QString(theRenderer->getTextureName(i).c_str()));
    }
}

RendererControl::~RendererControl()
{
    delete ui;
}

void RendererControl::setRenderMode(bool fill){
    if ( fill )
        ui->FillButton->setCheckState(Qt::Checked);
    else
        ui->WireButton->setCheckState(Qt::Checked);
}


void RendererControl::on_displayTextureComboBox_activated(int index)
{
    theRenderer->setDisplayTextureId(index);
    theGLWidget->updateGL();
}


void RendererControl::on_FillButton_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
        theRenderer->setRenderingMode(0);
    else
        theRenderer->setRenderingMode(1);
    theGLWidget->updateGL();
}

