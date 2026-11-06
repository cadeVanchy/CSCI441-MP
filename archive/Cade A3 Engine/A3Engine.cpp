#include "A3Engine.h"

#include <CSCI441/ArcballCam.hpp>
#include <CSCI441/FreeCam.hpp>
#include <CSCI441/FirstPersonCam.hpp>
#include <CSCI441/objects.hpp>

#include <random>

//*************************************************************************************
// Helpers

namespace {
    /// \brief Simple helper class to return a random number between 0.0f and 1.0f.
    class RandomGenerator {
    public:
        /// \brief generates a random value
        /// \returns random GLfloat within the inclusive range [0.0f, 1.0f]
        static GLfloat getRand() { return _dist( _rng ); }

        // do not allow an object to be copied nor moved
        RandomGenerator(RandomGenerator&) = delete;
        RandomGenerator& operator=(RandomGenerator&) = delete;
        RandomGenerator(RandomGenerator&&) = delete;
        RandomGenerator& operator=(RandomGenerator&&) = delete;
    private:
        // do not allow an object to be externally created
        RandomGenerator() {};
        static std::mt19937 _rng;
        static std::uniform_real_distribution<GLfloat> _dist;
    };

    std::mt19937 RandomGenerator::_rng = std::mt19937( std::random_device{}() );
    std::uniform_real_distribution<GLfloat> RandomGenerator::_dist = std::uniform_real_distribution<GLfloat>( 0.0f, std::nextafter( 1.0f, 2.0f ) );

    /// \brief vertex layout for grid
    struct VertexNormal {
    glm::vec3 position;
    glm::vec3 normal;
};
}

//*************************************************************************************
// Public Interface

A3Engine::A3Engine()
    : CSCI441::OpenGLEngine(4, 1, 640, 480, "A3"),
    _mousePosition( {MOUSE_UNINITIALIZED, MOUSE_UNINITIALIZED} ),
    _leftMouseButtonState(GLFW_RELEASE),
    _pArcballCam(nullptr),
    _pFreeCam(nullptr),
    _pFirstPersonCam(nullptr),
    _cameraMode(CameraMode::ARCBALL),
    _pHero(nullptr),
    _gridVAO(0),
    _lightingShaderProgram(nullptr),
    _sceneUniformLocations( {-1, -1, -1} )
{
    for(auto& _key : _keys) _key = GL_FALSE;
}

A3Engine::~A3Engine() {
    delete _pHero;
    delete _pArcballCam;
    delete _lightingShaderProgram;
}

//*************************************************************************************
// Internal Event Handlers

void A3Engine::handleKeyEvent(const GLint KEY, const GLint ACTION) {
    if(KEY != GLFW_KEY_UNKNOWN)
        _keys[KEY] = ((ACTION == GLFW_PRESS) || (ACTION == GLFW_REPEAT));

    if(ACTION == GLFW_PRESS) {
        switch( KEY ) {
            // quit!
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                setWindowShouldClose();
                break;

            case GLFW_KEY_R:
                mReloadShaders();
                _setLightingParameters();
                _pHero->setProgramUniformLocations( _lightingShaderProgram->getShaderProgramHandle(), _lightingLocs );
                break;
            // camera switch cases
            case GLFW_KEY_1:
                _cameraMode = CameraMode::ARCBALL;
                break;
            case GLFW_KEY_2:
                _cameraMode = CameraMode::FREE;
                break;
            case GLFW_KEY_3:
                _cameraMode = CameraMode::FIRST_PERSON;
                break;

            default: break; // suppress CLion warning
        }
    }
}

void A3Engine::handleMouseButtonEvent(const GLint button, const GLint action) {
    // if the event is for the left mouse button
    if( button == GLFW_MOUSE_BUTTON_LEFT ) {
        // update the left mouse button's state
        _leftMouseButtonState = action;
    }
}

void A3Engine::handleCursorPositionEvent(const glm::vec2 currMousePosition) {
    // if mouse hasn't moved in the window, prevent camera from flipping out
    if(_mousePosition.x == MOUSE_UNINITIALIZED) {
        _mousePosition = currMousePosition;
    }

    // if the left mouse button is being held down while the mouse is moving
    if(_leftMouseButtonState == GLFW_PRESS) {
        const glm::vec2 delta = currMousePosition - _mousePosition;

        if( _cameraMode == CameraMode::ARCBALL ) {
            if( _keys[GLFW_KEY_LEFT_SHIFT] ) {
                // shift + drag zooms
                if( delta.y > 0.0f ) {
                    _pArcballCam->moveBackward( delta.y * CAMERA_ZOOM_SPEED );
                } else {
                    _pArcballCam->moveForward( -delta.y * CAMERA_ZOOM_SPEED );
                }
            } else {
                // drag orbits the camera around the hero
                _pArcballCam->rotate( delta.x * CAMERA_ROTATE_SPEED, -delta.y * CAMERA_ROTATE_SPEED );
            }
        } else if( _cameraMode == CameraMode::FREE ) {
            // drag turns the free cam's view
            _pFreeCam->rotate( delta.x * CAMERA_ROTATE_SPEED, -delta.y * CAMERA_ROTATE_SPEED );
        }
    }

    // update the mouse position
    _mousePosition = currMousePosition;
}

//*************************************************************************************
// Engine Setup

void A3Engine::mSetupGLFW() {
    CSCI441::OpenGLEngine::mSetupGLFW();

    // set callbacks
    glfwSetKeyCallback(mpWindow, a3_keyboard_callback);
    glfwSetMouseButtonCallback(mpWindow, a3_mouse_button_callback);
    glfwSetCursorPosCallback(mpWindow, a3_cursor_callback);
}

void A3Engine::mSetupOpenGL() {
    glEnable( GL_DEPTH_TEST );					                    // enable depth testing
    glDepthFunc( GL_LESS );							                // use less than depth test

    glEnable(GL_BLEND);									            // enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	            // use one minus blending equation

    glClearColor( 0.4f, 0.4f, 0.4f, 1.0f );	// clear the frame buffer to gray

    CSCI441::OpenGLUtils::checkOpenGLErrors("mSetupOpenGL() done");
}

void A3Engine::mSetupShaders() {
    _lightingShaderProgram = new CSCI441::ShaderProgram("shaders/a3.v.glsl", "shaders/a3.f.glsl" );
    _lightingLocs.mvpMtx = _lightingShaderProgram->getUniformLocation("mvpMatrix");
    _lightingLocs.modelMtx = _lightingShaderProgram->getUniformLocation("modelMatrix");
    _lightingLocs.normalMtx = _lightingShaderProgram->getUniformLocation("normalMatrix");
    _lightingLocs.materialDiffuse = _lightingShaderProgram->getUniformLocation("materialDiffuse");
    _lightingLocs.materialAmbient = _lightingShaderProgram->getUniformLocation("materialAmbient");
    _lightingLocs.materialSpecular= _lightingShaderProgram->getUniformLocation("materialSpecular");
    _lightingLocs.materialShininess = _lightingShaderProgram->getUniformLocation("materialShininess");

    _sceneUniformLocations.lightDirection = _lightingShaderProgram->getUniformLocation("lightDirection");
    _sceneUniformLocations.lightColor = _lightingShaderProgram->getUniformLocation("lightColor");
    _sceneUniformLocations.eyePos = _lightingShaderProgram->getUniformLocation("eyePos");


    CSCI441::OpenGLUtils::checkOpenGLErrors("mSetupShaders() done");
}

void A3Engine::mSetupBuffers() {
    // needed to connect our 3D Object Library to our shader
    CSCI441::setVertexAttributeLocations( LightingAttributes::POSITION, LightingAttributes::NORMAL );

    _pHero = new Hero( _lightingShaderProgram->getShaderProgramHandle(), _lightingLocs );
    _pHero->setWorldHalfSize( WORLD_SIZE );

    _createGridBuffers();
    _generateEnvironment();

    CSCI441::OpenGLUtils::checkOpenGLErrors("mSetupBuffers() done");
}

void A3Engine::_createGridBuffers() {
    constexpr GLfloat SPACING = 1.0f;
    const GLint numSteps = static_cast<GLint>(2.0f * WORLD_SIZE / SPACING);
    constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);

    std::vector<VertexNormal> vertices;
    _gridStripFirsts.clear();
    _gridStripCounts.clear();

    // lines parallel to the Z axis
    for(GLint i = 0; i <= numSteps; i++) {
        const GLfloat x = -WORLD_SIZE + static_cast<GLfloat>(i) * SPACING;
        _gridStripFirsts.push_back(static_cast<GLint>(vertices.size()));
        for(GLint j = 0; j <= numSteps; j++) {
            const GLfloat z = -WORLD_SIZE + static_cast<GLfloat>(j) * SPACING;
            vertices.push_back({ {x, 0.0f, z}, UP });
        }
        _gridStripCounts.push_back(numSteps + 1);
    }

    // lines parallel to the X axis
    for(GLint i = 0; i <= numSteps; i++) {
        const GLfloat z = -WORLD_SIZE + static_cast<GLfloat>(i) * SPACING;
        _gridStripFirsts.push_back(static_cast<GLint>(vertices.size()));
        for(GLint j = 0; j <= numSteps; j++) {
            const GLfloat x = -WORLD_SIZE + static_cast<GLfloat>(j) * SPACING;
            vertices.push_back({ {x, 0.0f, z}, UP });
        }
        _gridStripCounts.push_back(numSteps + 1);
    }

    glGenVertexArrays(1, &_gridVAO);
    glBindVertexArray(_gridVAO);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(VertexNormal)), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(LightingAttributes::POSITION);
    glVertexAttribPointer(LightingAttributes::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), (void*)0);

    glEnableVertexAttribArray(LightingAttributes::NORMAL);
    glVertexAttribPointer(LightingAttributes::NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), (void*)offsetof(VertexNormal, normal));
}

void A3Engine::_generateEnvironment() {
    constexpr GLint EDGE = static_cast<GLint>(WORLD_SIZE) - 1;
    // keep the area around the hero's start empty
    constexpr GLint SPAWN_CLEARANCE = 4;

    for(GLint i = -EDGE; i <= EDGE; i++) {
        for(GLint j = -EDGE; j <= EDGE; j++) {
            if( std::abs(i) < SPAWN_CLEARANCE && std::abs(j) < SPAWN_CLEARANCE ) {
                continue;
            }

            if( i % 2 && j % 2 && RandomGenerator::getRand() < 0.1f ) {
                // translate to spot
                const glm::mat4 transToSpotMtx = glm::translate( glm::mat4(1.0), glm::vec3(i, 0.0f, j) );

                // compute random height
                const GLfloat height = powf(RandomGenerator::getRand(), 2.5f) * 10.0f + 1.0f;
                // scale to building size
                const glm::mat4 scaleToHeightMtx = glm::scale( glm::mat4(1.0), glm::vec3(1, height, 1) );

                // translate up to grid
                const glm::mat4 transToHeight = glm::translate( glm::mat4(1.0), glm::vec3(0, height/2.0f, 0) );

                // compute full model matrix
                const glm::mat4 modelMatrix = transToHeight * scaleToHeightMtx * transToSpotMtx;

                // compute random color
                const glm::vec3 color( RandomGenerator::getRand(), RandomGenerator::getRand(), RandomGenerator::getRand() );
                // store building properties
                _buildings.push_back( {modelMatrix, color} );
            }
        }
    }
}

void A3Engine::mSetupScene() {
    // match the window's aspect ratio so the scene isn't stretched
    const GLfloat aspectRatio = static_cast<GLfloat>(mFramebufferWidth) / static_cast<GLfloat>(mFramebufferHeight);
    _pArcballCam = new CSCI441::ArcballCam( static_cast<GLfloat>(mFramebufferWidth) / static_cast<GLfloat>(mFramebufferHeight) );
    _pArcballCam->setTheta( 0.0f );
    _pArcballCam->setPhi( glm::pi<GLfloat>() / 3.0f );
    _pArcballCam->setRadius( 10.0f );
    _updateCameraTarget();
    // free cam starts behind and above the origin, looking down toward it
    _pFreeCam = new CSCI441::FreeCam( aspectRatio );
    _pFreeCam->setPosition( glm::vec3( 0.0f, 8.0f, 20.0f ) );
    _pFreeCam->setTheta( 0.0f );
    _pFreeCam->setPhi( glm::half_pi<GLfloat>() - 0.4f );
    _pFreeCam->recomputeOrientation();

    _pFirstPersonCam = new CSCI441::FirstPersonCam( aspectRatio );
    _updateFirstPersonCam();
    _setLightingParameters();

    CSCI441::OpenGLUtils::checkOpenGLErrors("mSetupScene() done");
}

void A3Engine::mSetupFonts() {
    if (!CSCI441::FontUtils::loadFont("assets/fonts/DroidSansMono.ttf")) {
        CSCI441::LogUtils::logError("[ERROR]: Could not load font\n");
    }
    CSCI441::FontUtils::setWindowSize(mFramebufferWidth, mFramebufferHeight);
    CSCI441::FontUtils::setFontSize(2.0f / static_cast<GLfloat>(mFramebufferWidth), 2.0f / static_cast<GLfloat>(mFramebufferHeight) );
    CSCI441::OpenGLUtils::checkOpenGLErrors("mSetupFonts() done");
}

//*************************************************************************************
// Engine Cleanup

void A3Engine::mCleanupShaders() {
    CSCI441::LogUtils::log("[INFO]: ...deleting Shaders.\n" );
    delete _lightingShaderProgram;
    _lightingShaderProgram = nullptr;

    CSCI441::OpenGLUtils::checkOpenGLErrors("mCleanupShaders() done");
}

void A3Engine::mCleanupBuffers() {
    CSCI441::LogUtils::log("[INFO]: ...deleting VAOs....\n" );
    CSCI441::deleteObjectVAOs();
    glDeleteVertexArrays( 1, &_gridVAO );

    CSCI441::LogUtils::log("[INFO]: ...deleting VBOs....\n" );
    CSCI441::deleteObjectVBOs();

    CSCI441::LogUtils::log("[INFO]: ...deleting models..\n" );
    delete _pHero;
    _pHero = nullptr;

    CSCI441::OpenGLUtils::checkOpenGLErrors("mCleanupBuffers() done");
}

void A3Engine::mCleanupScene() {
    CSCI441::LogUtils::log("[INFO]: ...deleting cameras.\n" );
    delete _pArcballCam;
    _pArcballCam = nullptr;
    delete _pFreeCam;
    _pFreeCam = nullptr;
    delete _pFirstPersonCam;
    _pFirstPersonCam = nullptr;

    CSCI441::OpenGLUtils::checkOpenGLErrors("mCleanupScene() done");
}

void A3Engine::mCleanupFonts() {
    CSCI441::FontUtils::releaseFont();

    CSCI441::OpenGLUtils::checkOpenGLErrors("mCleanupFonts() done");
}

//*************************************************************************************
// Rendering / Drawing Functions - this is where the magic happens!

void A3Engine::_renderScene(const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // use our lighting shader program
    _lightingShaderProgram->useProgram();

    // specular depends on where the camera is, so send it every frame
    _lightingShaderProgram->setProgramUniform(_sceneUniformLocations.eyePos, _getActiveCamera()->getPosition());

    //// BEGIN DRAWING THE GRID ////
    _computeAndSendMatrixUniforms(glm::mat4(1.0f), viewMtx, projMtx);

    constexpr glm::vec3 gridColor(1.0f, 1.0f, 1.0f);
    _sendColor(gridColor);

    glBindVertexArray(_gridVAO);
    glMultiDrawArrays(GL_LINE_STRIP, _gridStripFirsts.data(), _gridStripCounts.data(), static_cast<GLsizei>(_gridStripFirsts.size()));
    //// END DRAWING THE GRID ////

    //// BEGIN DRAWING THE SCENE OBJECTS ////
    for( const auto& [modelMatrix, color] : _buildings ) {
        _computeAndSendMatrixUniforms(modelMatrix, viewMtx, projMtx);
        _sendColor(color);
        CSCI441::drawSolidCube(1.0);
    }
    //// END DRAWING THE SCENE OBJECTS ////

    //// BEGIN DRAWING THE HERO ////
    _pHero->draw(viewMtx, projMtx);
    //// END DRAWING THE HERO ////
}

void A3Engine::_updateScene() {
    bool moving = false;

    // turn left / right
    if( _keys[GLFW_KEY_A] ) {
        _pHero->turn(  HERO_TURN_SPEED );
    }
    if( _keys[GLFW_KEY_D] ) {
        _pHero->turn( -HERO_TURN_SPEED );
    }

    // move forward / backward along the heading
    if( _keys[GLFW_KEY_W] ) {
        _pHero->moveForward(  HERO_MOVE_SPEED );
        moving = true;
    }
    if( _keys[GLFW_KEY_S] ) {
        _pHero->moveBackward( HERO_MOVE_SPEED );
        moving = true;
    }

    _pHero->setMoving( moving );
    _pHero->update();

    // free cam flies with space (forward) and shift + space (backward)
    if( _cameraMode == CameraMode::FREE && _keys[GLFW_KEY_SPACE] ) {
        if( _keys[GLFW_KEY_LEFT_SHIFT] ) {
            _pFreeCam->moveBackward( FREE_CAM_MOVE_SPEED );
        } else {
            _pFreeCam->moveForward( FREE_CAM_MOVE_SPEED );
        }
    }

    _updateCameraTarget();
    _updateFirstPersonCam();
}

void A3Engine::_updateFirstPersonCam() const {
    _pFirstPersonCam->follow( _pHero->getEyePosition(), _pHero->getForward() );
}

CSCI441::Camera* A3Engine::_getActiveCamera() const {
    if( _cameraMode == CameraMode::FREE ) {
        return _pFreeCam;
    }
    if( _cameraMode == CameraMode::FIRST_PERSON ) {
        return _pFirstPersonCam;
    }
    return _pArcballCam;
}

void A3Engine::_updateCameraTarget() const {
    glm::vec3 target( 0.0f, CAMERA_TARGET_HEIGHT, 0.0f );
    if( _pHero != nullptr ) {
        target += _pHero->getPosition();
    }
    _pArcballCam->setLookAtPoint( target );
    _pArcballCam->recomputeOrientation();
}

void A3Engine::_renderTextOverlay(const GLdouble fps, const GLdouble fpsAvg) const {
    CSCI441::FontUtils::bindFont();

    const GLint top = mFramebufferHeight;

    constexpr GLuint STR_LEN = 80;
    char fpsStr[STR_LEN];
    snprintf( fpsStr, STR_LEN, "%7.3f frames/sec (Avg: %7.3f)", fps, fpsAvg);
    CSCI441::FontUtils::renderText(fpsStr, 24.f,   static_cast<GLfloat>(top - 24*1));

    snprintf( fpsStr, STR_LEN, "%7.3f sec/frame  (Avg: %7.3f)", 1000.f / fps, 1000.f / fpsAvg);
    CSCI441::FontUtils::renderText(fpsStr, 24.f,   static_cast<GLfloat>(top - 24*2));
}

void A3Engine::_setLightingParameters() const {
    constexpr glm::vec3 lightDir = {-1.0f, -1.0f,  1.0f};
    constexpr glm::vec3 lightCol = { 1.0f,  1.0f,  1.0f};
    _lightingShaderProgram->setProgramUniform(_sceneUniformLocations.lightDirection, lightDir);
    _lightingShaderProgram->setProgramUniform(_sceneUniformLocations.lightColor, lightCol);
}

void A3Engine::_sendColor(const glm::vec3& color) const {
    _lightingShaderProgram->setProgramUniform(_lightingLocs.materialDiffuse, color);
    _lightingShaderProgram->setProgramUniform(_lightingLocs.materialAmbient, color * 0.2f);
    _lightingShaderProgram->setProgramUniform(_lightingLocs.materialSpecular, glm::vec3(0.3f));
    _lightingShaderProgram->setProgramUniform(_lightingLocs.materialShininess, 16.0f);
}

void A3Engine::_computeAndSendMatrixUniforms(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    // precompute the Model-View-Projection matrix on the CPU
    const glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    // then send it to the shader on the GPU to apply to every vertex
    _lightingShaderProgram->setProgramUniform(_lightingLocs.mvpMtx, mvpMtx);

    _lightingShaderProgram->setProgramUniform(_lightingLocs.modelMtx, modelMtx);
    const glm::mat3 normalMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
    _lightingShaderProgram->setProgramUniform(_lightingLocs.normalMtx, normalMtx);

}

void A3Engine::run() {
    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while( !glfwWindowShouldClose(mpWindow) ) {	        // check if the window was instructed to be closed
        glDrawBuffer( GL_BACK );				        // work with our back frame buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore,
        // query what the actual size of the window we are rendering to is.
        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize( mpWindow, &framebufferWidth, &framebufferHeight );

        // update the viewport - tell OpenGL we want to render to the whole window
        glViewport( 0, 0, framebufferWidth, framebufferHeight );

        // draw everything to the window
        const CSCI441::Camera* pCam = _getActiveCamera();
        _renderScene(pCam->getViewMatrix(), pCam->getProjectionMatrix());
        _renderTextOverlay(this->clockFrame(), this->fpsAverage()); // display FPS information on screen
        
        _updateScene();


        glfwSwapBuffers(mpWindow);                       // flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();				                // check for any events and signal to redraw screen
    }
}

//*************************************************************************************
// Callbacks

void a3_keyboard_callback(GLFWwindow *window, const int key, const int scancode, const int action, const int mods ) {
    const auto engine = static_cast<A3Engine *>(glfwGetWindowUserPointer(window));

    // pass the key and action through to the engine
    engine->handleKeyEvent(key, action);
}

void a3_cursor_callback(GLFWwindow *window, const double x, const double y ) {
    const auto engine = static_cast<A3Engine *>(glfwGetWindowUserPointer(window));

    // pass the cursor position through to the engine
    engine->handleCursorPositionEvent(glm::vec2(x, y));
}

void a3_mouse_button_callback(GLFWwindow *window, const int button, const int action, const int mods ) {
    const auto engine = static_cast<A3Engine *>(glfwGetWindowUserPointer(window));

    // pass the mouse button and action through to the engine
    engine->handleMouseButtonEvent(button, action);
}
