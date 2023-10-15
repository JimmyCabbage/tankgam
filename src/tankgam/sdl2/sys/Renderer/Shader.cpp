#include "sys/Renderer/Shader.h"

#include <stdexcept>
#include <array>
#include <string>

#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>

Shader::Shader(GladGLContext& gl,
               std::string_view vertexShaderCode, std::string_view fragmentShaderCode)
    : gl{ gl }
{
    GLuint vertex = createShader(vertexShaderCode, GL_VERTEX_SHADER);
    
    GLuint fragment = createShader(fragmentShaderCode, GL_FRAGMENT_SHADER);
    
    program = linkProgram(vertex, fragment);
    
    gl.DeleteShader(fragment);
    gl.DeleteShader(vertex);
}

Shader::~Shader()
{
    gl.DeleteShader(program);
}

void Shader::use()
{
    gl.UseProgram(program);
}

void Shader::setVec3(std::string_view name, glm::vec3 vec)
{
    const GLint loc = getUniformLoc(name);
    gl.Uniform3fv(loc, 1, glm::value_ptr(vec));
}

void Shader::setMat4(std::string_view name, glm::mat4 mat)
{
    const GLint loc = getUniformLoc(name);
    gl.UniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

GLuint Shader::createShader(std::string_view code, GLenum type)
{
    GLuint shader = gl.CreateShader(type);
    
    const GLchar* codePtr = code.data();
    GLint codeLen = static_cast<GLint>(code.size());
    
    gl.ShaderSource(shader,
                    1, &codePtr,
                    &codeLen);
    gl.CompileShader(shader);
    
    GLint success = 0;
    gl.GetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0)
    {
        std::array<GLchar, 1024> infoLog{};
        GLsizei infoLogSize = infoLog.size();
        
        gl.GetShaderInfoLog(shader, infoLogSize, nullptr, infoLog.data());
        
        std::string shaderTypeName;
        switch (type)
        {
        case GL_FRAGMENT_SHADER:
            shaderTypeName = "fragment";
            break;
        case GL_VERTEX_SHADER:
            shaderTypeName = "vertex";
            break;
        default:
            shaderTypeName = "unknown";
            break;
        }
        
        throw std::runtime_error{ fmt::format("{} shader compile err:\n{}", shaderTypeName, infoLog.data()) };
    }
    
    return shader;
}

GLuint Shader::linkProgram(GLuint vertex, GLuint fragment)
{
    GLuint program = gl.CreateProgram();
    gl.AttachShader(program, vertex);
    gl.AttachShader(program, fragment);
    gl.LinkProgram(program);
    
    GLint success = 0;
    gl.GetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == 0)
    {
        std::array<GLchar, 1024> infoLog{};
        GLsizei infoLogSize = infoLog.size();
        
        gl.GetProgramInfoLog(program, infoLogSize, nullptr, infoLog.data());
        
        throw std::runtime_error{ fmt::format("shader program linking err:\n{}", infoLog.data()) };
    }
    
    return program;
}

GLint Shader::getUniformLoc(std::string_view name)
{
    return gl.GetUniformLocation(program, name.data());
}
