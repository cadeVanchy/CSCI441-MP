#ifndef LIGHTING_SHADER_H
#define LIGHTING_SHADER_H

#include "LightingUniforms.h"

#include <CSCI441/ShaderProgram.hpp>

#include <glad/gl.h>
#include <glm/glm.hpp>

/// \brief Owns the Gouraud/Phong lighting shader program.
///
/// Loads the shader, looks up every uniform location, and holds the three
/// lights (directional, point, spot). Heroes get their uniform locations from
/// getUniformLocations(); the engine calls setEyePosition() before drawing each
/// viewport. Objects without their own helpers (ground, scenery) can use
/// sendMatrices() and setMaterial().

class LightingShader {
public:
    /// \brief loads and links the lighting shader, then looks up all uniform locations
    /// \param vertexShaderPath path to the Gouraud vertex shader
    /// \param fragmentShaderPath path to the Gouraud fragment shader
    explicit LightingShader(const char* vertexShaderPath   = "shaders/gouraudShader.v.glsl", const char* fragmentShaderPath = "shaders/gouraudShader.f.glsl");

    ~LightingShader();

    //owns a GPU resource, so don't allow copies
    LightingShader(const LightingShader&) = delete;
    LightingShader& operator=(const LightingShader&) = delete;


    /// \brief makes this the active shader program, call before drawing lit objects
    void useProgram() const;

    /// \brief OpenGL handle of the shader program (what heroes' constructors take)
    GLuint getShaderProgramHandle() const;

    /// \brief the matrix + material uniform locations every hero needs
    const LightingUniformLocations& getUniformLocations() const;

    /// \brief camera position for specular highlights. Call once per viewport, with the position of the camera that viewport is drawn from.
    void setEyePosition(const glm::vec3& eyePos) const;

    /// \brief computes and sends MVP, model, and normal matrices
    void sendMatrices(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const;

    /// \brief sends a material's four Phong properties
    void setMaterial(const glm::vec3& diffuse, const glm::vec3& ambient, const glm::vec3& specular, GLfloat shininess) const;

    /// \brief sets up all three lights with the scene's starting values
    void setupDefaultLights() const;

    /// \brief sets the directional light (like the sun)
    /// \param direction the direction the light travels, world space
    /// \param color light color
    void setDirectionalLight(const glm::vec3& direction, const glm::vec3& color) const;

    /// \brief sets the point light (like a lamp)
    /// \param position light position, world space
    /// \param color light color
    /// \param attenuation (constant, linear, quadratic) falloff terms
    void setPointLight(const glm::vec3& position, const glm::vec3& color, const glm::vec3& attenuation) const;

    /// \brief sets the spotlight (like a flashlight)
    /// \param position light position, world space
    /// \param direction the direction the cone points, world space
    /// \param color light color
    /// \param attenuation (constant, linear, quadratic) falloff terms
    /// \param innerAngleDegrees full brightness inside this angle from the cone's center
    /// \param outerAngleDegrees no light outside this angle from the cone's center
    void setSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color,
                      const glm::vec3& attenuation, GLfloat innerAngleDegrees, GLfloat outerAngleDegrees) const;

private:
    /// \brief the compiled and linked Gouraud shader program
    CSCI441::ShaderProgram* _shaderProgram;

    /// \brief matrix + material locations (the contract; handed to heroes)
    LightingUniformLocations _uniformLocations;

    /// \brief camera position location (vec3 eyePos)
    GLint _eyePosLocation;

    /// \brief locations of every light uniform in the shader
    struct LightUniformLocations {
        GLint dirDirection, dirColor;
        GLint pointPosition, pointColor, pointAttenuation;
        GLint spotPosition, spotDirection, spotColor, spotAttenuation;
        GLint spotCosInnerCutoff, spotCosOuterCutoff;
    } _lightLocations;

    /// \brief looks up a uniform and logs a warning if the shader doesn't have it
    GLint _lookUp(const char* uniformName) const;

};//end class LightingShader

#endif // LIGHTING_SHADER_H