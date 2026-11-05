#ifndef LIGHTING_UNIFORMS_H
#define LIGHTING_UNIFORMS_H

#include <glad/gl.h>

/// \brief The shared "contract" between our Gouraud lighting shader and
/// everything that gets drawn with it (heroes, ground, scenery).
///
/// Every object drawn with the lighting shader must, for EVERY draw call:
///   1) send its matrices:  mvpMatrix, modelMatrix, normalMatrix
///   2) send its material:  materialDiffuse, materialAmbient,
///                          materialSpecular, materialShininess
///
/// GLSL names in the lighting vertex shader (these must match exactly):
///   attributes:  vPos    (layout location 0)
///                vNormal (layout location 1)
///   uniforms:    mat4  mvpMatrix
///                mat4  modelMatrix
///                mat3  normalMatrix
///                vec3  materialDiffuse
///                vec3  materialAmbient
///                vec3  materialSpecular
///                float materialShininess
///                vec3  eyePos          (camera position in world space,
///                                       set per viewport by the engine)
///
/// Light uniforms are owned by the lighting code and are not part of this
/// contract; heroes and scenery never touch them.

/// \brief fixed vertex attribute locations used by EVERY shader in the project
/// (lighting, skybox, etc.) so CSCI441::setVertexAttributeLocations() only
/// needs to be called once
namespace LightingAttributes {

    /// \brief vertex position attribute location (vPos)
    constexpr GLint POSITION = 0;

    /// \brief vertex normal attribute location (vNormal)
    constexpr GLint NORMAL = 1;

} //end namespace

/// \brief uniform locations every hero/object needs to draw itself with lighting.
/// The lighting shader code fills this in and hands a copy to each hero.
struct LightingUniformLocations {

    /// \brief precomputed Model-View-Projection matrix (mat4 mvpMatrix)
    GLint mvpMtx = -1;

    /// \brief model matrix, used to put vertices in world space (mat4 modelMatrix)
    GLint modelMtx = -1;

    /// \brief inverse-transpose of the model matrix for normals (mat3 normalMatrix)
    GLint normalMtx = -1;

    /// \brief material diffuse color (vec3 materialDiffuse)
    GLint materialDiffuse = -1;

    /// \brief material ambient color (vec3 materialAmbient)
    GLint materialAmbient = -1;

    /// \brief material specular color (vec3 materialSpecular)
    GLint materialSpecular = -1;

    /// \brief material shininess exponent (float materialShininess)
    GLint materialShininess = -1;
    
};//end struct

#endif // LIGHTING_UNIFORMS_H