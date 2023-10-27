#include "WorldEditorWindow.h"

WorldEditorWindow::WorldEditorWindow(QWidget* parent)
    : QWidget{ parent }, editor{}
{
    mainLayout = new QHBoxLayout{ this };
    setLayout(mainLayout);
    
    renderAndSettingSplitter = new QSplitter;
    mainLayout->addWidget(renderAndSettingSplitter);
    {
        //render stuff
        renderWidget = new RenderWidget{ editor };
        renderAndSettingSplitter->addWidget(renderWidget);
        
        //settings stuff
        settingsTab = new SettingsTab;
        renderAndSettingSplitter->addWidget(settingsTab);
        
        connect(settingsTab, &SettingsTab::toolSelected, renderWidget, &RenderWidget::toolSelected);
    }
    
    resize(1600, 900);
}

WorldEditorWindow::~WorldEditorWindow() = default;
