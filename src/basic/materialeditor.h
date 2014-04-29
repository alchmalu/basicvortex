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

/**
 * @brief Dialog permettant d'éditer les propriétés du materiau d'un objet (Kd, Ks, Ka, Shininess) et éventuellement
 * de visualiser ses textures si elles sont définies.
 */
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
    void openColorPicker();

private slots:
    void updateSelectedColor(const QColor &color);
    void updateShininess(double value);
    void updateTexture(int index);

    /**
     * Mémorise la couleur selectionnée et ouvre la palette de couleir
     * @see openColorPicker();
     */
    void pickDiffuseColor();
    void pickSpecularColor();
    void pickAmbientColor();

private:
    OpenGLWidget *mGLWidget;
    MyRenderer *mRenderer;

    /**
     * @brief Le modele en cours d'édition
     */
    vortex::Material *material;

    QColorDialog mColorPicker;
    Ui::MaterialEditor *ui;

    /**
     * @brief Permet de savoir qu'elle couleur est concernée par la changement via la palette de couleur
     */
    EColor mColorSelected;
    /**
     * @brief Permet d'avoir accès à la couleur QColor de chaque Ks, Kd, Ka
     */
    std::map<EColor, QColor> mColors;

    /**
     * @brief Permet de mettre à jour les informations relatives à la couleur spéculaire du material
     */
    void specularChanged();

    /**
     * @brief Permet de mettre à jour les informations relatives à la couleur diffuse du material
     */
    void diffuseChanged();

    /**
     * @brief Permet de mettre à jour les informations relatives à la couleur ambiente du material
     */
    void ambientChanged();

    /**
     * @brief Permet de mettre à jour les informations relatives au shininess du material
     */
    void shininessChanged();

    /**
     * @brief vec3ToQColor
     * @param vec3
     * @return la valeur convertie de vec3 en QColor
     */
    QColor vec3ToQColor(const glm::vec3& vec3) {
        return QColor::fromRgbF(vec3.x, vec3.y, vec3.z);
    }

    /**
     * @brief QColorToVec3
     * @param color
     * @return  la valeur convertie de QColor en vec3
     */
    glm::vec3 QColorToVec3(const QColor& color) {
        return glm::vec3(color.redF(), color.greenF(), color.blueF());
    }
};

#endif // MATERIALEDITOR_H
