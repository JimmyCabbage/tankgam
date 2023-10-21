#include <QApplication>

#include "WorldEditorWindow.h"
#include "Plane.h"

int main(int argc, char* argv[])
{
    QApplication app{ argc, argv };
    
    WorldEditorWindow worldEditorWindow{};
    worldEditorWindow.show();
    
    return QApplication::exec();
}
