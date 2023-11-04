#pragma once

#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>

#include "ViewportToolType.h"

class SettingsTab : public QTabWidget
{
    Q_OBJECT
    
public:
    explicit SettingsTab(QWidget* parent = nullptr);
    ~SettingsTab() override;
    
public slots:
    void textHighlightedTools(const QString& text);
    
signals:
    void toolSelected(ViewportToolType viewportToolType);

private:
    QVBoxLayout* generalTabLayout;
    QWidget* generalTab;
    QLabel* toolsDropdownLabel;
    QComboBox* toolsDropdown;
};
