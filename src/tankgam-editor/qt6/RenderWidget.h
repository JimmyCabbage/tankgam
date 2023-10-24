#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

#include "Editor.h"
#include "ViewportWindow.h"

class RenderWidget : public QWidget
{
public:
    RenderWidget(Editor& editor, QWidget* parent = nullptr);
    ~RenderWidget();

private:
    QVBoxLayout* renderLayout;
    QPushButton* refreshButton;
    ViewportWindow* viewportWindow;
};
