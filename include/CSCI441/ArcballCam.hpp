#ifndef CSCI441_ARCBALL_CAM_HPP
#define CSCI441_ARCBALL_CAM_HPP

#include "PerspectiveCamera.hpp"

namespace CSCI441 {

    // orbits the look at point on a sphere
    // moveForward/moveBackward zoom the radius
    class ArcballCam final : public CSCI441::PerspectiveCamera {
    public:
        explicit ArcballCam(GLfloat aspectRatio = 1.0f, GLfloat fovy = glm::quarter_pi<GLfloat>(), GLfloat nearClipPlane = 0.001f, GLfloat farClipPlane = 1000.0f, GLfloat minRadius = 2.0f, GLfloat maxRadius = 50.0f);

        ArcballCam(const ArcballCam&) = default;
        ArcballCam& operator=(const ArcballCam&) = default;
        ArcballCam(ArcballCam&&) = default;
        ArcballCam& operator=(ArcballCam&&) = default;
        ~ArcballCam() override = default;

        void recomputeOrientation() override;
        void moveForward(GLfloat movementFactor) override;
        void moveBackward(GLfloat movementFactor) override;

    private:
        GLfloat _minRadius;
        GLfloat _maxRadius;

        void _updateArcballCameraViewMatrix();
    };
}

inline CSCI441::ArcballCam::ArcballCam(
        const GLfloat aspectRatio,
        const GLfloat fovy,
        const GLfloat nearClipPlane,
        const GLfloat farClipPlane,
        const GLfloat minRadius,
        const GLfloat maxRadius
) : PerspectiveCamera(aspectRatio, fovy, nearClipPlane, farClipPlane),
    _minRadius(minRadius),
    _maxRadius(maxRadius)
{
    mCameraRadius = 10.0f;
    recomputeOrientation();
}

inline void CSCI441::ArcballCam::recomputeOrientation() {
    // direction points from the camera in toward the look at point
    mCameraDirection.x = -glm::sin(mCameraTheta ) * glm::sin(mCameraPhi );
    mCameraDirection.y = -glm::cos(mCameraPhi );
    mCameraDirection.z =  glm::cos(mCameraTheta ) * glm::sin(mCameraPhi );

    mCameraDirection = glm::normalize(mCameraDirection );

    _updateArcballCameraViewMatrix();
}

inline void CSCI441::ArcballCam::moveForward(const GLfloat movementFactor) {
    mCameraRadius = glm::clamp(mCameraRadius - movementFactor, _minRadius, _maxRadius );
    _updateArcballCameraViewMatrix();
}

inline void CSCI441::ArcballCam::moveBackward(const GLfloat movementFactor) {
    mCameraRadius = glm::clamp(mCameraRadius + movementFactor, _minRadius, _maxRadius );
    _updateArcballCameraViewMatrix();
}

inline void CSCI441::ArcballCam::_updateArcballCameraViewMatrix() {
    setPosition(mCameraLookAtPoint - mCameraDirection * mCameraRadius );
    computeViewMatrix();
}

#endif // CSCI441_ARCBALL_CAM_HPP