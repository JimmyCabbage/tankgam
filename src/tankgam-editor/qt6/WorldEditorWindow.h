#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>

#include "ViewportWindow.h"
#include "Editor.h"

class WorldEditorWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit WorldEditorWindow(QWidget* parent = nullptr);
    ~WorldEditorWindow() override;
    
private:
    Editor editor;
    
    QVBoxLayout* mainLayout;
    QPushButton* pushButton;
    ViewportWindow* viewportWindow;
};
