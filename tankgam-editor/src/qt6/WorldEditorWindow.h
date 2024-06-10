#pragma once

#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>

#include "ViewportWindow.h"
#include "Editor.h"
#include "SettingsTab.h"

class WorldEditorWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit WorldEditorWindow(QWidget* parent = nullptr);
    ~WorldEditorWindow() override;
    
private:
    Editor editor;

    QAction* newFileAction;
    QAction* openFileAction;
    QAction* saveFileAction;
    QAction* saveAsFileAction;
    QAction* buildFileAction;
    QMenu* fileMenu;

    void createFileMenu();

    QMenu* editMenu;

    void createEditMenu();
    
    QHBoxLayout* mainLayout;
    
    QSplitter* viewportAndSettingSplitter;
    ViewportWindow* viewportWindow;
    SettingsTab* settingsTab;
};
