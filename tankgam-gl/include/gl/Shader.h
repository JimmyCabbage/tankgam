#pragma once

#include <string_view>

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glad/gl.h>

class Shader
{
public:
    Shader(GladGLContext& gl,
           std::string_view vertexShaderCode, std::string_view fragmentShaderCode);
    ~Shader();
    
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    
    void use();
    
    void setVec3(std::string_view name, glm::vec3 vec);
    
    void setMat4(std::string_view name, glm::mat4 mat);

private:
    GladGLContext& gl;
    
    GLuint program;
    
    GLuint createShader(std::string_view code, GLenum type);
    
    GLuint linkProgram(GLuint vertex, GLuint fragment);
    
    GLint getUniformLoc(std::string_view name);
};
