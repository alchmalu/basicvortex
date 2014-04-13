#ifndef MATERIALEDITOR_H
#define MATERIALEDITOR_H

#include <QDialog>
#include <QColorDialog>

#include "scenegraph.h"

class MyRenderer;
class OpenGLWidget;

namespace Ui {
    class MaterialEditor;
}

class MaterialEditor : public QDialog
{
    Q_OBJECT
    
public:
    enum EColor {DIFFUSE, SPECULAR, AMBIENT, NUM_COLORS};

    explicit MaterialEditor(MyRenderer *renderer, QWidget *parent = 0);
    ~MaterialEditor();

    void setGlWidget(OpenGLWidget *widget) {
        mGLWidget = widget;
    }

    class GetMaterialByMeshName : public vortex::SceneGraph::VisitorOperation {
        public:
            GetMaterialByMeshName(vortex::Material **mMaterial, const std::string &meshName);
            void operator()(vortex::SceneGraph::Node *theNode);
        private:
            vortex::Material **mMaterialPicked;
            std::string mMeshName;
    };

signals:
    void colorSelected(const QColor &color);

public slots:
    void updateView();

private slots:
    void updateModel();

    void updateSelectedColor(const QColor &color) {
        setColor(mColorSelected, color);
        updateModel();
    }

    void updateShininess(double value) {
        setShininess(value);
        updateModel();
    }

    void selectDiffuseColor() {
        mColorSelected = DIFFUSE;
        emit(colorSelected(mColorSelected));
    }

    void selectSpecularColor() {
        mColorSelected = SPECULAR;
        emit(colorSelected(mColorSelected));
    }

    void selectAmbientColor() {
        mColorSelected = AMBIENT;
        emit(colorSelected(mColorSelected));
    }

    void openColorPicker() {
        mColorPicker.show();
        mColorPicker.setCurrentColor(mColors[mColorSelected]);
    }

private:
    OpenGLWidget *mGLWidget;
    MyRenderer *mRenderer;
    QColorDialog mColorPicker;
    Ui::MaterialEditor *ui;

    std::map<EColor, QPushButton*> mColorButtons;

    EColor mColorSelected;
    QColor mColors[NUM_COLORS];
    double mShininess;

    void setColor(EColor color, const QColor &value);
    void setShininess(double value);

    QColor vec3ToQColor(const glm::vec3& vec3) {
        return QColor::fromRgbF(vec3.x, vec3.y, vec3.z);
    }

    glm::vec3 QColorToVec3(const QColor& color) {
        return glm::vec3(color.redF(), color.greenF(), color.blueF());
    }
};

#endif // MATERIALEDITOR_H
