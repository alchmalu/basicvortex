#include "materialeditor.h"
#include "ui_materialeditor.h"

using namespace vortex;

#include "myrenderer.h"
#include "openglwidget.h"
#include "qcolordialog.h"

MaterialEditor::MaterialEditor(MyRenderer *renderer, QWidget *parent) :
    QDialog(parent), mRenderer(renderer), mColorPicker(this),
    ui(new Ui::MaterialEditor)
{
    ui->setupUi(this);

    mColorButtons[DIFFUSE] = ui->diffuseButton;
    mColorButtons[SPECULAR] = ui->specularButton;
    mColorButtons[AMBIENT] = ui->ambientButton;

    connect(ui->spinBoxShininess, SIGNAL(valueChanged(double)), this, SLOT(updateShininess(double)));

    connect(ui->diffuseButton, SIGNAL(clicked()), this, SLOT(selectDiffuseColor()));
    connect(ui->ambientButton, SIGNAL(clicked()), this, SLOT(selectAmbientColor()));
    connect(ui->specularButton, SIGNAL(clicked()), this, SLOT(selectSpecularColor()));

    connect(this, SIGNAL(colorSelected(QColor)), this, SLOT(openColorPicker()));
    connect(&mColorPicker, SIGNAL(colorSelected(QColor)), this, SLOT(updateSelectedColor(QColor)));

    updateView();
}


MaterialEditor::~MaterialEditor() {
    delete ui;
}

void MaterialEditor::updateView() {
    Material* matPicked = mRenderer->getMaterialPicked();

    if (matPicked) {
        setColor(AMBIENT, vec3ToQColor(matPicked->getAmbientColor()));
        setColor(SPECULAR, vec3ToQColor(matPicked->getSpecularColor()));
        setColor(DIFFUSE, vec3ToQColor(matPicked->getDiffuseColor()));

        setShininess(matPicked->getShininess());
    } else {
        setColor(AMBIENT, QColor());
        setColor(SPECULAR, QColor());
        setColor(DIFFUSE, QColor());

        setShininess(0);
    }
}

void MaterialEditor::updateModel() {
    Material* matPicked = mRenderer->getMaterialPicked();

    if (matPicked != NULL) {
        matPicked->setDiffuseColor(QColorToVec3(mColors[DIFFUSE]));
        matPicked->setAmbientColor(QColorToVec3(mColors[AMBIENT]));
        matPicked->setSpecularColor(QColorToVec3(mColors[SPECULAR]));
        matPicked->setShininess(mShininess);

        mGLWidget->updateGL();
    }
}

void MaterialEditor::setColor(MaterialEditor::EColor color, const QColor &value) {
    mColors[color] = value;
    mColorButtons[color]->setPalette(QPalette(value));
}

void MaterialEditor::setShininess(double value) {
    mShininess = value;
    ui->spinBoxShininess->setValue(mShininess);
}
