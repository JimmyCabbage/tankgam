#include "SettingsTab.h"

#include <optional>

#include <QFileDialog>
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
        toolsDropdown->addItem("Select Face");
        toolsDropdown->addItem("Brush");
        generalTabLayout->addWidget(toolsDropdown);
        
        connect(toolsDropdown, &QComboBox::currentTextChanged, this, &SettingsTab::currentTextChangedTools);
        
        //create some cool shortcuts
        {
            QShortcut* selectShortcut = new QShortcut{ QKeySequence{ Qt::ALT | Qt::Key_S }, this };
            connect(selectShortcut, &QShortcut::activated, this, [this]() { toolsDropdown->setCurrentIndex(0); });
            
            QShortcut* selectFaceShortcut = new QShortcut{ QKeySequence{ Qt::ALT | Qt::Key_F }, this };
            connect(selectFaceShortcut, &QShortcut::activated, this, [this]() { toolsDropdown->setCurrentIndex(1); });
            
            QShortcut* brushShortcut = new QShortcut{ QKeySequence{ Qt::ALT | Qt::Key_B }, this };
            connect(brushShortcut, &QShortcut::activated, this, [this]() { toolsDropdown->setCurrentIndex(2); });
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
        
        buildMapButton = new QPushButton{ "Build Map", this };
        generalTabLayout->addWidget(buildMapButton);
        
        connect(buildMapButton, &QPushButton::clicked, this, &SettingsTab::buildMap);
    }
    addTab(generalTab, "General");
    
    fileTabLayout = new QVBoxLayout{ this };
    fileTabLayout->setAlignment(Qt::AlignTop);
    
    fileTab = new QWidget{ this };
    fileTab->setLayout(fileTabLayout);
    {
        mapNameLabel = new QLabel{ "Map Name:", this };
        fileTabLayout->addWidget(mapNameLabel);
        
        mapNameEntry = new QLineEdit{ "default", this };
        fileTabLayout->addWidget(mapNameEntry);
        
        connect(mapNameEntry, &QLineEdit::editingFinished, this, &SettingsTab::editingFinishedMapName);
        
        saveMapButton = new QPushButton{ "Save Map", this };
        fileTabLayout->addWidget(saveMapButton);
        
        connect(saveMapButton, &QPushButton::clicked, this, &SettingsTab::saveMap);
        
        loadMapButton = new QPushButton{ "Load Map", this };
        fileTabLayout->addWidget(loadMapButton);
        
        connect(loadMapButton, &QPushButton::clicked, this, &SettingsTab::clickedLoadMap);
    }
    addTab(fileTab, "File");
}

SettingsTab::~SettingsTab() = default;

void SettingsTab::currentTextChangedTools(const QString& text)
{
    std::optional<ViewportToolType> toolType;
    
    if (text == "Select")
    {
        toolType = ViewportToolType::Select;
    }
    else if (text == "Select Face")
    {
        toolType = ViewportToolType::SelectFace;
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

void SettingsTab::editingFinishedMapName()
{
    emit changeMapName(mapNameEntry->text().toStdString());
}

void SettingsTab::clickedLoadMap()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "Open Map File", "", "Map Files (*)");
    if (fileName.isEmpty())
    {
        return;
    }
    
    emit loadMap(fileName.toStdString());
}

void SettingsTab::updateTextboxMapName(std::string mapName)
{
    mapNameEntry->setText(QString::fromStdString(mapName));
}
