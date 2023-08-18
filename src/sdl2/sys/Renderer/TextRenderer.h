#pragma once

#include <unordered_map>
#include <memory>
#include <string_view>

#include <glm/glm.hpp>

class Renderer;
struct Model;

class TextRenderer
{
public:
    explicit TextRenderer(Renderer& renderer);
    ~TextRenderer();
    
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    
    void drawText(std::string_view text, glm::vec2 position, float size);

private:
    Renderer& renderer;
    
    std::unordered_map<char, std::unique_ptr<Model>> characterModels;
};
