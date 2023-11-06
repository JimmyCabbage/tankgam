#include "RenderWidget.h"

RenderWidget::RenderWidget(Editor& editor, QWidget* parent)
    : QWidget{ parent }
{
    renderLayout = new QVBoxLayout;
    setLayout(renderLayout);
    {
        refreshButton = new QPushButton{ "Refresh 3D View", this };
        renderLayout->addWidget(refreshButton);
        
        viewportWindow = new ViewportWindow{ editor.getViewport(), nullptr };
        renderLayout->addWidget(QWidget::createWindowContainer(viewportWindow));
        
        connect(this, &RenderWidget::toolSelected, viewportWindow, &ViewportWindow::toolSelected);
        connect(this, &RenderWidget::textureSelected, viewportWindow, &ViewportWindow::textureSelected);
        
        connect(refreshButton, &QPushButton::pressed, viewportWindow, &ViewportWindow::renderNow);
    }
}

RenderWidget::~RenderWidget() = default;
