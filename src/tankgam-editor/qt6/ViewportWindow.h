#pragma once

#include <glad/gl.h>

#include <QWindow>
#include <QOpenGLContext>

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

protected:
    bool event(QEvent* event) override;
    
    void exposeEvent(QExposeEvent* event) override;
    
    void resizeEvent(QResizeEvent* event) override;
    
    void mousePressEvent(QMouseEvent* event) override;
    
    void mouseReleaseEvent(QMouseEvent* event) override;
    
    void keyPressEvent(QKeyEvent* event) override;
    
private:
    Viewport& viewport;
    
    QOpenGLContext* context;
    GladGLContext gl;
    
    int width;
    int height;
    
    void loadGL();
};
