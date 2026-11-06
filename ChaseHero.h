#ifndef HERO3D_H
#define HERO3D_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include "LightingUniforms.h"

/// \brief Our Hero's vehicle: a cube body with a teapot on top and two donut wheels that roll as it moves
class ChaseHero {
public:

    /// \param shaderProgramHandle lighting shader program to draw with
    /// \param uniformLocations where to send matrices and material values
    ChaseHero(const GLuint shaderProgramHandle, const LightingUniformLocations& uniformLocations);

    /// \brief draws the Hero at its current position and heading
    void draw(const glm::mat4& viewMtx, const glm::mat4& projMtx) const;

    /// \brief drive forward along the heading
    void moveForward();
    /// \brief drive backward along the heading
    void moveBackward();
    /// \brief rotate heading to the left
    void turnLeft();
    /// \brief rotate heading to the right
    void turnRight();
    /// \brief current world position (for the camera to follow)
    glm::vec3 getPosition() const;

private:
    /// \brief Phong material properties for one part
    struct Material {
        glm::vec3 diffuse;
        glm::vec3 ambient;
        glm::vec3 specular;
        GLfloat shininess;
    };

    GLuint _shaderProgramHandle;
    LightingUniformLocations _uniformLocations;

    /// \brief world position of the Hero (on the ground)
    glm::vec3 _position;
    /// \brief direction the Hero faces, radians around +Y
    GLfloat _heading;
    /// \brief current spin of the wheels, radians
    GLfloat _wheelAngle;

    void _drawBody(glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const;

    void _drawTeapot(glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const;

    void _drawWheel(bool isLeft, glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const;

    /// \brief sends all four material uniforms
    void _setMaterial(const Material& material) const;
    /// \brief sends MVP, model, and normal matrices
    void _computeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const;

    /// \brief distance moved per frame
    static constexpr GLfloat MOVE_SPEED = 0.1f;
    /// \brief radians turned per frame
    static constexpr GLfloat TURN_SPEED = 0.04f;
    /// \brief outer radius of the donut wheels (tube + ring)
    static constexpr GLfloat WHEEL_RADIUS = 0.45f;
    /// \brief how far from the origin the Hero may go in X and Z
    static constexpr GLfloat WORLD_LIMIT = 54.0f;

    /// \brief moves along the heading by distance (negative = backward), stays in bounds, rolls wheels
    void _move(GLfloat distance);

};

#endif // HERO3D_H