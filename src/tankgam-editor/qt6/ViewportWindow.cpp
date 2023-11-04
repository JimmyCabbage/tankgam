#include "ViewportWindow.h"

#include <QResizeEvent>

#include <fmt/format.h>

#include "Viewport.h"

ViewportWindow::ViewportWindow(Viewport& viewport, QWindow* parent)
    : QWindow{ parent },
      viewport{ viewport },
      context{ nullptr }
{
    setSurfaceType(QSurface::SurfaceType::OpenGLSurface);
    
    setMinimumWidth(800);
    setMinimumHeight(600);
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
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setVersion(4, 2);
        format.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);
        format.setOption(QSurfaceFormat::FormatOption::DeprecatedFunctions);
        if (format.depthBufferSize() < 0)
        {
            format.setDepthBufferSize(24);
        }
        
        context = new QOpenGLContext{ this };
        context->setFormat(format);
        context->create();
        
        QObject::connect(context, &QOpenGLContext::aboutToBeDestroyed, this, &ViewportWindow::quitOpenGL);
        
        viewportWidth = width();
        viewportHeight = height();
        
        needsInitialize = true;
    }
    
    context->makeCurrent(this);
    
    if (needsInitialize)
    {
        loadGL();
        
        viewport.initGL(gl, viewportWidth * devicePixelRatio(), viewportHeight * devicePixelRatio());
    }
    
    viewport.render();
    
    context->swapBuffers(this);
}

void ViewportWindow::quitOpenGL()
{
    viewport.quitGL();
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
    viewportWidth = size.width();
    viewportHeight = size.height();
    
    viewport.changeSize(viewportWidth * devicePixelRatio(), viewportHeight * devicePixelRatio());
}

void ViewportWindow::mousePressEvent(QMouseEvent* event)
{
    const auto& pos = event->pos();
    const int x = pos.x();
    const int y = pos.y();
    
    viewport.clickLeftStart(x * devicePixelRatio(), y * devicePixelRatio());
}

void ViewportWindow::mouseReleaseEvent(QMouseEvent* event)
{
    const auto& pos = event->pos();
    const int x = pos.x();
    const int y = pos.y();
    
    viewport.clickLeftEnd(x * devicePixelRatio(), y * devicePixelRatio());
    
    renderNow();
}

void ViewportWindow::keyPressEvent(QKeyEvent* event)
{
    bool shouldRedraw = true;
    
    switch (event->key())
    {
    case Qt::Key::Key_BracketLeft:
        viewport.zoomOutCamera();
        renderNow();
        break;
    case Qt::Key::Key_BracketRight:
        viewport.zoomInCamera();
        renderNow();
        break;
    case Qt::Key::Key_W:
        viewport.moveCamera(Viewport::MoveDir::Forward);
        break;
    case Qt::Key::Key_S:
        viewport.moveCamera(Viewport::MoveDir::Back);
        break;
    case Qt::Key::Key_A:
        viewport.moveCamera(Viewport::MoveDir::Left);
        break;
    case Qt::Key::Key_D:
        viewport.moveCamera(Viewport::MoveDir::Right);
        break;
    case Qt::Key::Key_Up:
        viewport.moveCamera(Viewport::MoveDir::Up);
        break;
    case Qt::Key::Key_Down:
        viewport.moveCamera(Viewport::MoveDir::Down);
        break;
    case Qt::Key::Key_Left:
        viewport.turnCamera(Viewport::TurnDir::Left);
        break;
    case Qt::Key::Key_Right:
        viewport.turnCamera(Viewport::TurnDir::Right);
        break;
    default:
        shouldRedraw = false;
        break;
    }
    
    if (shouldRedraw)
    {
        renderNow();
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
