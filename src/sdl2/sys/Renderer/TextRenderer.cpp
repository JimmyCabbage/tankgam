#include "sys/Renderer/TextRenderer.h"

#include <array>
#include <string>

#include <fmt/format.h>

#include "sys/Renderer.h"

TextRenderer::TextRenderer(Renderer& renderer)
    : renderer{ renderer }
{
    //generated via Codehead's Bitmap Font Generator
    constexpr std::string_view TEXT_TEXTURE_NAME = "dev_texttexture";
    
    renderer.loadTexture(TEXT_TEXTURE_NAME, "textures/consolasfont.png");
    
    constexpr int TEXT_BOX_WIDTH = 32;
    constexpr int TEXT_BOX_HEIGHT = 32;
    constexpr int TEXT_IMAGE_SIZE = 256;
    
    const auto convertCoordinate = [](int x, int y) -> glm::vec2
    {
        constexpr auto convertRange = [](float t,
                                         float a, float b,
                                         float c, float d) -> float
        {
            return c + (((d - c) / (b - a)) * (t - a));
        };
        
        const float s = convertRange(static_cast<float>(x),
                                     0.0f, static_cast<float>(TEXT_IMAGE_SIZE),
                                     0.0f, 1.0f);
        const float t = convertRange(static_cast<float>(y),
                                     0.0f, static_cast<float>(TEXT_IMAGE_SIZE),
                                     1.0f, 0.0f);
        
        return glm::vec2{ s, t };
    };
    
    char currentChar = ' ';
    
    for (int y = 0; y < TEXT_IMAGE_SIZE; y += TEXT_BOX_HEIGHT)
    {
        for (int x = 0; x < TEXT_IMAGE_SIZE; x += TEXT_BOX_WIDTH)
        {
            const Vertex topLeft
            {
                .position = { 0.0f, 0.0f, -10.0f },
                .color = { 1.0f, 1.0f, 1.0f },
                .texCoord = convertCoordinate(x, y)
            };
            
            const Vertex bottomLeft
            {
                .position = { 0.0f, -1.0f, -10.0f },
                .color = { 1.0f, 1.0f, 1.0f },
                .texCoord = convertCoordinate(x, y + TEXT_BOX_HEIGHT)
            };
            
            const Vertex topRight
            {
                .position = { 1.0f, 0.0f, -10.0f },
                .color = { 1.0f, 1.0f, 1.0f },
                .texCoord = convertCoordinate(x + TEXT_BOX_WIDTH, y)
            };
            
            const Vertex bottomRight
            {
                .position = { 1.0f, -1.0f, -10.0f },
                .color = { 1.0f, 1.0f, 1.0f },
                .texCoord = convertCoordinate(x + TEXT_BOX_WIDTH, y + TEXT_BOX_HEIGHT)
            };
            
            const std::array<Vertex, 6> vertices
            {
                topLeft, topRight, bottomRight,
                topLeft, bottomRight, bottomLeft
            };
            
            const std::string charMeshName = fmt::format("dev_textchar_{}", currentChar);
            renderer.loadMesh(charMeshName, vertices);
            
            characterModels[currentChar++] = renderer.createModel(charMeshName, TEXT_TEXTURE_NAME, ShaderType::Text);
        }
    }
}

TextRenderer::~TextRenderer() = default;

void TextRenderer::drawText(std::string_view text, glm::vec2 position, float size)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        const auto x = static_cast<float>(i);
        const glm::vec3 scale{ size, size, 0.0f };
        const glm::vec3 translate
        {
            position.x + (x * size),
            static_cast<float>(renderer.getHeight()) - position.y,
            -1.0f
        };
        
        char c = text[i];
        if (characterModels.find(c) == characterModels.end())
        {
            if (std::isalpha(static_cast<int>(c)))
            {
                c = static_cast<char>(std::toupper(static_cast<int>(c)));
            }
            else
            {
                c = ' ';
            }
        }
        
        renderer.drawModel(*characterModels[c], scale, glm::identity<glm::quat>(), translate);
    }
}
