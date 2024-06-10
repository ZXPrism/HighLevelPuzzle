#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

struct ShaderInfo
{
    std::string _VertexShaderPath;
    std::string _TesselControlShaderPath;
    std::string _TesselEvalShaderPath;
    std::string _GeometryShaderPath;
    std::string _FragmentShaderPath;
    std::string _ComputeShaderPath;
};

class Shader
{
public:
    Shader() = default;
    void Init(const ShaderInfo &shaderInfo);
    Shader(const ShaderInfo &shaderInfo);
    ~Shader();

    void Activate();

    void SetUniform(const std::string &name, int value);
    void SetUniform(const std::string &name, float value);
    void SetUniform(const std::string &name, const glm::mat4 &matrix);
    void SetUniform(const std::string &name, const glm::vec3 &vec);

private:
    int GetUniformLoc(const std::string &name);

private:
    unsigned _ProgramID;
    std::unordered_map<std::string, int> _UniformMap;
};
