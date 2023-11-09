#include "ViewportWindow.h"

#include <QResizeEvent>
#include <QGuiApplication>

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

void ViewportWindow::toolSelected(ViewportToolType viewportToolType)
{
    viewport.setToolType(viewportToolType);
}

void ViewportWindow::textureSelected(std::string textureName)
{
    viewport.setTextureName(std::move(textureName));
}

bool ViewportWindow::event(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::UpdateRequest:
        event->accept();
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
        event->accept();
        renderNow();
    }
}

void ViewportWindow::resizeEvent(QResizeEvent* event)
{
    const QSize& size = event->size();
    viewportWidth = size.width();
    viewportHeight = size.height();
    
    event->accept();
    viewport.changeSize(viewportWidth * devicePixelRatio(), viewportHeight * devicePixelRatio());
}

void ViewportWindow::mousePressEvent(QMouseEvent* event)
{
    const auto& pos = event->pos();
    const int x = pos.x();
    const int y = pos.y();
    
    event->accept();
    viewport.clickLeftStart(x * devicePixelRatio(), y * devicePixelRatio());
}

void ViewportWindow::mouseReleaseEvent(QMouseEvent* event)
{
    const auto& pos = event->pos();
    const int x = pos.x();
    const int y = pos.y();
    
    viewport.clickLeftEnd(x * devicePixelRatio(), y * devicePixelRatio());
    
    event->accept();
    renderNow();
}

void ViewportWindow::keyPressEvent(QKeyEvent* event)
{
    bool shouldRedraw = true;
    
    const auto modifiers = QGuiApplication::keyboardModifiers();
    const bool ctrl = modifiers.testFlag(Qt::ControlModifier);
    const bool shift = modifiers.testFlag(Qt::ShiftModifier);
    
    switch (event->key())
    {
    case Qt::Key::Key_Delete:
        viewport.deleteKey();
        break;
    case Qt::Key::Key_BracketLeft:
        viewport.zoomOutCamera();
        break;
    case Qt::Key::Key_BracketRight:
        viewport.zoomInCamera();
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
        if (shift)
        {
            viewport.moveSelectedBrushes(Viewport::MoveDir::Up);
        }
        else
        {
            viewport.moveCamera(Viewport::MoveDir::Up);
        }
        break;
    case Qt::Key::Key_Down:
        if (shift)
        {
            viewport.moveSelectedBrushes(Viewport::MoveDir::Down);
        }
        else
        {
            viewport.moveCamera(Viewport::MoveDir::Down);
        }
        break;
    case Qt::Key::Key_Left:
        if (shift)
        {
            viewport.moveSelectedBrushes(Viewport::MoveDir::Left);
        }
        else
        {
            viewport.turnCamera(Viewport::TurnDir::Left);
        }
        break;
    case Qt::Key::Key_Right:
        if (shift)
        {
            viewport.moveSelectedBrushes(Viewport::MoveDir::Right);
        }
        else
        {
            viewport.turnCamera(Viewport::TurnDir::Right);
        }
        break;
    default:
        shouldRedraw = false;
        break;
    }
    
    if (shouldRedraw)
    {
        event->accept();
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
