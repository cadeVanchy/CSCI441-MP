#include "LightingShader.h"

#include <CSCI441/objects.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

LightingShader::LightingShader(const char* vertexShaderPath, const char* fragmentShaderPath)
    : _shaderProgram(new CSCI441::ShaderProgram(vertexShaderPath, fragmentShaderPath)),
      _uniformLocations(),
      _eyePosLocation(-1),
      _lightLocations() {

    //matrix + material uniforms (the contract every hero uses)
    _uniformLocations.mvpMtx            = _lookUp("mvpMatrix");
    _uniformLocations.modelMtx          = _lookUp("modelMatrix");
    _uniformLocations.normalMtx         = _lookUp("normalMatrix");
    _uniformLocations.materialDiffuse   = _lookUp("materialDiffuse");
    _uniformLocations.materialAmbient   = _lookUp("materialAmbient");
    _uniformLocations.materialSpecular  = _lookUp("materialSpecular");
    _uniformLocations.materialShininess = _lookUp("materialShininess");

    //camera
    _eyePosLocation = _lookUp("eyePos");

    //lights - struct members are looked up as "structName.memberName"
    _lightLocations.dirDirection       = _lookUp("dirLight.direction");
    _lightLocations.dirColor           = _lookUp("dirLight.color");

    _lightLocations.pointPosition      = _lookUp("pointLight.position");
    _lightLocations.pointColor         = _lookUp("pointLight.color");
    _lightLocations.pointAttenuation   = _lookUp("pointLight.attenuation");

    _lightLocations.spotPosition       = _lookUp("spotLight.position");
    _lightLocations.spotDirection      = _lookUp("spotLight.direction");
    _lightLocations.spotColor          = _lookUp("spotLight.color");
    _lightLocations.spotAttenuation    = _lookUp("spotLight.attenuation");
    _lightLocations.spotCosInnerCutoff = _lookUp("spotLight.cosInnerCutoff");
    _lightLocations.spotCosOuterCutoff = _lookUp("spotLight.cosOuterCutoff");

    //connect the CSCI441 shape library (drawSolidCube, etc.) to our fixed attribute locations so its VAOs feed vPos and vNormal correctly
    CSCI441::setVertexAttributeLocations(LightingAttributes::POSITION, LightingAttributes::NORMAL);

}//end constructor



LightingShader::~LightingShader() {
    delete _shaderProgram;
}//end destructor



GLint LightingShader::_lookUp(const char* uniformName) const {

    const GLint location = _shaderProgram->getUniformLocation(uniformName);
    return location;

}//end _lookUp



void LightingShader::useProgram() const {
    _shaderProgram->useProgram();
}//end useProgram()



GLuint LightingShader::getShaderProgramHandle() const {
    return _shaderProgram->getShaderProgramHandle();
}//end getShaderProgramHandle()



const LightingUniformLocations& LightingShader::getUniformLocations() const {
    return _uniformLocations;
}//end getUniformLocations()



void LightingShader::setEyePosition(const glm::vec3& eyePos) const {
    glProgramUniform3fv(getShaderProgramHandle(), _eyePosLocation, 1, glm::value_ptr(eyePos));
}//end setEyePosition()



void LightingShader::sendMatrices(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) const {
    const GLuint handle = getShaderProgramHandle();

    // precompute the MVP matrix CPU side so it is done once, not once per vertex
    const glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    glProgramUniformMatrix4fv(handle, _uniformLocations.mvpMtx, 1, GL_FALSE, glm::value_ptr(mvpMtx));

    // model matrix puts vertices in world space, where the lighting is done
    glProgramUniformMatrix4fv(handle, _uniformLocations.modelMtx, 1, GL_FALSE, glm::value_ptr(modelMtx));

    // normal matrix keeps normals perpendicular even with non-uniform scaling
    const glm::mat3 normalMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
    glProgramUniformMatrix3fv(handle, _uniformLocations.normalMtx, 1, GL_FALSE, glm::value_ptr(normalMtx));
}//end sendMatrices



void LightingShader::setMaterial(const glm::vec3& diffuse, const glm::vec3& ambient, const glm::vec3& specular, const GLfloat shininess) const {
    const GLuint handle = getShaderProgramHandle();
    glProgramUniform3fv(handle, _uniformLocations.materialDiffuse, 1, glm::value_ptr(diffuse));
    glProgramUniform3fv(handle, _uniformLocations.materialAmbient, 1, glm::value_ptr(ambient));
    glProgramUniform3fv(handle, _uniformLocations.materialSpecular, 1, glm::value_ptr(specular));
    glProgramUniform1f( handle, _uniformLocations.materialShininess, shininess);

}//end setMaterial



void LightingShader::setupDefaultLights() const {
    //directional
    setDirectionalLight(glm::vec3(-1.0f, -2.0f, -1.0f),
        glm::vec3(0.55f, 0.50f, 0.42f));

    // point
    setPointLight(glm::vec3(8.0f, 3.0f, 8.0f),
        glm::vec3(0.30f, 0.55f, 1.00f),
        glm::vec3(1.0f, 0.09f, 0.032f));

    // spotlight
    setSpotLight(glm::vec3(0.0f, 15.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(1.00f, 0.95f, 0.85f),
        glm::vec3(1.0f, 0.022f, 0.0019f),
        15.0f, 25.0f);

}//end setupDefaultLights()



void LightingShader::setDirectionalLight(const glm::vec3& direction, const glm::vec3& color) const {
    const GLuint handle = getShaderProgramHandle();
    glProgramUniform3fv(handle, _lightLocations.dirDirection, 1, glm::value_ptr(direction));
    glProgramUniform3fv(handle, _lightLocations.dirColor,     1, glm::value_ptr(color));
}//end setDirectionalLight()



void LightingShader::setPointLight(const glm::vec3& position, const glm::vec3& color, const glm::vec3& attenuation) const {

    const GLuint handle = getShaderProgramHandle();
    glProgramUniform3fv(handle, _lightLocations.pointPosition,    1, glm::value_ptr(position));
    glProgramUniform3fv(handle, _lightLocations.pointColor,       1, glm::value_ptr(color));
    glProgramUniform3fv(handle, _lightLocations.pointAttenuation, 1, glm::value_ptr(attenuation));
}//end setPointLight()



void LightingShader::setSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color,
    const glm::vec3& attenuation, const GLfloat innerAngleDegrees, const GLfloat outerAngleDegrees) const {

    const GLuint handle = getShaderProgramHandle();
    glProgramUniform3fv(handle, _lightLocations.spotPosition,    1, glm::value_ptr(position));
    glProgramUniform3fv(handle, _lightLocations.spotDirection,   1, glm::value_ptr(direction));
    glProgramUniform3fv(handle, _lightLocations.spotColor,       1, glm::value_ptr(color));
    glProgramUniform3fv(handle, _lightLocations.spotAttenuation, 1, glm::value_ptr(attenuation));

    // the shader compares cosines, so convert the angles once here instead of per vertex
    glProgramUniform1f(handle, _lightLocations.spotCosInnerCutoff, glm::cos(glm::radians(innerAngleDegrees)));
    glProgramUniform1f(handle, _lightLocations.spotCosOuterCutoff, glm::cos(glm::radians(outerAngleDegrees)));
}//end setSpotLight()