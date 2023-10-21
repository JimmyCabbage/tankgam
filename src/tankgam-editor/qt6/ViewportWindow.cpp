#include "ViewportWindow.h"

#include <QResizeEvent>

ViewportWindow::ViewportWindow(QWindow* parent)
    : QWindow{ parent },
      context{ nullptr }
{
    setSurfaceType(QSurface::SurfaceType::OpenGLSurface);
}

ViewportWindow::~ViewportWindow() = default;

void ViewportWindow::renderLater()
{
    requestUpdate();
}

void ViewportWindow::renderNow()
{
    if (!isExposed())
    {
        return;
    }
    
    bool needsInitialize = false;
    
    if (!context)
    {
        QSurfaceFormat format;
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);
        
        context = new QOpenGLContext{ this };
        context->setFormat(format);
        context->create();
        
        const QSize windowSize = size();
        width = windowSize.width();
        height = windowSize.height();
        
        needsInitialize = true;
    }
    
    context->makeCurrent(this);
    
    if (needsInitialize)
    {
        loadGL();
        
        initGL();
    }
    
    render();
    
    context->swapBuffers(this);
}

bool ViewportWindow::event(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void ViewportWindow::exposeEvent(QExposeEvent* event)
{
    Q_UNUSED(event);
    
    if (isExposed())
    {
        renderNow();
    }
}

void ViewportWindow::resizeEvent(QResizeEvent* event)
{
    const QSize& size = event->size();
    width = size.width();
    height = size.height();
    
    if (context)
    {
        gl.Viewport(0, 0, width, height);
    }
}

void ViewportWindow::loadGL()
{
    const auto glProcAddress = [](void* userptr, const char* name) -> GLADapiproc
    {
        QOpenGLContext* context = reinterpret_cast<QOpenGLContext*>(userptr);
        return reinterpret_cast<GLADapiproc>(context->getProcAddress(name));
    };
    
    if (gladLoadGLContextUserPtr(&gl, glProcAddress, context) == 0)
    {
        throw std::runtime_error{ "Failed to get OpenGL functions" };
    }
}

void ViewportWindow::initGL()
{
    gl.Enable(GL_DEPTH_TEST);
    gl.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void ViewportWindow::render()
{
    gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
