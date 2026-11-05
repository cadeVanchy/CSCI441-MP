#include "Hero.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <CSCI441/objects.hpp>

#include <algorithm>
#include <cmath>

Hero::Hero(
    const GLuint shaderProgramHandle,
    const GLint mvpMtxUniformLocation,
    const GLint modelMtxUniformLocation,
    const GLint normalMtxUniformLocation,
    const GLint materialColorUniformLocation,
    const GLfloat worldHalfSize
) : _position( 0.0f, 0.0f, 0.0f ),
    _heading( 0.0f ),
    _worldHalfSize( worldHalfSize ),
    _isMoving( false ),
    _stepDirection( 1.0f ),
    _walkPhase( 0.0f ),
    _walkBlend( 0.0f ),
    _frameCount( 0 ),
    _shaderProgramHandle( 0 ),
    _shaderProgramUniformLocations( {-1, -1, -1, -1} )
{
    setProgramUniformLocations( shaderProgramHandle, mvpMtxUniformLocation, modelMtxUniformLocation, normalMtxUniformLocation, materialColorUniformLocation );
}

void Hero::setProgramUniformLocations( const GLuint shaderProgramHandle, const GLint mvpMtxUniformLocation, const GLint modelMtxUniformLocation, const GLint normalMtxUniformLocation, const GLint materialColorUniformLocation ) {
    _shaderProgramHandle = shaderProgramHandle;
    _shaderProgramUniformLocations = { mvpMtxUniformLocation, modelMtxUniformLocation, normalMtxUniformLocation, materialColorUniformLocation };
}

glm::vec3 Hero::getForward() const {
    return { std::sin(_heading), 0.0f, std::cos(_heading) };
}

glm::vec3 Hero::getEyePosition() const {
    const GLfloat eyeHeight = HIP_HEIGHT + TORSO_HEIGHT + 0.08f + HEAD_RADIUS * 0.9f + 0.015f;
    return _position + glm::vec3( 0.0f, eyeHeight * HERO_SCALE, 0.0f ) + getForward() * ((HEAD_RADIUS + 0.08f) * HERO_SCALE);
}

void Hero::moveForward( const GLfloat distance ) {
    _position += getForward() * distance;
    _stepDirection = 1.0f;
    _clampToWorld();
}

void Hero::moveBackward( const GLfloat distance ) {
    _position -= getForward() * distance;
    _stepDirection = -1.0f;
    _clampToWorld();
}

void Hero::turn( const GLfloat angle ) {
    _heading += angle;
    if( _heading >  glm::pi<GLfloat>() ) {
        _heading -= glm::two_pi<GLfloat>();
    }
    if( _heading < -glm::pi<GLfloat>() ) {
        _heading += glm::two_pi<GLfloat>();
    }
}

void Hero::_clampToWorld() {
    _position.x = glm::clamp( _position.x, -_worldHalfSize, _worldHalfSize );
    _position.z = glm::clamp( _position.z, -_worldHalfSize, _worldHalfSize );
}

void Hero::update() {
    _frameCount++;

    if( _isMoving ) {
        _walkPhase += WALK_PHASE_STEP * _stepDirection;
        _walkPhase = std::fmod( _walkPhase, glm::two_pi<GLfloat>() );
    }

    GLfloat target = 0.0f;
    if( _isMoving ) {
        target = 1.0f;
    }
    _walkBlend += (target - _walkBlend) * WALK_BLEND_RATE;
}

Hero::HeroPose Hero::_computePose() const {
    HeroPose p{};
    const GLfloat walkSin = std::sin( _walkPhase );
    const GLfloat idle    = 1.0f - _walkBlend;
    const GLfloat frame   = static_cast<GLfloat>( _frameCount );

    p.bob = 0.015f * std::sin( 0.04f * frame );
    p.headTilt = 0.06f  * std::sin( 0.0217f * frame );

    p.legSwing = _walkBlend * 0.55f * walkSin;
    p.armSwing = _walkBlend * 0.45f * walkSin + idle * 0.05f * std::sin( 0.0267f * frame );

    p.capeLift = 0.06f + 0.04f * std::sin( 0.0333f * frame + 0.5f ) + _walkBlend * 0.4f;
    p.hairLag  = 0.08f + 0.05f * std::sin( 0.0217f * frame - 0.9f ) + _walkBlend * 0.3f;

    const GLuint blinkFrame = _frameCount % BLINK_PERIOD;
    GLfloat blink = 0.0f;
    if( blinkFrame < BLINK_LENGTH ) {
        blink = std::sin( static_cast<GLfloat>(blinkFrame) / static_cast<GLfloat>(BLINK_LENGTH) * glm::pi<GLfloat>() );
    }
    p.eyeOpen = 1.0f - 0.9f * blink;

    return p;
}

glm::mat4 Hero::_computeRootMatrix() const {
    // root pivot: ground between the feet
    glm::mat4 rootMtx( 1.0f );
    rootMtx = glm::translate( rootMtx, _position );
    rootMtx = glm::rotate( rootMtx, _heading, CSCI441::Y_AXIS );
    rootMtx = glm::scale( rootMtx, glm::vec3( HERO_SCALE ) );
    return rootMtx;
}

void Hero::draw( const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    const HeroPose pose = _computePose();

    _drawHips( _computeRootMatrix(), pose, viewMtx, projMtx );
}

void Hero::_drawHips( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, HIP_HEIGHT, 0.0f ) );

    _drawBox( modelMtx, glm::vec3( 0.34f, 0.16f, 0.22f ), DARK, viewMtx, projMtx );

    _drawLeg( modelMtx,  1.0f, pose, viewMtx, projMtx );
    _drawLeg( modelMtx, -1.0f, pose, viewMtx, projMtx );
    _drawTorso( modelMtx, pose, viewMtx, projMtx );
}

void Hero::_drawLeg( glm::mat4 modelMtx, const GLfloat side, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    modelMtx = glm::translate( modelMtx, glm::vec3( side * HIP_WIDTH, 0.0f, 0.0f ) );
    // positive X rotation swings the limb backward
    modelMtx = glm::rotate( modelMtx, side * pose.legSwing, CSCI441::X_AXIS );

    // limbs hang from their joint; cylinders grow up +Y from their base
    const glm::mat4 ankleMtx = glm::translate( modelMtx, glm::vec3( 0.0f, -LEG_LENGTH, 0.0f ) );
    _drawCylinder( ankleMtx, 0.07f, 0.09f, LEG_LENGTH, DARK, viewMtx, projMtx );

    _drawBox( glm::translate( ankleMtx, glm::vec3( 0.0f, -0.04f, 0.05f ) ),
              glm::vec3( 0.14f, 0.12f, 0.26f ), SILVER, viewMtx, projMtx );
}

void Hero::_drawTorso( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, pose.bob, 0.0f ) );

    _drawBox( glm::translate( modelMtx, glm::vec3( 0.0f, TORSO_HEIGHT / 2.0f, 0.0f ) ),
              glm::vec3( 0.42f, TORSO_HEIGHT, 0.24f ), TUNIC, viewMtx, projMtx );
    _drawBox( glm::translate( modelMtx, glm::vec3( 0.0f, 0.04f, 0.0f ) ),
              glm::vec3( 0.44f, 0.05f, 0.26f ), SILVER, viewMtx, projMtx );
    _drawCapeClasp( glm::translate( modelMtx, glm::vec3( 0.0f, TORSO_HEIGHT - 0.1f, 0.12f ) ), viewMtx, projMtx );

    _drawArm( modelMtx,  1.0f, pose, viewMtx, projMtx );
    _drawArm( modelMtx, -1.0f, pose, viewMtx, projMtx );
    _drawCape( modelMtx, pose, viewMtx, projMtx );
    _drawHead( modelMtx, pose, viewMtx, projMtx );
}

void Hero::_drawArm( glm::mat4 modelMtx, const GLfloat side, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    modelMtx = glm::translate( modelMtx, glm::vec3( side * SHOULDER_WIDTH, TORSO_HEIGHT - 0.06f, 0.0f ) );
    modelMtx = glm::rotate( modelMtx, side * 0.12f, CSCI441::Z_AXIS );
    modelMtx = glm::rotate( modelMtx, -side * pose.armSwing, CSCI441::X_AXIS );

    _drawSphere( modelMtx, 0.07f, TUNIC, viewMtx, projMtx );

    const glm::mat4 wristMtx = glm::translate( modelMtx, glm::vec3( 0.0f, -ARM_LENGTH, 0.0f ) );
    _drawCylinder( wristMtx, 0.05f, 0.06f, ARM_LENGTH, TUNIC, viewMtx, projMtx );
    _drawSphere( wristMtx, 0.055f, SKIN, viewMtx, projMtx );
}

void Hero::_drawCape( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    constexpr GLfloat PANEL_LENGTH = 0.6f;
    const glm::vec3 panelSize( 0.46f, PANEL_LENGTH, 0.03f );

    modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, TORSO_HEIGHT - 0.04f, -0.14f ) );
    modelMtx = glm::rotate( modelMtx, pose.capeLift, CSCI441::X_AXIS );
    _drawBox( glm::translate( modelMtx, glm::vec3( 0.0f, -PANEL_LENGTH / 2.0f, 0.0f ) ),
              panelSize, CAPE, viewMtx, projMtx );

    modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, -PANEL_LENGTH, 0.0f ) );
    modelMtx = glm::rotate( modelMtx, 0.6f * pose.capeLift, CSCI441::X_AXIS );
    _drawBox( glm::translate( modelMtx, glm::vec3( 0.0f, -PANEL_LENGTH / 2.0f, 0.0f ) ),
              panelSize, CAPE, viewMtx, projMtx );
}

void Hero::_drawHead( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, TORSO_HEIGHT, 0.0f ) );
    _drawCylinder( modelMtx, 0.06f, 0.06f, 0.08f, SKIN, viewMtx, projMtx );

    modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, 0.08f + HEAD_RADIUS * 0.9f, 0.0f ) );
    modelMtx = glm::rotate( modelMtx, pose.headTilt, CSCI441::Z_AXIS );

    _drawSphere( modelMtx, HEAD_RADIUS, SKIN, viewMtx, projMtx, glm::vec3( 0.95f, 1.05f, 1.0f ) );

    for( const GLfloat side : { 1.0f, -1.0f } ) {
        _drawEye( modelMtx, side, pose, viewMtx, projMtx );
        _drawEar( modelMtx, side, viewMtx, projMtx );
    }
    _drawHair( modelMtx, pose, viewMtx, projMtx );
    _drawCirclet( modelMtx, viewMtx, projMtx );
}

void Hero::_drawEye( glm::mat4 modelMtx, const GLfloat side, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    modelMtx = glm::translate( modelMtx, glm::vec3( side * 0.058f, 0.015f, HEAD_RADIUS * 0.84f ) );
    modelMtx = glm::rotate( modelMtx, side * 0.25f, CSCI441::Y_AXIS );
    modelMtx = glm::rotate( modelMtx, side * 0.2f, CSCI441::Z_AXIS );

    _drawSphere( glm::translate( modelMtx, glm::vec3( 0.0f, 0.0f, -0.006f ) ),
                 0.033f, DARK, viewMtx, projMtx, glm::vec3( 1.5f, 0.8f, 0.4f ) );

    glm::mat4 browMtx = glm::translate( modelMtx, glm::vec3( 0.0f, 0.036f, 0.004f ) );
    browMtx = glm::rotate( browMtx, side * 0.15f, CSCI441::Z_AXIS );
    _drawBox( browMtx, glm::vec3( 0.06f, 0.009f, 0.012f ), HAIR, viewMtx, projMtx );

    // blink squashes the eye and everything on it
    modelMtx = glm::scale( modelMtx, glm::vec3( 1.0f, pose.eyeOpen, 1.0f ) );

    _drawSphere( modelMtx, 0.026f, EYES, viewMtx, projMtx, glm::vec3( 1.4f, 0.75f, 0.5f ) );
    _drawSphere( glm::translate( modelMtx, glm::vec3( 0.0f, 0.0f, 0.011f ) ),
                 0.011f, DARK, viewMtx, projMtx, glm::vec3( 0.5f, 1.3f, 0.5f ) );
    _drawSphere( glm::translate( modelMtx, glm::vec3( -side * 0.008f, 0.007f, 0.015f ) ),
                 0.005f, GLINT, viewMtx, projMtx );
}

void Hero::_drawCirclet( glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    constexpr GLfloat BAND_HEIGHT = 0.085f;
    constexpr GLfloat BAND_RADIUS = 0.15f;
    constexpr GLint   SEGMENTS    = 9;
    constexpr GLfloat ARC         = 2.4f;

    for( GLint i = 0; i < SEGMENTS; i++ ) {
        const GLfloat angle = -ARC / 2.0f + ARC * static_cast<GLfloat>(i) / static_cast<GLfloat>(SEGMENTS - 1);
        glm::mat4 segMtx = glm::rotate( modelMtx, angle, CSCI441::Y_AXIS );
        segMtx = glm::translate( segMtx, glm::vec3( 0.0f, BAND_HEIGHT, BAND_RADIUS ) );
        _drawBox( segMtx, glm::vec3( 0.05f, 0.014f, 0.012f ), SILVER, viewMtx, projMtx );
    }

    modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, BAND_HEIGHT, BAND_RADIUS + 0.006f ) );
    _drawSphere( modelMtx, 0.028f, SILVER, viewMtx, projMtx, glm::vec3( 1.0f, 1.3f, 0.45f ) );

    glm::mat4 gemMtx = glm::translate( modelMtx, glm::vec3( 0.0f, 0.0f, 0.01f ) );
    gemMtx = glm::rotate( gemMtx, glm::quarter_pi<GLfloat>(), CSCI441::Y_AXIS );
    gemMtx = glm::scale( gemMtx, glm::vec3( 1.0f, 1.0f, 0.6f ) );
    _drawFacetedGem( gemMtx, 0.02f, 0.032f, 4, GEM, viewMtx, projMtx );
}

void Hero::_drawHair( glm::mat4 modelMtx, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    _drawSphere( glm::translate( modelMtx, glm::vec3( 0.0f, 0.02f, -0.035f ) ),
                 HEAD_RADIUS * 1.06f, HAIR, viewMtx, projMtx );

    _drawBangs( modelMtx, viewMtx, projMtx );

    _drawSideLock( modelMtx,  1.0f, pose, viewMtx, projMtx );
    _drawSideLock( modelMtx, -1.0f, pose, viewMtx, projMtx );

    for( const GLfloat x : { -0.09f, 0.0f, 0.09f } ) {
        _drawBackLock( modelMtx, x, pose, viewMtx, projMtx );
    }
}

void Hero::_drawBangs( glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    constexpr GLfloat HAIRLINE = 0.12f;
    for( const GLfloat x : { -0.11f, -0.075f, -0.04f, 0.04f, 0.075f, 0.11f } ) {
        const GLfloat surfaceZ = std::sqrt( std::max( 0.0f, HEAD_RADIUS * HEAD_RADIUS - x * x - HAIRLINE * HAIRLINE * 0.8f ) );

        glm::mat4 bangMtx = glm::translate( modelMtx, glm::vec3( x, HAIRLINE, surfaceZ + 0.008f ) );
        GLfloat sweep = -0.2f;
        if( x > 0.0f ) {
            sweep = 0.2f;
        }
        bangMtx = glm::rotate( bangMtx, std::atan2( x, surfaceZ ), CSCI441::Y_AXIS );
        bangMtx = glm::rotate( bangMtx, glm::pi<GLfloat>() - 0.25f, CSCI441::X_AXIS );
        bangMtx = glm::rotate( bangMtx, sweep, CSCI441::Z_AXIS );
        _drawCone( bangMtx, 0.03f, 0.075f, HAIR, viewMtx, projMtx, 8, glm::vec3( 1.0f, 1.0f, 0.4f ) );
    }
}

void Hero::_drawSideLock( glm::mat4 modelMtx, const GLfloat side, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    constexpr GLfloat SEGMENT_LENGTH = 0.22f;

    modelMtx = glm::translate( modelMtx, glm::vec3( side * 0.125f, 0.06f, 0.06f ) );
    modelMtx = glm::rotate( modelMtx, side * 0.08f, CSCI441::Z_AXIS );
    modelMtx = glm::rotate( modelMtx, 0.5f * pose.hairLag, CSCI441::X_AXIS );

    modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, -SEGMENT_LENGTH, 0.0f ) );
    _drawCylinder( modelMtx, 0.025f, 0.035f, SEGMENT_LENGTH, HAIR, viewMtx, projMtx );

    modelMtx = glm::rotate( modelMtx, 0.3f * pose.hairLag, CSCI441::X_AXIS );
    modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, -SEGMENT_LENGTH, 0.0f ) );
    _drawCylinder( modelMtx, 0.008f, 0.025f, SEGMENT_LENGTH, HAIR, viewMtx, projMtx );
}

void Hero::_drawBackLock( glm::mat4 modelMtx, const GLfloat xOffset, const HeroPose& pose, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    constexpr GLfloat SEGMENT_LENGTH = 0.2f;
    constexpr GLfloat WIDTHS[3] = { 0.1f, 0.075f, 0.045f };

    modelMtx = glm::translate( modelMtx, glm::vec3( xOffset, 0.03f, -0.12f ) );
    modelMtx = glm::rotate( modelMtx, xOffset * 0.7f, CSCI441::Z_AXIS );

    for( GLint i = 0; i < 3; i++ ) {
        modelMtx = glm::rotate( modelMtx, pose.hairLag * (0.6f + 0.3f * static_cast<GLfloat>(i)), CSCI441::X_AXIS );
        _drawBox( glm::translate( modelMtx, glm::vec3( 0.0f, -SEGMENT_LENGTH / 2.0f, 0.0f ) ),
                  glm::vec3( WIDTHS[i], SEGMENT_LENGTH, 0.05f ), HAIR, viewMtx, projMtx );
        modelMtx = glm::translate( modelMtx, glm::vec3( 0.0f, -SEGMENT_LENGTH, 0.0f ) );
    }
}

void Hero::_drawCapeClasp( glm::mat4 modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    modelMtx = glm::rotate( modelMtx, glm::half_pi<GLfloat>(), CSCI441::X_AXIS );

    _drawCylinder( modelMtx, 0.055f, 0.05f, 0.015f, SILVER, viewMtx, projMtx );

    for( GLint i = 0; i < 6; i++ ) {
        glm::mat4 prongMtx = glm::translate( modelMtx, glm::vec3( 0.0f, 0.012f, 0.0f ) );
        prongMtx = glm::rotate( prongMtx, glm::two_pi<GLfloat>() * static_cast<GLfloat>(i) / 6.0f, CSCI441::Y_AXIS );
        prongMtx = glm::rotate( prongMtx, glm::half_pi<GLfloat>(), CSCI441::X_AXIS );
        _drawCone( prongMtx, 0.01f, 0.075f, SILVER, viewMtx, projMtx, 6 );
    }

    glm::mat4 gemMtx = glm::translate( modelMtx, glm::vec3( 0.0f, 0.015f, 0.0f ) );
    _drawCylinder( gemMtx, 0.034f, 0.034f, 0.01f, GEM_DEEP, viewMtx, projMtx );
    gemMtx = glm::translate( gemMtx, glm::vec3( 0.0f, 0.01f, 0.0f ) );
    _drawCone( gemMtx, 0.034f, 0.022f, GEM, viewMtx, projMtx, 8 );
}

// two low-slice cones base to base give a cut-gem look
void Hero::_drawFacetedGem( glm::mat4 modelMtx, const GLfloat radius, const GLfloat halfHeight, const GLint facets, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    _drawCone( modelMtx, radius, halfHeight, color, viewMtx, projMtx, facets );
    _drawCone( glm::rotate( modelMtx, glm::pi<GLfloat>(), CSCI441::X_AXIS ), radius, halfHeight,
               color * 0.6f, viewMtx, projMtx, facets );
}

void Hero::_drawEar( glm::mat4 modelMtx, const GLfloat side, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    modelMtx = glm::translate( modelMtx, glm::vec3( side * HEAD_RADIUS * 0.9f, 0.0f, -0.01f ) );
    modelMtx = glm::rotate( modelMtx, -side * (glm::half_pi<GLfloat>() - 0.35f), CSCI441::Z_AXIS );
    _drawCone( modelMtx, 0.045f, 0.18f, SKIN, viewMtx, projMtx, 16, glm::vec3( 1.0f, 1.0f, 0.4f ) );
}

void Hero::_drawBox( const glm::mat4& modelMtx, const glm::vec3& size, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    _computeAndSendMatrixUniforms( glm::scale( modelMtx, size ), viewMtx, projMtx );
    _sendColor( color );
    CSCI441::drawSolidCube( 1.0f );
}

void Hero::_drawSphere( const glm::mat4& modelMtx, const GLfloat radius, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx, const glm::vec3& stretch ) const {
    _computeAndSendMatrixUniforms( glm::scale( modelMtx, stretch ), viewMtx, projMtx );
    _sendColor( color );
    CSCI441::drawSolidSphere( radius, 16, 24 );
}

void Hero::_drawCylinder( const glm::mat4& modelMtx, const GLfloat baseRadius, const GLfloat topRadius, const GLfloat height, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    _computeAndSendMatrixUniforms( modelMtx, viewMtx, projMtx );
    _sendColor( color );
    CSCI441::drawSolidCylinder( baseRadius, topRadius, height, 4, 24 );
}

void Hero::_drawCone( const glm::mat4& modelMtx, const GLfloat baseRadius, const GLfloat height, const glm::vec3& color, const glm::mat4& viewMtx, const glm::mat4& projMtx, const GLint slices, const glm::vec3& stretch ) const {
    _computeAndSendMatrixUniforms( glm::scale( modelMtx, stretch ), viewMtx, projMtx );
    _sendColor( color );
    CSCI441::drawSolidCone( baseRadius, height, 4, slices );
}

void Hero::_sendColor( const glm::vec3& color ) const {
    glProgramUniform3fv( _shaderProgramHandle, _shaderProgramUniformLocations.materialColor, 1, glm::value_ptr(color) );
}

void Hero::_computeAndSendMatrixUniforms( const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx ) const {
    const glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    glProgramUniformMatrix4fv( _shaderProgramHandle, _shaderProgramUniformLocations.mvpMtx, 1, GL_FALSE, glm::value_ptr(mvpMtx) );

    glProgramUniformMatrix4fv( _shaderProgramHandle, _shaderProgramUniformLocations.modelMtx, 1, GL_FALSE, glm::value_ptr(modelMtx) );

    const glm::mat3 normalMtx = glm::mat3( glm::transpose( glm::inverse( modelMtx ) ) );
    glProgramUniformMatrix3fv( _shaderProgramHandle, _shaderProgramUniformLocations.normalMtx, 1, GL_FALSE, glm::value_ptr(normalMtx) );
}
