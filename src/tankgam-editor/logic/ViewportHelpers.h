#pragma once

#include <array>
#include <span>
#include <vector>
#include <utility>

#include <glm/glm.hpp>

#include <gl/Vertex.h>
#include <util/Brush.h>

#include "Viewport.h"
#include "ViewportCamera.h"

std::array<Vertex, 12> generateBorders();

glm::vec3 centerOfVerticies(std::span<const glm::vec3> verticies);

void sortVerticiesClockwise(const glm::vec3& normal, std::vector<glm::vec3>& verticies);

std::pair<std::vector<Vertex>, std::string> makeFaceVertices(const BrushFace& face, glm::vec3 color);

std::vector<std::pair<std::vector<Vertex>, std::string>> makeBrushVertices(const Brush& brush, glm::vec3 overrideColor = glm::vec3{});

ViewportCamera setupCamera(Viewport::ViewportType type);

constexpr int getSkipAxis(Viewport::ViewportType type);

std::vector<Vertex> generateGrid(Viewport::ViewportType type);

std::vector<Vertex> generateCoordinates();

glm::vec3 getPositionFromMouse(Viewport::ViewportData& viewport, glm::ivec2 viewportSize, glm::ivec2 mouse);

std::pair<glm::vec2, int> getRoundedPositionAndAxis(Viewport::ViewportType type, const glm::vec3& position);
