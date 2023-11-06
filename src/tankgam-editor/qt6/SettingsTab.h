#pragma once

#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>

#include "ViewportToolType.h"

class Editor;

class SettingsTab : public QTabWidget
{
    Q_OBJECT
    
public:
    explicit SettingsTab(Editor& editor, QWidget* parent = nullptr);
    ~SettingsTab() override;
    
public slots:
    void textHighlightedTools(const QString& text);

    void textHighlightedTextures(const QString& text);
    
signals:
    void toolSelected(ViewportToolType viewportToolType);
    
    void textureSelected(std::string textureName);

private:
    Editor& editor;
    
    QVBoxLayout* generalTabLayout;
    QWidget* generalTab;
    QLabel* toolsDropdownLabel;
    QComboBox* toolsDropdown;
    QLabel* texturesDropdownLabel;
    QComboBox* texturesDropdown;
};
