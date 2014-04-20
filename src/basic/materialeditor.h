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

    void initView();

signals:
    void colorSelected(const EColor &color);

private slots:
    void updateSelectedColor(const QColor &color);
    void updateShininess(double value);
    void updateTexture(int index);
    void selectDiffuseColor();
    void selectSpecularColor();
    void selectAmbientColor();
    void openColorPicker(const EColor &color);

private:
    OpenGLWidget *mGLWidget;
    MyRenderer *mRenderer;
    vortex::Material *material;
    QColorDialog mColorPicker;
    Ui::MaterialEditor *ui;

    EColor mColorSelected;
    std::map<EColor, QColor> mColors;

    void specularChanged();
    void diffuseChanged();
    void ambientChanged();
    void shininessChanged();

    QColor vec3ToQColor(const glm::vec3& vec3) {
        return QColor::fromRgbF(vec3.x, vec3.y, vec3.z);
    }

    glm::vec3 QColorToVec3(const QColor& color) {
        return glm::vec3(color.redF(), color.greenF(), color.blueF());
    }
};

#endif // MATERIALEDITOR_H
