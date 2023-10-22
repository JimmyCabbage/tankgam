#include "Editor.h"

Editor::Editor()
    : viewport{ *this }
{
}

Editor::~Editor() = default;

Viewport& Editor::getViewport()
{
    return viewport;
}
