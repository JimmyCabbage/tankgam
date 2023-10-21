#include "WorldEditorWindow.h"

WorldEditorWindow::WorldEditorWindow(QWidget* parent)
    : QWidget{ parent }
{
    mainLayout = new QVBoxLayout{ this };
    
    pushButton = new QPushButton{ "HELP", this };
    mainLayout->addWidget(pushButton);
    
    viewportWindow = new ViewportWindow{ nullptr };
    mainLayout->addWidget(QWidget::createWindowContainer(viewportWindow));
    
    setLayout(mainLayout);
    
    resize(1024, 724);
}

WorldEditorWindow::~WorldEditorWindow() = default;
