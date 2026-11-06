#ifndef A3_A3_ENGINE_H
#define A3_A3_ENGINE_H

#include <CSCI441/Camera.hpp>
#include <CSCI441/OpenGLEngine.hpp>
#include <CSCI441/ShaderProgram.hpp>
#include <CSCI441/ArcballCam.hpp>
#include <CSCI441/FreeCam.hpp>
#include <CSCI441/FirstPersonCam.hpp>

#include "Hero.h"

#include <vector>

class A3Engine final : public CSCI441::OpenGLEngine {
public:
    A3Engine();
    ~A3Engine() override;

    void run() override;

    //*************************************************************************************
    // Internal Event Handlers

    /// \brief handle any key events inside the engine
    /// \param KEY key as represented by GLFW_KEY_ macros
    /// \param ACTION key event action as represented by GLFW_ macros
    void handleKeyEvent(GLint KEY, GLint ACTION);

    /// \brief handle any mouse button events inside the engine
    /// \param button mouse button as represented by GLFW_MOUSE_BUTTON_ macros
    /// \param action mouse event as represented by GLFW_ macros
    void handleMouseButtonEvent(GLint button, GLint action);

    /// \brief handle any cursor movement events inside the engine
    /// \param currMousePosition the current cursor position
    void handleCursorPositionEvent(glm::vec2 currMousePosition);

protected:
    //***************************************************************************
    // Engine Setup
    void mSetupGLFW() override;
    void mSetupOpenGL() override;
    void mSetupShaders() override;
    void mSetupBuffers() override;
    void mSetupScene() override;
    void mSetupFonts() override;

    //***************************************************************************
    // Engine Cleanup
    void mCleanupScene() override;
    void mCleanupBuffers() override;
    void mCleanupShaders() override;
    void mCleanupFonts() override;

private:
    //*************************************************************************************
    // Rendering / Drawing Functions
    /// \brief draws everything to the scene from a particular point of view
    /// \param viewMtx the current view matrix for our camera
    /// \param projMtx the current projection matrix for our camera
    void _renderScene(const glm::mat4& viewMtx, const glm::mat4& projMtx) const;
    /// \brief handles moving our FreeCam as determined by keyboard input
    void _updateScene();

    /// \brief set the lighting parameters to the shader
    void _setLightingParameters() const;

    void _updateCameraTarget() const;

    /// \brief precomputes the matrix uniforms CPU-side and then sends them
    /// to the GPU to be used in the shader for each vertex.  It is more efficient
    /// to calculate these once and then use the resultant product in the shader.
    /// \param modelMtx model transformation matrix
    /// \param viewMtx camera view matrix
    /// \param projMtx camera projection matrix
    void _computeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const;

    /// \brief draws FPS information to screen
    /// \param fps frames per second of last frame
    /// \param fpsAvg frames per second average over last 10 frames
    void _renderTextOverlay(GLdouble fps, GLdouble fpsAvg) const;

    //***************************************************************************
    // Engine State Information

    /// \brief tracks the number of different keys that can be present as determined by GLFW
    static constexpr GLuint NUM_KEYS = GLFW_KEY_LAST;
    /// \brief boolean array tracking each key state.  if true, then the key is in a pressed or held
    /// down state.  if false, then the key is in a released state and not being interacted with
    GLboolean _keys[NUM_KEYS] = {0};

    /// \brief last location of the mouse in window coordinates
    glm::vec2 _mousePosition;
    /// \brief current state of the left mouse button
    GLint _leftMouseButtonState;

    /// \brief value off-screen to represent mouse has not begun interacting with window yet
    static constexpr GLfloat MOUSE_UNINITIALIZED = -99999.0f;

    //***************************************************************************
    // Camera & Hero

    /// \brief arcball camera that orbits and follows the hero
    CSCI441::Camera* _pArcballCam;
    /// \brief free camera
    CSCI441::Camera* _pFreeCam;
    /// \brief FP camera
    CSCI441::FirstPersonCam* _pFirstPersonCam;
    /// \brief camera mode enum
    enum class CameraMode {ARCBALL, FREE, FIRST_PERSON};
    CameraMode _cameraMode;
    /// \brief return active camera
    [[nodiscard]] CSCI441::Camera* _getActiveCamera() const;
    /// \brief update FP cam based on player position (eye height)
    void _updateFirstPersonCam() const;
    /// \brief Free cam speed
    static constexpr GLfloat FREE_CAM_MOVE_SPEED = 0.167f;
    /// \brief how far above the hero's feet the camera looks
    static constexpr GLfloat CAMERA_TARGET_HEIGHT = 1.0f;
    /// \brief radians the camera rotates per pixel of mouse movement
    static constexpr GLfloat CAMERA_ROTATE_SPEED = 0.005f;
    /// \brief world units the camera zooms per pixel of mouse movement
    static constexpr GLfloat CAMERA_ZOOM_SPEED = 0.05f;

    /// \brief my hero
    Hero* _pHero;
    /// \brief world units the hero moves per second
    static constexpr GLfloat HERO_MOVE_SPEED = 0.1f;
    /// \brief radians the hero turns per second
    static constexpr GLfloat HERO_TURN_SPEED = 0.042f;

    //***************************************************************************
    // VAO & Object Information

    /// \brief the size of the world (controls the ground size and locations of buildings)
    static constexpr GLfloat WORLD_SIZE = 55.0f;
    /// \brief VAO for the grid of line strips
    GLuint _gridVAO;
    /// \brief starting vertex of each line strip in the grid VBO
    std::vector<GLint> _gridStripFirsts;
    /// \brief number of vertices in each line strip
    std::vector<GLsizei> _gridStripCounts;
    /// \brief creates the grid VAO
    void _createGridBuffers();

    /// \brief smart container to store information specific to each building we wish to draw
    struct BuildingData {
        /// \brief transformations to position and size the building
        glm::mat4 modelMatrix;
        /// \brief color to draw the building
        glm::vec3 color;
    };
    /// \desc information list of all the buildings to draw
    std::vector<BuildingData> _buildings;

    /// \brief generates building information to make up our scene
    void _generateEnvironment();

    //***************************************************************************
    // Shader Information & Helper Functions

    /// \brief shader program that performs lighting
    CSCI441::ShaderProgram* _lightingShaderProgram;   // the wrapper for our shader program
    /// \brief per-object uniform locations, shared with the hero (group lighting contract)
    LightingUniformLocations _lightingLocs;
    /// \brief light and camera uniform locations, owned by the engine
    struct SceneUniformLocations {
        GLint lightDirection;
        GLint lightColor;
        GLint eyePos;
    } _sceneUniformLocations;

    /// \brief sends diffuse/ambient/specular/shininess for one color
    void _sendColor(const glm::vec3& color) const;
};
//*************************************************************************************
// Callbacks
static void a3_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods );
static void a3_cursor_callback(GLFWwindow *window, double x, double y );
static void a3_mouse_button_callback(GLFWwindow *window, int button, int action, int mods );

#endif// A3_A3_ENGINE_H
