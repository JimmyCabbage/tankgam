#include "WorldEditorWindow.h"

WorldEditorWindow::WorldEditorWindow(QWidget* parent)
    : QWidget{ parent }, editor{}
{
    mainLayout = new QVBoxLayout{ this };
    
    pushButton = new QPushButton{ "HELP", this };
    mainLayout->addWidget(pushButton);
    
    viewportWindow = new ViewportWindow{ editor.getViewport(), nullptr };
    mainLayout->addWidget(QWidget::createWindowContainer(viewportWindow));
    
    setLayout(mainLayout);
    
    resize(1024, 724);
}

WorldEditorWindow::~WorldEditorWindow() = default;
