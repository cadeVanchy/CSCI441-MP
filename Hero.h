#ifndef A3_HERO_H
#define A3_HERO_H

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Hierarchical drow hero
// Root pivot is the ground between the feet
// faces +Z at heading 0

class Hero {
public:
    Hero( GLuint shaderProgramHandle, GLint mvpMtxUniformLocation, GLint modelMtxUniformLocation, GLint normalMtxUniformLocation, GLint materialColorUniformLocation, GLfloat worldHalfSize );

    void setProgramUniformLocations( GLuint shaderProgramHandle, GLint mvpMtxUniformLocation, GLint modelMtxUniformLocation, GLint normalMtxUniformLocation, GLint materialColorUniformLocation );

    void draw( const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;

    void update();

    void moveForward( GLfloat distance );
    void moveBackward( GLfloat distance );
    void turn( GLfloat angle );
    void setMoving( bool moving ) { _isMoving = moving; }

    [[nodiscard]] glm::vec3 getPosition() const { return _position; }
    [[nodiscard]] GLfloat getHeading() const { return _heading; }
    [[nodiscard]] glm::vec3 getForward() const;
    [[nodiscard]] glm::vec3 getEyePosition() const;

private:
    glm::vec3 _position;
    GLfloat _heading;
    GLfloat _worldHalfSize;

    bool _isMoving;
    GLfloat _stepDirection;
    GLfloat _walkPhase;
    GLfloat _walkBlend;
    GLuint _frameCount;

    struct HeroPose {
        GLfloat bob;
        GLfloat legSwing;
        GLfloat armSwing;
        GLfloat capeLift;
        GLfloat headTilt;
        GLfloat hairLag;
        GLfloat eyeOpen;
    };
    [[nodiscard]] HeroPose _computePose() const;

    void _clampToWorld();

    [[nodiscard]] glm::mat4 _computeRootMatrix() const;

    void _drawHips( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawLeg( glm::mat4 modelMtx, GLfloat side, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawTorso( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawArm( glm::mat4 modelMtx, GLfloat side, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawCape( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawHead( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawEar( glm::mat4 modelMtx, GLfloat side, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawEye( glm::mat4 modelMtx, GLfloat side, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawCirclet( glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawHair( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawBangs( glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawSideLock( glm::mat4 modelMtx, GLfloat side, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawBackLock( glm::mat4 modelMtx, GLfloat xOffset, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawCapeClasp( glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawFacetedGem( glm::mat4 modelMtx, GLfloat radius, GLfloat halfHeight, GLint facets, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;

    void _drawBox( const glm::mat4& modelMtx, const glm::vec3& size, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawSphere( const glm::mat4& modelMtx, GLfloat radius, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx, const glm::vec3& stretch = glm::vec3(1.0f) ) const;
    void _drawCylinder( const glm::mat4& modelMtx, GLfloat baseRadius, GLfloat topRadius, GLfloat height, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;
    void _drawCone( const glm::mat4& modelMtx, GLfloat baseRadius, GLfloat height, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx, GLint slices = 16, const glm::vec3& stretch = glm::vec3(1.0f) ) const;

    void _sendColor( const glm::vec3& color ) const;
    void _computeAndSendMatrixUniforms( const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const;

    GLuint _shaderProgramHandle;
    struct ShaderProgramUniformLocations {
        GLint mvpMtx;
        GLint modelMtx;
        GLint normalMtx;
        GLint materialColor;
    } _shaderProgramUniformLocations;

    // per-frame animation rates (tuned for 60 FPS)
    static constexpr GLfloat WALK_PHASE_STEP = 0.133f;
    static constexpr GLfloat WALK_BLEND_RATE = 0.133f;
    static constexpr GLuint  BLINK_PERIOD    = 240;
    static constexpr GLuint  BLINK_LENGTH    = 9;

    static constexpr GLfloat HERO_SCALE    = 1.0f;

    static constexpr GLfloat HIP_HEIGHT    = 0.95f;
    static constexpr GLfloat LEG_LENGTH    = 0.85f;
    static constexpr GLfloat HIP_WIDTH     = 0.11f;
    static constexpr GLfloat TORSO_HEIGHT  = 0.6f;
    static constexpr GLfloat SHOULDER_WIDTH = 0.26f;
    static constexpr GLfloat ARM_LENGTH    = 0.6f;
    static constexpr GLfloat HEAD_RADIUS   = 0.16f;

    static constexpr glm::vec3 SKIN   = {0.30f, 0.27f, 0.40f};
    static constexpr glm::vec3 HAIR   = {0.92f, 0.93f, 0.98f};
    static constexpr glm::vec3 EYES   = {0.95f, 0.15f, 0.20f};
    static constexpr glm::vec3 DARK   = {0.05f, 0.04f, 0.08f};
    static constexpr glm::vec3 TUNIC  = {0.20f, 0.10f, 0.32f};
    static constexpr glm::vec3 SILVER = {0.70f, 0.74f, 0.85f};
    static constexpr glm::vec3 CAPE   = {0.06f, 0.10f, 0.16f};
    static constexpr glm::vec3 GEM    = {0.80f, 0.20f, 0.90f};
    static constexpr glm::vec3 GEM_DEEP = {0.45f, 0.08f, 0.55f};
    static constexpr glm::vec3 GLINT  = {1.00f, 1.00f, 1.00f};
};

#endif