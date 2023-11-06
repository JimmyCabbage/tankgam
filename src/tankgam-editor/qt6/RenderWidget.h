#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

#include "Editor.h"
#include "ViewportWindow.h"

class RenderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RenderWidget(Editor& editor, QWidget* parent = nullptr);
    ~RenderWidget() override;

signals:
    void toolSelected(ViewportToolType viewportToolType);
    
    void textureSelected(std::string textureName);

private:
    QVBoxLayout* renderLayout;
    QPushButton* refreshButton;
    ViewportWindow* viewportWindow;
};
