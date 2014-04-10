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

MaterialEditor::~MaterialEditor()
{
    delete ui;
}

void MaterialEditor::updateView() {
    mMatPicked = NULL;

    if (mRenderer->getScene()->sceneGraph()) {
        GetMaterialPicked get(&mMatPicked);
        SceneGraph::PostOrderVisitor visitor(mRenderer->getScene()->sceneGraph(), get);
        visitor.go();
    }

    if (mMatPicked == NULL) {
        setColor(AMBIENT, QColor());
        setColor(SPECULAR, QColor());
        setColor(DIFFUSE, QColor());

        setShininess(0);
    }
    else if (mMatPicked){
        setColor(AMBIENT, vec3ToQColor(mMatPicked->getAmbientColor()));
        setColor(SPECULAR, vec3ToQColor(mMatPicked->getSpecularColor()));
        setColor(DIFFUSE, vec3ToQColor(mMatPicked->getDiffuseColor()));

        setShininess(mMatPicked->getShininess());
    }
}

void MaterialEditor::updateModel() {
    if (mMatPicked != NULL) {
        mMatPicked->setDiffuseColor(QColorToVec3(mColors[DIFFUSE]));
        mMatPicked->setAmbientColor(QColorToVec3(mColors[AMBIENT]));
        mMatPicked->setSpecularColor(QColorToVec3(mColors[SPECULAR]));
        mMatPicked->setShininess(mShininess);

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

MaterialEditor::GetMaterialPicked::GetMaterialPicked(Material **material) : mMaterialPicked(material) {}

void MaterialEditor::GetMaterialPicked::operator()(SceneGraph::Node *theNode) {
    if (theNode->isLeaf()) {
        SceneGraph::LeafMeshNode *leafNode = static_cast<SceneGraph::LeafMeshNode *>(theNode);
        for (int i = 0; i < leafNode->nMeshes(); ++i) {
            if ((*leafNode)[i]->isSelected()) {
                (*mMaterialPicked) = leafNode->getRenderState(i)->getMaterial();
            }
        }
    }
}
