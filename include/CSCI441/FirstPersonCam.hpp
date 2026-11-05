#ifndef CSCI441_FIRST_PERSON_CAM_HPP
#define CSCI441_FIRST_PERSON_CAM_HPP

#include "PerspectiveCamera.hpp"

namespace CSCI441 {

    // cam perspective through heros eyes
    // call follow() every frame with the eye position and facing
    class FirstPersonCam final : public CSCI441::PerspectiveCamera {
    public:
        explicit FirstPersonCam(GLfloat aspectRatio = 1.0f, GLfloat fovy = glm::quarter_pi<GLfloat>(), GLfloat nearClipPlane = 0.001f, GLfloat farClipPlane = 1000.0f, GLfloat pitch = 0.15f);

        FirstPersonCam(const FirstPersonCam&) = default;
        FirstPersonCam& operator=(const FirstPersonCam&) = default;
        FirstPersonCam(FirstPersonCam&&) = default;
        FirstPersonCam& operator=(FirstPersonCam&&) = default;
        ~FirstPersonCam() override = default;

        void follow(const glm::vec3& eyePosition, const glm::vec3& forward);

        void recomputeOrientation() override;
        // the character moves the camera, so these do nothing
        void moveForward(GLfloat movementFactor) override {}
        void moveBackward(GLfloat movementFactor) override {}

    private:
        GLfloat _pitch;
    };
}

inline CSCI441::FirstPersonCam::FirstPersonCam(
        const GLfloat aspectRatio,
        const GLfloat fovy,
        const GLfloat nearClipPlane,
        const GLfloat farClipPlane,
        const GLfloat pitch
) : PerspectiveCamera(aspectRatio, fovy, nearClipPlane, farClipPlane),
    _pitch(pitch)
{
    recomputeOrientation();
}

inline void CSCI441::FirstPersonCam::follow(const glm::vec3& eyePosition, const glm::vec3& forward) {
    mCameraPosition = eyePosition;
    // look where the character faces, tipped down by the pitch
    mCameraDirection = glm::normalize(forward + glm::vec3(0.0f, -_pitch, 0.0f) );
    recomputeOrientation();
}

inline void CSCI441::FirstPersonCam::recomputeOrientation() {
    setLookAtPoint(mCameraPosition + mCameraDirection );
    computeViewMatrix();
}

#endif // CSCI441_FIRST_PERSON_CAM_HPP