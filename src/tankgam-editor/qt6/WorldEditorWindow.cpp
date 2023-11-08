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
        settingsTab = new SettingsTab{ editor };
        renderAndSettingSplitter->addWidget(settingsTab);
        
        connect(settingsTab, &SettingsTab::toolSelected, renderWidget, &RenderWidget::toolSelected);
        connect(settingsTab, &SettingsTab::textureSelected, renderWidget, &RenderWidget::textureSelected);
        connect(settingsTab, &SettingsTab::buildMap, this, [this]() { editor.buildMap(); });
        connect(settingsTab, &SettingsTab::changeMapName, this, [this](std::string mapName) { editor.setMapName(std::move(mapName)); });
    }
    
    resize(1600, 900);
    renderWidget->resize(924, 900);
}

WorldEditorWindow::~WorldEditorWindow() = default;
