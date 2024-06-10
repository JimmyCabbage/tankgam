#pragma once

#include <glad/gl.h>

#include <QWindow>
#include <QOpenGLContext>

#include "ViewportToolType.h"

class Viewport;

class ViewportWindow : public QWindow
{
    Q_OBJECT
    
public:
    ViewportWindow(Viewport& viewport, QWindow* parent = nullptr);
    ~ViewportWindow() override;
    
public slots:
    void renderLater();
    void renderNow();
    
    void quitOpenGL();
    
    void toolSelected(ViewportToolType viewportToolType);
    
    void textureSelected(std::string textureName);

protected:
    bool event(QEvent* event) override;
    
    void exposeEvent(QExposeEvent* event) override;
    
    void resizeEvent(QResizeEvent* event) override;
    
    void mousePressEvent(QMouseEvent* event) override;
    
    void mouseReleaseEvent(QMouseEvent* event) override;
    
    void wheelEvent(QWheelEvent* event) override;
    
    void keyPressEvent(QKeyEvent* event) override;
    
private:
    Viewport& viewport;
    
    QOpenGLContext* context;
    GladGLContext gl;
    
    int viewportWidth;
    int viewportHeight;
    
    void loadGL();
};
