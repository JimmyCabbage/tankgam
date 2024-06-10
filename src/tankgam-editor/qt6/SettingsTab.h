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
    
signals:
    //general
    void refreshViewport();
    
    void toolSelected(ViewportToolType viewportToolType);
    
    void textureSelected(std::string textureName);
    
private:
    Editor& editor;
    
    QVBoxLayout* generalTabLayout;
    QWidget* generalTab;
    QPushButton* refreshButton;
    QLabel* toolsDropdownLabel;
    QComboBox* toolsDropdown;
    QLabel* texturesDropdownLabel;
    QComboBox* texturesDropdown;
};
