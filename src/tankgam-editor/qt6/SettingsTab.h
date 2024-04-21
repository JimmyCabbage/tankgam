#pragma once

#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>

#include "ViewportToolType.h"

class Editor;

class SettingsTab : public QTabWidget
{
    Q_OBJECT
    
public:
    explicit SettingsTab(Editor& editor, QWidget* parent = nullptr);
    ~SettingsTab() override;
    
private slots:
    //general
    void currentTextChangedTools(const QString& text);

    void currentTextChangedTextures(const QString& text);
    
    //file
    void editingFinishedMapName();
    
    void clickedLoadMap();
    
public slots:
    void updateTextboxMapName(std::string mapName);
    
signals:
    //general
    void refreshViewport();
    
    void toolSelected(ViewportToolType viewportToolType);
    
    void textureSelected(std::string textureName);

    void buildMap();
    
    //file
    void changeMapName(std::string mapName);
    
    void saveMap();
    
    void loadMap(std::string mapName);
    
private:
    Editor& editor;
    
    QVBoxLayout* generalTabLayout;
    QWidget* generalTab;
    QPushButton* refreshButton;
    QLabel* toolsDropdownLabel;
    QComboBox* toolsDropdown;
    QLabel* texturesDropdownLabel;
    QComboBox* texturesDropdown;
    QPushButton* buildMapButton;
    
    QVBoxLayout* fileTabLayout;
    QWidget* fileTab;
    QLabel* mapNameLabel;
    QLineEdit* mapNameEntry;
    QPushButton* saveMapButton;
    QPushButton* loadMapButton;
};