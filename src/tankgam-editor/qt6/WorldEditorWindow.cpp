#include "WorldEditorWindow.h"

WorldEditorWindow::WorldEditorWindow(QWidget* parent)
    : QWidget{ parent }, editor{}
{
    mainLayout = new QVBoxLayout{ this };
    
    pushButton = new QPushButton{ "Refresh 3D View", this };
    mainLayout->addWidget(pushButton);
    
    viewportWindow = new ViewportWindow{ editor.getViewport(), nullptr };
    mainLayout->addWidget(QWidget::createWindowContainer(viewportWindow));
    
    connect(pushButton, &QPushButton::pressed, viewportWindow, &ViewportWindow::renderNow);
    
    setLayout(mainLayout);
    
    resize(1024, 724);
}

WorldEditorWindow::~WorldEditorWindow() = default;
