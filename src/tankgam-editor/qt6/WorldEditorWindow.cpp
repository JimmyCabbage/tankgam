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
        connect(settingsTab, &SettingsTab::saveMap, this, [this]() { editor.saveMap(); });
        connect(settingsTab, &SettingsTab::loadMap, this, [this](std::string mapName) { editor.loadMap(std::move(mapName)); });
        connect(settingsTab, &SettingsTab::loadMap, this, [this](std::string mapName)
        {
            size_t lastSlash = mapName.find_last_of('/');
            lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
            
            settingsTab->updateTextboxMapName(mapName.substr(lastSlash));
        });
        connect(settingsTab, &SettingsTab::loadMap, renderWidget, &RenderWidget::renderNow);
    }
    
    resize(1600, 900);
    renderWidget->resize(924, 900);
}

WorldEditorWindow::~WorldEditorWindow() = default;
