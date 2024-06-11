#include "WorldEditorWindow.h"

#include <QFileDialog>
#include <QStatusBar>

WorldEditorWindow::WorldEditorWindow(QWidget* parent)
    : QMainWindow{ parent }, editor{}
{
    createFileMenu();
    createEditMenu();

    statusBar()->showMessage("Welcome to the editor!");

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
    }
    
    resize(1600, 900);
    viewportWindow->resize(924, 900);
}

WorldEditorWindow::~WorldEditorWindow() = default;

void WorldEditorWindow::createFileMenu()
{
    newFileAction = new QAction{ "&New", this };
    newFileAction->setShortcuts(QKeySequence::New);
    newFileAction->setStatusTip("Clear out the editor for a new map");
    connect(newFileAction, &QAction::triggered, this, [this]() { editor.newMap(); });

    openFileAction = new QAction{ "&Open", this };
    openFileAction->setShortcuts(QKeySequence::Open);
    openFileAction->setStatusTip("Open a map file from disk");
    connect(openFileAction, &QAction::triggered, this, [this]()
        {
            QString fileName = QFileDialog::getOpenFileName(this, "Open Map File", "", "Map Files (*.map)");

            statusBar()->showMessage("Opening map file: " + fileName);

            editor.openMap(fileName.toStdString());

            statusBar()->showMessage("Opened map.");
        });

    saveFileAction = new QAction{ "&Save", this };
    saveFileAction->setShortcuts(QKeySequence::Save);
    saveFileAction->setStatusTip("Save the map file with the filename it has");
    connect(saveFileAction, &QAction::triggered, this, [this]()
        {
            if (!editor.saveMap())
            {
                QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "Map Files (*.map)");

                statusBar()->showMessage("Saving map file as: " + fileName + "...");

                editor.setMapName(fileName.toStdString());

                editor.saveMap();

                statusBar()->showMessage("Saved.");
            }
            else
            {
                statusBar()->showMessage("Saved.");
            }
        });

    saveAsFileAction = new QAction{ "Save &As", this };
    saveAsFileAction->setShortcuts(QKeySequence::SaveAs);
    saveAsFileAction->setStatusTip("Save the map file with a specific file name");
    connect(saveAsFileAction, &QAction::triggered, this, [this]()
        {
            QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "Map Files (*.map)");

            statusBar()->showMessage("Saving map file as: " + fileName + "...");

            editor.setMapName(fileName.toStdString());

            editor.saveMap();

            statusBar()->showMessage("Saved.");
        });

    buildFileAction = new QAction{ "&Build Map", this };
    buildFileAction->setStatusTip("Build the map into a format readable by the engine");
    connect(buildFileAction, &QAction::triggered, this, [this]() { editor.buildMap(); });

    fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(newFileAction);
    fileMenu->addAction(openFileAction);
    fileMenu->addAction(saveFileAction);
    fileMenu->addAction(saveAsFileAction);
    fileMenu->addSeparator();
    fileMenu->addAction(buildFileAction);
}

void WorldEditorWindow::createEditMenu()
{
    editMenu = menuBar()->addMenu("&Edit");
}
