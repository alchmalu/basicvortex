#ifndef RENDERERCONTROL_H
#define RENDERERCONTROL_H

#include <QDialog>

class MyRenderer;
class OpenGLWidget;

namespace Ui {
    class RendererControl;
}

class RendererControl : public QDialog
{
    Q_OBJECT

public:
    explicit RendererControl(MyRenderer *renderer, QWidget *parent = 0);
    ~RendererControl();

    void setGlWidget(OpenGLWidget *widget) {
        theGLWidget = widget;
    }

    void setRenderMode(bool fill);

private slots:
    void on_displayTextureComboBox_activated(int index);
    void on_FillButton_stateChanged(int arg1);
private:
    MyRenderer *theRenderer;
    OpenGLWidget *theGLWidget;

    Ui::RendererControl *ui;
};

#endif // RENDERERCONTROL_H
