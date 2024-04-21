#include "sys/Renderer.h"

#include <stdexcept>
#include <span>

#include <fmt/format.h>

#include <gl/Shader.h>
#include <gl/Mesh.h>
#include <gl/Texture.h>

#include "sys/Renderer/TextRenderer.h"

#include <util/FileManager.h>

Renderer::Renderer(Console& console, FileManager& fileManager, std::string_view windowName)
    : console{ console }, fileManager{ fileManager },
      width{ 1024 }, height{ 724 },
      gl{}
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
        std::string_view err = SDL_GetError();
        throw std::runtime_error{ fmt::format("Failed to initialize the SDL2 video subsystem:\n{}", err) };
    }
    
    window = SDL_CreateWindow(windowName.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        std::string_view err = SDL_GetError();
        throw std::runtime_error{ fmt::format("Failed to create SDL2's window:\n{}", err) };
    }
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    
    context = SDL_GL_CreateContext(window);
    if (!context)
    {
        std::string_view err = SDL_GetError();
        throw std::runtime_error{ fmt::format("Failed to create an OpenGL context via SDL2:\n{}", err) };
    }
    
    SDL_GL_MakeCurrent(window, context);
    
    constexpr auto glProcAddress = [](const char* name) -> GLADapiproc
    {
        return reinterpret_cast<GLADapiproc>(SDL_GL_GetProcAddress(name));
    };
    
    if (gladLoadGLContext(&gl, glProcAddress) == 0)
    {
        throw std::runtime_error{ "Failed to get OpenGL functions" };
    }
    
    setGLSettings();
    
    createGLShaders();
    
    textRenderer = std::make_unique<TextRenderer>(*this);
}

Renderer::~Renderer()
{
    SDL_GL_DeleteContext(context);
    
    SDL_DestroyWindow(window);
    
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

bool Renderer::consumeEvent(const Event& ev)
{
    switch (ev.type)
    {
    case EventType::WindowResize:
    {
        const int w = static_cast<int>(ev.data1);
        const int h = static_cast<int>(ev.data2);
        width = w;
        height = h;
        gl.Viewport(0, 0, w, h);
    }
        return true;
    default:
        return false;
    }
}

int Renderer::getWidth() const
{
    return width;
}

int Renderer::getHeight() const
{
    return height;
}

void Renderer::beginDraw()
{
    gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    const auto widthF = static_cast<float>(width);
    const auto heightF = static_cast<float>(height);
    
    projMat = glm::perspective(glm::radians(90.0f), widthF / heightF, 0.1f, 2000.0f);
    //modelShader->setMat4("uProjView", projMat);
    //textShader->setMat4("uProjView", projMat);

    orthoProjMat = glm::ortho(0.0f, widthF, 0.0f, heightF, 0.01f, 10.0f);
    //modelShader->setMat4("uOrthoProjView", orthoProjMat);
    //textShader->setMat4("uOrthoProjView", orthoProjMat);
}

void Renderer::endDraw()
{
    SDL_GL_SwapWindow(window);
}

void Renderer::loadMesh(std::string_view meshName, std::span<const Vertex> vertices)
{
    meshes.emplace_back(gl, vertices);
    meshNames.emplace_back(meshName.data());
}

void Renderer::loadMesh(std::string_view meshName, std::string_view meshFileName)
{
    std::stringstream fileStream = fileManager.readFile(meshFileName);
    
    meshes.emplace_back(gl, fileStream);
    meshNames.emplace_back(meshName.data());
}

void Renderer::loadTexture(std::string_view textureName, std::string_view textureFileName)
{
    textureNames.emplace_back(textureName.data());
    
    const std::vector<char> textureBuffer = fileManager.readFileRaw(textureFileName);
    
    auto textureData = reinterpret_cast<const uint8_t*>(textureBuffer.data());
    textures.emplace_back(gl, std::span<const uint8_t>{ textureData, textureBuffer.size() });
}

std::unique_ptr<Model> Renderer::createModel(std::string_view meshName, std::string_view textureName, ShaderType shaderType)
{
    auto model = std::make_unique<Model>();
    model->meshIndex = findMesh(meshName);
    model->textureIndex = findTexture(textureName);
    model->shaderType = shaderType;
    
    return model;
}

std::unique_ptr<Model> Renderer::createModel(std::string_view modelFileName)
{
    if (auto it = loadedModels.find(modelFileName.data()); it != loadedModels.end())
    {
        auto model = std::make_unique<Model>();
        model->meshIndex = it->second.meshIndex;
        model->textureIndex = it->second.textureIndex;
        model->shaderType = it->second.shaderType;
        
        return model;
    }
    
    std::stringstream fileStream = fileManager.readFile(modelFileName);
    
    size_t meshIndex = -1;
    size_t textureIndex = -1;
    
    std::string line;
    while (std::getline(fileStream, line))
    {
        if (line.empty())
        {
            continue;
        }
        
        std::stringstream lineStream{line};
        
        std::string type;
        lineStream >> type;
        if (type == "mdl")
        {
            std::string meshName;
            lineStream >> meshName;
            
            meshIndex = findMesh(meshName);
        }
        else if (type == "tex")
        {
            std::string textureName;
            lineStream >> textureName;
            
            textureIndex = findTexture(textureName);
        }
    }
    
    if (meshIndex == static_cast<size_t>(-1))
    {
        throw std::runtime_error{ fmt::format("Failed to find mesh in model file {}", modelFileName) };
    }
    
    if (textureIndex == static_cast<size_t>(-1))
    {
        throw std::runtime_error{ fmt::format("Failed to find texture in model file {}", modelFileName) };
    }
    
    auto model = std::make_unique<Model>();
    model->meshIndex = meshIndex;
    model->textureIndex = textureIndex;
    model->shaderType = ShaderType::Model;
    
    return model;
}

void Renderer::drawModel(Model& model, glm::vec3 scale, glm::quat rotation, glm::vec3 translate)
{
    Shader* shader = nullptr;
    switch (model.shaderType)
    {
    case ShaderType::Model:
        shader = modelShader.get();
        break;
    case ShaderType::Text:
        shader = textShader.get();
        break;
    default:
        throw std::runtime_error{ "Tried to draw a model with an unknown shader type!" };
    }
    
    const glm::mat4 transformMat = glm::translate(glm::mat4{ 1.0f }, translate);
    const glm::mat4 rotationMat = glm::mat4_cast(rotation);
    const glm::mat4 scaleMat = glm::scale(glm::mat4{ 1.0f }, scale);
    const glm::mat4 modelMat = transformMat * rotationMat * scaleMat;
    
    shader->use();
    shader->setMat4("uProjView", projMat);
    shader->setMat4("uOrthoProjView", orthoProjMat);
    shader->setMat4("uModel", modelMat);
    
    gl.ActiveTexture(GL_TEXTURE0);
    textures[model.textureIndex].bind();
    
    meshes[model.meshIndex].draw(GL_TRIANGLES);
}

void Renderer::drawText(std::string_view text, glm::vec2 position, float size)
{
    textRenderer->drawText(text, position, size);
}

void Renderer::setGLSettings() const
{
    gl.Enable(GL_DEPTH_TEST);
    gl.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Renderer::createGLShaders()
{
    //text shader
    {
        constexpr std::string_view VERTEX_CODE = R"(#version 330 core
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec3 aColor;
                layout (location = 2) in vec3 aNormal;
                layout (location = 3) in vec2 aTexCoord;

                out vec3 vColor;
                out vec2 vTexCoord;

                uniform mat4 uProjView;
                uniform mat4 uOrthoProjView;
                uniform mat4 uModel;

                void main()
                {
                    gl_Position = uOrthoProjView * uModel * vec4(aPos, 1.0);
                    vColor = aColor;
                    vTexCoord = aTexCoord;
                })";
        
        constexpr std::string_view FRAGMENT_CODE = R"(#version 330 core
                in vec3 vColor;
                in vec2 vTexCoord;

                out vec4 FragColor;

                uniform sampler2D diffuseTexture;

                void main()
                {
                    vec4 diffuseColor = texture(diffuseTexture, vTexCoord);
                    if (diffuseColor.a < 0.5f)
                    {
                        discard;
                    }

                    FragColor = diffuseColor * vec4(vColor, 1.0f);
                })";
        
        textShader = std::make_unique<Shader>(gl, VERTEX_CODE, FRAGMENT_CODE);
    }
    
    //model shader
    {
        constexpr std::string_view VERTEX_CODE = R"(#version 330 core
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec3 aColor;
                layout (location = 2) in vec3 aNormal;
                layout (location = 3) in vec2 aTexCoord;

                out vec3 vColor;
                out vec2 vTexCoord;

                uniform mat4 uProjView;
                uniform mat4 uOrthoProjView;
                uniform mat4 uModel;

                void main()
                {
                    gl_Position = uProjView * uModel * vec4(aPos, 1.0);
                    vColor = aColor;
                    vTexCoord = aTexCoord;
                })";
        
        constexpr std::string_view FRAGMENT_CODE = R"(#version 330 core
                in vec3 vColor;
                in vec2 vTexCoord;

                out vec4 FragColor;

                uniform sampler2D diffuseTexture;

                void main()
                {
                    vec4 diffuseColor = texture(diffuseTexture, vTexCoord);

                    FragColor = diffuseColor * vec4(vColor, 1.0f);
                })";
        
        modelShader = std::make_unique<Shader>(gl, VERTEX_CODE, FRAGMENT_CODE);
    }
}

size_t Renderer::findMesh(std::string_view meshName)
{
    size_t meshIndex = -1;
    for (size_t i = 0; i < meshNames.size(); i++)
    {
        if (meshName == meshNames[i])
        {
            meshIndex = i;
            break;
        }
    }
    
    if (meshIndex != -1)
    {
        return meshIndex;
    }
    
    meshIndex = meshes.size();
    
    loadMesh(meshName, meshName);
    
    return meshIndex;
}

size_t Renderer::findTexture(std::string_view textureName)
{
    size_t textureIndex = -1;
    for (size_t i = 0; i < textureNames.size(); i++)
    {
        if (textureName == textureNames[i])
        {
            textureIndex = i;
            break;
        }
    }
    
    if (textureIndex != -1)
    {
        return textureIndex;
    }
    
    textureIndex = textures.size();
    
    loadTexture(textureName, textureName);
    
    return textureIndex;
}
