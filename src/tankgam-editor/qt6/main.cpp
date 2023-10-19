#include <QApplication>

#include "WorldEditor.h"

int main(int argc, char* argv[])
{
    QApplication app{ argc, argv };
    
    WorldEditor worldEditor{};
    worldEditor.show();
    
    return QApplication::exec();
}
