#include "ChaseHero.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <CSCI441/objects.hpp>

void ChaseHero::moveForward()  { _move( MOVE_SPEED); }
void ChaseHero::moveBackward() { _move(-MOVE_SPEED); }
void ChaseHero::turnLeft()     { _heading += TURN_SPEED; }
void ChaseHero::turnRight()    { _heading -= TURN_SPEED; }

ChaseHero::ChaseHero(const GLuint shaderProgramHandle, const LightingUniformLocations& uniformLocations)
    : _shaderProgramHandle(shaderProgramHandle),
      _uniformLocations(uniformLocations),
      _position(0.0f, 0.0f, 0.0f),
      _heading(0.0f),
      _wheelAngle(0.0f) {
}

void ChaseHero::draw(const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // move to the Hero's position, then turn to face its heading
    glm::mat4 modelMtx = glm::translate(glm::mat4(1.0f), _position);
    modelMtx = glm::rotate(modelMtx, _heading, CSCI441::Y_AXIS);

    _drawBody(modelMtx, viewMtx, projMtx);
}

void ChaseHero::_drawBody(glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // purple shirt color
    const Material BODY_MATERIAL = {
        glm::vec3(0.22f, 0.20f, 0.32f),   // diffuse
        glm::vec3(0.04f, 0.04f, 0.06f),   // ambient
        glm::vec3(0.3f),                  // specular
        16.0f                             // shininess
    };

    // lift the body up so it sits above the ground (room for the wheels)
    modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, 0.75f, 0.0f));

    _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);
    _setMaterial(BODY_MATERIAL);
    CSCI441::drawSolidCube(1.0f);

    _drawTeapot(modelMtx, viewMtx, projMtx);

    _drawWheel(true, modelMtx, viewMtx, projMtx);
    _drawWheel(false, modelMtx, viewMtx, projMtx);
}

void ChaseHero::_setMaterial(const Material& material) const {
    // vec3 uniform
    glProgramUniform3fv(_shaderProgramHandle, _uniformLocations.materialDiffuse, 1, glm::value_ptr(material.diffuse));

    // float uniform
    glProgramUniform1f(_shaderProgramHandle, _uniformLocations.materialShininess, material.shininess);

    // vec3 uniform
    glProgramUniform3fv(_shaderProgramHandle, _uniformLocations.materialAmbient, 1, glm::value_ptr(material.ambient));

    // vec3 uniform
    glProgramUniform3fv(_shaderProgramHandle, _uniformLocations.materialSpecular, 1, glm::value_ptr(material.specular));

}

void ChaseHero::_computeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {

    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    glProgramUniformMatrix4fv( _shaderProgramHandle, _uniformLocations.mvpMtx, 1, GL_FALSE, glm::value_ptr(mvpMtx) );

    glm::mat3 normalMtx = glm::mat3( glm::transpose( glm::inverse( modelMtx )));
    glProgramUniformMatrix3fv( _shaderProgramHandle, _uniformLocations.normalMtx, 1, GL_FALSE, glm::value_ptr(normalMtx) );

    glProgramUniformMatrix4fv( _shaderProgramHandle, _uniformLocations.modelMtx, 1, GL_FALSE, glm::value_ptr(modelMtx) );
}

void ChaseHero::_drawTeapot(glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // shiny gray (robe color) so the specular highlight shows
    const Material TEAPOT_MATERIAL = {
        glm::vec3(0.44f, 0.41f, 0.43f),   // diffuse
        glm::vec3(0.09f, 0.08f, 0.09f),   // ambient
        glm::vec3(0.8f),                  // specular
        64.0f                             // shininess
    };

    // move from the body's center up to sit on top of the cube
    modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, 0.4f, 0.0f));
    modelMtx = glm::rotate(modelMtx, -glm::half_pi<float>(), CSCI441::Y_AXIS);   // turn spout to face +Z
    modelMtx = glm::rotate(modelMtx, -1* glm::half_pi<float>(), CSCI441::X_AXIS);   // stand the teapot upright
    modelMtx = glm::scale(modelMtx, glm::vec3(0.2f));

    _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);
    _setMaterial(TEAPOT_MATERIAL);
    CSCI441::drawSolidTeapot();
}

void ChaseHero::_drawWheel(const bool isLeft, glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // shiny dragon-scale red
    const Material WHEEL_MATERIAL = {
        glm::vec3(0.58f, 0.19f, 0.16f),   // diffuse
        glm::vec3(0.12f, 0.04f, 0.03f),   // ambient
        glm::vec3(0.6f),                  // specular
        48.0f                             // shininess
    };
    constexpr GLfloat TUBE_RADIUS = 0.1f;    // thickness of the donut
    constexpr GLfloat RING_RADIUS = 0.35f;   // center of donut to center of tube

    // left wheel on +X side, right wheel on -X side (+X is the Hero's left when facing +Z)
    const GLfloat SIDE = isLeft ? 1.0f : -1.0f;

    // move from body center to the side of the cube, down so the wheel touches the ground
    modelMtx = glm::translate(modelMtx, glm::vec3(SIDE * 0.6f, -0.3f, 0.0f));
    // spin around the axle (X axis)
    modelMtx = glm::rotate(modelMtx, _wheelAngle, CSCI441::X_AXIS);

    // spin around the axle (X axis)
    modelMtx = glm::rotate(modelMtx, _wheelAngle, CSCI441::X_AXIS);

    // spoke: a thin bar across the donut's hole so the spin is visible
    const Material SPOKE_MATERIAL = {
        glm::vec3(0.63f, 0.37f, 0.18f),   // diffuse: dragon gold
        glm::vec3(0.13f, 0.07f, 0.04f),   // ambient
        glm::vec3(0.5f),                  // specular
        32.0f                             // shininess
    };
    const glm::mat4 SPOKE_MTX = glm::scale(modelMtx, glm::vec3(0.05f, 2.0f * RING_RADIUS, 0.05f));
    _computeAndSendMatrixUniforms(SPOKE_MTX, viewMtx, projMtx);
    _setMaterial(SPOKE_MATERIAL);
    CSCI441::drawSolidCube(1.0f);


    // stand the donut up so it faces sideways
    modelMtx = glm::rotate(modelMtx, glm::half_pi<float>(), CSCI441::Y_AXIS);

    _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);
    _setMaterial(WHEEL_MATERIAL);
    CSCI441::drawSolidTorus(TUBE_RADIUS, RING_RADIUS, 16, 32);
}

glm::vec3 ChaseHero::getPosition() const { return _position; }

void ChaseHero::_move(const GLfloat distance) {
    // direction the Hero faces (heading 0 = +Z)
    const glm::vec3 FORWARD(glm::sin(_heading), 0.0f, glm::cos(_heading));

    // step, then keep inside the world
    glm::vec3 newPosition = _position + FORWARD * distance;
    newPosition.x = glm::clamp(newPosition.x, -WORLD_LIMIT, WORLD_LIMIT);
    newPosition.z = glm::clamp(newPosition.z, -WORLD_LIMIT, WORLD_LIMIT);

    // roll the wheels by how far we actually went (positive dot = moved forward)
    const GLfloat TRAVELED = glm::dot(newPosition - _position, FORWARD);
    _wheelAngle += TRAVELED / WHEEL_RADIUS;

    _position = newPosition;
}