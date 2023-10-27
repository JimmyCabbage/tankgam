#include "SettingsTab.h"

#include <optional>

SettingsTab::SettingsTab(QWidget* parent)
    : QTabWidget{ parent }
{
    generalTabLayout = new QVBoxLayout{ this };
    generalTabLayout->setAlignment(Qt::AlignTop);
    
    generalTab = new QWidget{ this };
    generalTab->setLayout(generalTabLayout);
    {
        toolsDropdownLabel = new QLabel{ "Current Tool:", this };
        generalTabLayout->addWidget(toolsDropdownLabel);
        
        toolsDropdown = new QComboBox{ this };
        toolsDropdown->addItem("Select");
        toolsDropdown->addItem("Brush");
        generalTabLayout->addWidget(toolsDropdown);
        
        connect(toolsDropdown, &QComboBox::textHighlighted, this, &SettingsTab::textHighlightedTools);
    }
    addTab(generalTab, "General");
}

SettingsTab::~SettingsTab() = default;

void SettingsTab::textHighlightedTools(const QString& text)
{
    std::optional<ViewportToolType> toolType;
    
    if (text == "Select")
    {
        toolType = ViewportToolType::Select;
    }
    else if (text == "Brush")
    {
        toolType = ViewportToolType::Brush;
    }
    
    if (toolType.has_value())
    {
        emit toolSelected(toolType.value());
    }
}
