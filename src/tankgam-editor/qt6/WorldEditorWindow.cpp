#include "WorldEditorWindow.h"

WorldEditorWindow::WorldEditorWindow(QWidget* parent)
    : QWidget{ parent }, editor{}
{
    mainLayout = new QHBoxLayout{ this };
    setLayout(mainLayout);
    
    viewportAndSettingSplitter = new QSplitter;
    mainLayout->addWidget(viewportAndSettingSplitter);
    {
        //viewport stuff
        viewportWindow = new ViewportWindow{ editor.getViewport() };
        viewportAndSettingSplitter->addWidget(QWidget::createWindowContainer(viewportWindow));
        
        //settings stuff
        settingsTab = new SettingsTab{ editor };
        viewportAndSettingSplitter->addWidget(settingsTab);
        
        connect(settingsTab, &SettingsTab::refreshViewport, viewportWindow, &ViewportWindow::renderNow);
        connect(settingsTab, &SettingsTab::toolSelected, viewportWindow, &ViewportWindow::toolSelected);
        connect(settingsTab, &SettingsTab::textureSelected, viewportWindow, &ViewportWindow::textureSelected);
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
        connect(settingsTab, &SettingsTab::loadMap, viewportWindow, &ViewportWindow::renderNow);
    }
    
    resize(1600, 900);
    viewportWindow->resize(924, 900);
}

WorldEditorWindow::~WorldEditorWindow() = default;
