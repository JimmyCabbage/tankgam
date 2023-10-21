#include <QApplication>

#include "WorldEditorWindow.h"
#include "Plane.h"

int main(int argc, char* argv[])
{
    QApplication app{ argc, argv };
    
    Plane plan1{ .normal = glm::vec3{1.0f, 0.0f, 0.0f}, .distance=10.0f };
    Plane plan2{ .normal = glm::vec3{0.0f, 1.0f, 0.0f}, .distance=4.0f };
    Plane plan3{ .normal = glm::vec3{0.0f, 0.0f, 1.0f}, .distance=3.0f };
    
    Plane::intersectPlanes(plan1, plan2, plan3);
    
    WorldEditorWindow worldEditorWindow{};
    worldEditorWindow.show();
    
    return QApplication::exec();
}
