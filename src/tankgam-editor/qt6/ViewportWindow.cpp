#include "ViewportWindow.h"

#include <QResizeEvent>

#include "Viewport.h"

ViewportWindow::ViewportWindow(Viewport& viewport, QWindow* parent)
    : QWindow{ parent },
      viewport{ viewport },
      context{ nullptr }
{
    setSurfaceType(QSurface::SurfaceType::OpenGLSurface);
}

ViewportWindow::~ViewportWindow()
{
    viewport.removeGL();
}

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
        
        viewport.initGL(gl, width, height);
    }
    
    viewport.render();
    
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
    
    viewport.changeSize(width, height);
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
