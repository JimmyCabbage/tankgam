#include "SettingsTab.h"

#include <optional>

#include "Editor.h"

SettingsTab::SettingsTab(Editor& editor, QWidget* parent)
    : QTabWidget{ parent }, editor{ editor }
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
        
        texturesDropdownLabel = new QLabel{ "Current Texture:", this };
        generalTabLayout->addWidget(texturesDropdownLabel);
        
        texturesDropdown = new QComboBox{ this };
        const auto textures = editor.getAvailableTextures();
        for (const auto& texture : textures)
        {
            texturesDropdown->addItem(QString::fromStdString(texture));
        }
        generalTabLayout->addWidget(texturesDropdown);
        
        connect(texturesDropdown, &QComboBox::textHighlighted, this, &SettingsTab::textHighlightedTextures);
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

void SettingsTab::textHighlightedTextures(const QString& text)
{
    emit textureSelected(text.toStdString());
}
