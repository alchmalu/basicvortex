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

    connect(ui->comboBoxTexture, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTexture(int)));

    connect(ui->spinBoxShininess, SIGNAL(valueChanged(double)), this, SLOT(updateShininess(double)));

    connect(ui->diffuseButton, SIGNAL(clicked()), this, SLOT(selectDiffuseColor()));
    connect(ui->ambientButton, SIGNAL(clicked()), this, SLOT(selectAmbientColor()));
    connect(ui->specularButton, SIGNAL(clicked()), this, SLOT(selectSpecularColor()));

    connect(this, SIGNAL(colorSelected(EColor)), this, SLOT(openColorPicker(EColor)));
    connect(&mColorPicker, SIGNAL(colorSelected(QColor)), this, SLOT(updateSelectedColor(QColor)));

    initView();
}


MaterialEditor::~MaterialEditor() {
    delete ui;
}

void MaterialEditor::initView() {
    material = mRenderer->getMaterialPicked();

    ui->comboBoxTexture->clear();

    if (material) {
        ui->ambientButton->setEnabled(true);
        ui->diffuseButton->setEnabled(true);
        ui->specularButton->setEnabled(true);
        ui->spinBoxShininess->setEnabled(true);

        specularChanged();
        diffuseChanged();
        ambientChanged();
        shininessChanged();

        for (int i = 0 ; i < material->numTexture() ; i++)
            ui->comboBoxTexture->addItem(QString(material->getTextureSemanticString(i).c_str()), i+1);
    }
    else {
        ui->ambientButton->setEnabled(false);
        ui->diffuseButton->setEnabled(false);
        ui->specularButton->setEnabled(false);
        ui->spinBoxShininess->setEnabled(false);

        ui->specularButton->setIcon(QIcon());
        ui->diffuseButton->setIcon(QIcon());
        ui->ambientButton->setIcon(QIcon());
        ui->spinBoxShininess->setValue(0);
    }

    if (ui->comboBoxTexture->count() > 0) {
        ui->comboBoxTexture->setEnabled(true);
        ui->textureButton->setEnabled(true);
    }
    else {
        ui->comboBoxTexture->setEnabled(false);
        ui->textureButton->setEnabled(false);
        ui->textureButton->setIcon(QIcon());
    }
}

void MaterialEditor::updateSelectedColor(const QColor &color) {
    glm::vec3 vec3color = QColorToVec3(color);

    switch (mColorSelected) {
        case MaterialEditor::DIFFUSE:
            material->setDiffuseColor(vec3color);
            diffuseChanged();
            break;
        case MaterialEditor::SPECULAR:;
            material->setSpecularColor(vec3color);
            specularChanged();
            break;
        case MaterialEditor::AMBIENT:
            material->setAmbientColor(vec3color);
            ambientChanged();
            break;
    }

    mGLWidget->updateGL();
}

void MaterialEditor::updateShininess(double value) {
    if (material) {
        material->setShininess(value);
        shininessChanged();
        mGLWidget->updateGL();
    }
}

void MaterialEditor::updateTexture(int index) {
    index = ui->comboBoxTexture->itemData(index).toInt();

    if (index != QVariant::Invalid) {
        Texture *texture = material->getTexture(material->getTextureSemantic(index-1));

        QImage tex(ui->textureButton->size(), QImage::Format_RGB32);
        int h = ui->textureButton->height(), w = ui->textureButton->width();
        float wu = 1.f/w, hv = 1.f/h;
        int x = 0, y = 0;
        float mu = w*wu - wu, mv = h*hv - hv;
        for (float u = 0 ; u < mu ; u += wu) {
            for (float v = 0 ; v < mv ; v += hv) {
                tex.setPixel(x, y, vec3ToQColor(glm::vec3(texture->getTexel(u,v))).rgb());
                y++;
            }
            y = 0;
            x++;
        }
        ui->textureButton->setIcon(QIcon(QPixmap::fromImage(tex)));
    }
}

void MaterialEditor::selectDiffuseColor() {
    mColorSelected = DIFFUSE;
    emit(colorSelected(mColorSelected));
}

void MaterialEditor::selectSpecularColor() {
    mColorSelected = SPECULAR;
    emit(colorSelected(mColorSelected));
}

void MaterialEditor::selectAmbientColor() {
    mColorSelected = AMBIENT;
    emit(colorSelected(mColorSelected));
}

void MaterialEditor::openColorPicker(const EColor &color) {
    mColorPicker.show();
    mColorPicker.setCurrentColor(mColors[color]);
}

void MaterialEditor::specularChanged() {
    QColor color = vec3ToQColor(material->getSpecularColor());
    mColors[SPECULAR] = color;
    QPixmap pxm(ui->specularButton->size());
    pxm.fill(mColors[SPECULAR]);
    ui->specularButton->setIcon(QIcon(pxm));
}

void MaterialEditor::diffuseChanged() {
    QColor color = vec3ToQColor(material->getDiffuseColor());
    mColors[DIFFUSE] = color;
    QPixmap pxm(ui->diffuseButton->size());
    pxm.fill(mColors[DIFFUSE]);
    ui->diffuseButton->setIcon(QIcon(pxm));
}

void MaterialEditor::ambientChanged() {
    QColor color = vec3ToQColor(material->getAmbientColor());
    mColors[AMBIENT] = color;
    QPixmap pxm(ui->ambientButton->size());
    pxm.fill(mColors[AMBIENT]);
    ui->ambientButton->setIcon(QIcon(pxm));
}

void MaterialEditor::shininessChanged() {
    ui->spinBoxShininess->setValue(material->getShininess());
}


