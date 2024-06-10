#include "WorldEditorWindow.h"

#include <QFileDialog>

WorldEditorWindow::WorldEditorWindow(QWidget* parent)
    : QMainWindow{ parent }, editor{}
{
    createFileMenu();
    createEditMenu();

    QWidget* mainWidget = new QWidget{};
    setCentralWidget(mainWidget);

    mainLayout = new QHBoxLayout{ mainWidget };
    setLayout(mainLayout);
    
    viewportAndSettingSplitter = new QSplitter;
    mainLayout->addWidget(viewportAndSettingSplitter);
    {
        //viewport stuff
        viewportWindow = new ViewportWindow{ editor.getViewport() };
        viewportAndSettingSplitter->addWidget(QWidget::createWindowContainer(viewportWindow));
        
        //settings stuff
        settingsTab = new SettingsTab{ editor };
        viewportAndSettingSplitter->addWidget(settingsTab);
        
        connect(settingsTab, &SettingsTab::refreshViewport, viewportWindow, &ViewportWindow::renderNow);
        connect(settingsTab, &SettingsTab::toolSelected, viewportWindow, &ViewportWindow::toolSelected);
        connect(settingsTab, &SettingsTab::textureSelected, viewportWindow, &ViewportWindow::textureSelected);
        connect(settingsTab, &SettingsTab::buildMap, this, [this]() { editor.buildMap(); });
    }
    
    resize(1600, 900);
    viewportWindow->resize(924, 900);
}

WorldEditorWindow::~WorldEditorWindow() = default;

void WorldEditorWindow::createFileMenu()
{
    newFileAction = new QAction{ "&New", this };
    newFileAction->setShortcuts(QKeySequence::New);
    newFileAction->setStatusTip("Create a new map");
    connect(newFileAction, &QAction::triggered, this, [this]() { editor.newMap(); });

    openFileAction = new QAction{ "&Open", this };
    openFileAction->setShortcuts(QKeySequence::Open);
    openFileAction->setStatusTip("Open map file");
    connect(openFileAction, &QAction::triggered, this, [this]()
        {
            QString fileName = QFileDialog::getOpenFileName(this, "Open Map File", "", "Map Files (*.map)");

            editor.openMap(fileName.toStdString());
        });

    saveFileAction = new QAction{ "&Save", this };
    saveFileAction->setShortcuts(QKeySequence::Save);
    saveFileAction->setStatusTip("Save the map file");
    connect(saveFileAction, &QAction::triggered, this, [this]()
        {
            if (!editor.saveMap())
            {
                QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "Map Files (*.map)");

                editor.setMapName(fileName.toStdString());

                editor.saveMap();
            }
        });

    saveAsFileAction = new QAction{ "Save &As", this };
    saveAsFileAction->setShortcuts(QKeySequence::SaveAs);
    saveAsFileAction->setStatusTip("Save the map file");
    connect(saveAsFileAction, &QAction::triggered, this, [this]()
        {
            QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "Map Files (*.map)");

            editor.setMapName(fileName.toStdString());

            editor.saveMap();
        });

    fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(newFileAction);
    fileMenu->addAction(openFileAction);
    fileMenu->addAction(saveFileAction);
    fileMenu->addAction(saveAsFileAction);
}

void WorldEditorWindow::createEditMenu()
{
    editMenu = menuBar()->addMenu("&Edit");
}
