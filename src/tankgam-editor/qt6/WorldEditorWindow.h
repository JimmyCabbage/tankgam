#pragma once

#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>

#include "ViewportWindow.h"
#include "Editor.h"
#include "RenderWidget.h"
#include "SettingsTab.h"

class WorldEditorWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit WorldEditorWindow(QWidget* parent = nullptr);
    ~WorldEditorWindow() override;
    
private:
    Editor editor;
    
    QHBoxLayout* mainLayout;
    
    QSplitter* renderAndSettingSplitter;
    
    RenderWidget* renderWidget;
    
    SettingsTab* settingsTab;
};
