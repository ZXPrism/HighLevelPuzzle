#include "Shader.h"

#include <fstream>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>

#include <glad/glad.h>

void Shader::Init(const ShaderInfo &shaderInfo)
{
    static char infoLog[1024];
    int success = 1;

    _ProgramID = glCreateProgram();

    auto LoadShader = [&](const std::string &path, int shaderType) {
        std::optional<GLuint> shader;

        if (path.empty())
            return shader;

        std::string shaderSrc;
        const char *shaderSrcPtr;
        std::ifstream fin(path);
        std::stringstream ssm;

        if (!fin)
        {
            std::cout << "fatal: Unable to open file " << path << std::endl;
            return shader;
        }

        ssm << fin.rdbuf();
        shaderSrc = ssm.str();
        fin.close();

        std::regex pattern("uniform\\s+\\S+\\d?\\s+(\\S+);");
        std::smatch match;
        auto targetIter = shaderSrc.cbegin();
        while (std::regex_search(targetIter, shaderSrc.cend(), match, pattern))
        {
            targetIter = match[0].second;
            _UniformMap[match[1].str()] = 0;
            std::cout << "info: Detected uniform variable " << match[1].str() << std::endl;
        }

        shaderSrcPtr = shaderSrc.c_str();
        shader = glCreateShader(shaderType);
        glShaderSource(shader.value(), 1, &shaderSrcPtr, nullptr);

        glCompileShader(shader.value());
        glGetShaderiv(shader.value(), GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader.value(), 1024, NULL, infoLog);
            std::cout << infoLog << std::endl;
        }

        glAttachShader(_ProgramID, shader.value());

        return shader;
    };

    std::vector<std::optional<GLuint>> shaders;

    shaders.push_back(LoadShader(shaderInfo._VertexShaderPath, GL_VERTEX_SHADER));
    shaders.push_back(LoadShader(shaderInfo._FragmentShaderPath, GL_FRAGMENT_SHADER));
    shaders.push_back(LoadShader(shaderInfo._TesselControlShaderPath, GL_TESS_CONTROL_SHADER));
    shaders.push_back(LoadShader(shaderInfo._TesselEvalShaderPath, GL_TESS_EVALUATION_SHADER));
    shaders.push_back(LoadShader(shaderInfo._GeometryShaderPath, GL_GEOMETRY_SHADER));
    shaders.push_back(LoadShader(shaderInfo._ComputeShaderPath, GL_COMPUTE_SHADER));

    glLinkProgram(_ProgramID);
    glGetProgramiv(_ProgramID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(_ProgramID, 1024, NULL, infoLog);
        std::cout << infoLog << std::endl;
    }

    for (const auto &shader : shaders)
    {
        if (shader.has_value())
            glDeleteShader(shader.value());
    }

    for (auto &[name, loc] : _UniformMap)
        loc = glGetUniformLocation(_ProgramID, name.c_str());
}

Shader::Shader(const ShaderInfo &shaderInfo)
{
    Init(shaderInfo);
}

Shader::~Shader()
{
    glDeleteProgram(_ProgramID);
}

void Shader::Activate()
{
    glUseProgram(_ProgramID);
}

void Shader::SetUniform(const std::string &name, int value)
{
    glUniform1i(GetUniformLoc(name), value);
}

void Shader::SetUniform(const std::string &name, float value)
{
    glUniform1f(GetUniformLoc(name), value);
}

void Shader::SetUniform(const std::string &name, const glm::mat4 &matrix)
{
    glUniformMatrix4fv(GetUniformLoc(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetUniform(const std::string &name, const glm::vec3 &vec)
{
    glUniform3fv(GetUniformLoc(name), 1, &vec[0]);
}

int Shader::GetUniformLoc(const std::string &name)
{
    auto iter = _UniformMap.find(name);
    if (iter == _UniformMap.end())
    {
        std::cout << "warning: Unknown uniform variable: " << name << std::endl;
        return INT_MAX;
    }
    return iter->second;
}
