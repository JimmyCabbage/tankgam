#include "SettingsTab.h"

#include <optional>

#include <QShortcut>

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
        
        connect(toolsDropdown, &QComboBox::currentTextChanged, this, &SettingsTab::currentTextChangedTools);
        
        //create some cool shortcuts
        {
            QShortcut* selectShortcut = new QShortcut{ QKeySequence{ Qt::ALT + Qt::Key_S }, this };
            connect(selectShortcut, &QShortcut::activated, this, [this]() {
                toolsDropdown->setCurrentIndex(0); });
            
            QShortcut* brushShortcut = new QShortcut{ QKeySequence{ Qt::ALT + Qt::Key_B }, this };
            connect(brushShortcut, &QShortcut::activated, this, [this]() { toolsDropdown->setCurrentIndex(1); });
        }
        
        texturesDropdownLabel = new QLabel{ "Current Texture:", this };
        generalTabLayout->addWidget(texturesDropdownLabel);
        
        texturesDropdown = new QComboBox{ this };
        const auto textures = editor.getAvailableTextures();
        for (const auto& texture : textures)
        {
            texturesDropdown->addItem(QString::fromStdString(texture));
        }
        generalTabLayout->addWidget(texturesDropdown);
        
        connect(texturesDropdown, &QComboBox::currentTextChanged, this, &SettingsTab::currentTextChangedTextures);
    }
    addTab(generalTab, "General");
}

SettingsTab::~SettingsTab() = default;

void SettingsTab::currentTextChangedTools(const QString& text)
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

void SettingsTab::currentTextChangedTextures(const QString& text)
{
    emit textureSelected(text.toStdString());
}
