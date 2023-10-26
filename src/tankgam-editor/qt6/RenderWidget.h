#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

#include "Editor.h"
#include "ViewportWindow.h"

class RenderWidget : public QWidget
{
public:
    explicit RenderWidget(Editor& editor, QWidget* parent = nullptr);
    ~RenderWidget() override;

private:
    QVBoxLayout* renderLayout;
    QPushButton* refreshButton;
    ViewportWindow* viewportWindow;
};
