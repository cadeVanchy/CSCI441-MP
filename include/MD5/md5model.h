/*
 * md5model.h -- md5mesh model loader + animation
 * last modification: 01 september 2023 (Dr. Jeffrey Paone)
 * encapsulated into a class
 * supports texturing
 *
 * Doom3's md5mesh viewer with animation.  Mesh and Animation declaration
 * Dependencies: md5mesh.cpp, md5anim.cpp
 *
 * Copyright (c) 2005-2007 David HENRY
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef MD5_MODEL_H
#define MD5_MODEL_H

#include <glad/gl.h>

namespace MD5 {
    // Vectors
    typedef GLfloat vector2f[2];
    typedef GLfloat vector3f[3];

    // Quaternion (x, y, z, w)
    typedef GLfloat quaternion4f[4];

    enum { X, Y, Z, W };

    // Quaternion prototypes
    void quaternion_compute_w(quaternion4f q);
    void quaternion_normalize(quaternion4f q);
    void quaternion_multiply_quaternion(const quaternion4f QA, const quaternion4f QB, quaternion4f out);
    void quaternion_multiply_vector(const quaternion4f Q, const vector3f IN, quaternion4f out);
    void quaternion_rotate_point(const quaternion4f Q, const vector3f IN, vector3f out);
    GLfloat quaternion_dot_product(const quaternion4f QA, const quaternion4f QB);
    void quaternion_slerp(const quaternion4f QA, const quaternion4f QB, GLfloat T, quaternion4f out);

    class MD5Model {
    protected:
        // Joint
        struct MD5Joint {
            static constexpr GLint NULL_JOINT = -1;

            char name[256] = "";
            GLint parent = NULL_JOINT;

            vector3f position = {0.0f, 0.0f, 0.0f};
            quaternion4f orientation = {0.0f, 0.0f, 0.0f, 0.0f};
        };

        // Joint info
        struct MD5JointInfo {
            char name[256] = "";
            GLint parent = MD5Joint::NULL_JOINT;
            GLuint flags = 0;
            GLint startIndex = 0;
        };

        // Base frame joint
        struct MD5BaseFrameJoint {
            vector3f position = {0.0f, 0.0f, 0.0f};
            quaternion4f orientation = {0.0f, 0.0f, 0.0f, 0.0f};
        };

        // Vertex
        struct MD5Vertex {
            vector2f st = {0.0f, 0.0f};

            GLint start = 0; // start weight
            GLint count = 0; // weight count
        };

        // Triangle
        struct MD5Triangle {
            GLint index[3] = {0};
        };

        // Weight
        struct MD5Weight {
            GLint joint = 0;
            GLfloat bias = 0.f;

            vector3f position = {0.0f, 0.0f, 0.0f};
        };

        // Texture Handles
        struct MD5Texture {
            GLuint texHandle = 0;
            char filename[512] = "";
        };

        // Bounding box
        struct MD5BoundingBox {
            vector3f min = {0.0f, 0.0f, 0.0f};
            vector3f max = {0.0f, 0.0f, 0.0f};
        };

        // MD5 mesh
        struct MD5Mesh {
            MD5Vertex *vertices = nullptr;
            MD5Triangle *triangles = nullptr;
            MD5Weight *weights = nullptr;
            MD5Texture textures[4];

            GLint numVertices = 0;
            GLint numTriangles = 0;
            GLint numWeights = 0;

            char shader[512] = "";
        };

        // Animation data
        struct MD5Animation {
            GLint numFrames = 0;
            GLint numJoints = 0;
            GLint frameRate = 0;

            MD5Joint **skeletonFrames = nullptr;
            MD5BoundingBox *boundingBoxes = nullptr;
        };

        // Animation info
        struct MD5AnimationInfo {
            GLint currFrame = 0;
            GLint nextFrame = 0;

            GLdouble lastTime = 0.0;
            GLdouble maxTime = 0.0;
        };

    public:
        /**
         * initializes an empty MD5 Model
         * @note need to call loadMD5Model() or readMD5Model() after construction to actually load in a mesh file
         */
        MD5Model();
        /**
         * deallocates any used memory on the CPU and GPU
         */
        ~MD5Model();

        /**
         * @brief loads a corresponding md5mesh and md5anim file to the object
         * @param MD5_MESH_FILE name of file to load mesh from
         * @param MD5_ANIM_FILE name of file to load animation from
         */
        bool loadMD5Model(const char* MD5_MESH_FILE, const char* MD5_ANIM_FILE = "");

        /**
         * @brief returns if the MD5 Model has an accompanying animation
         * @returns true if animation exists for model
         */
        [[nodiscard]] bool isAnimated() const { return _isAnimated; }

        // md5mesh prototypes
        /**
         * @brief parses md5mesh file and allocates corresponding mesh data
         * @param FILENAME *.md5mesh file to load
         * @returns true if file parsed successfully
         */
        [[nodiscard]] bool readMD5Model(const char* FILENAME);
        void allocVertexArrays(GLuint vPosAttribLoc, GLuint vColorAttribLoc, GLuint vTexCoordAttribLoc);
        /**
         * @brief draws all the meshes that make up the model
         */
        void draw() const;
        /**
         * @brief draws the skeleton joints (as points) and bones (as lines)
         */
        void drawSkeleton() const;

        // md5anim prototypes
        /**
         * reads in an animation sequence from an external file
         * @param filename *.md5anim file to open
         * @returns true iff mesh and animation are compatible
         */
        [[nodiscard]] bool readMD5Anim(const char* filename);
        /**
         * advances the model forward in its animation sequence the corresponding amount of time based on frame rate
         * @param dt delta time since last frame
         */
        void animate(GLdouble dt);

    private:
        MD5Joint* _baseSkeleton;
        MD5Mesh* _meshes;

        GLint _numJoints;
        GLint _numMeshes;

        // vertex array related stuff
        GLint _maxVertices;
        GLint _maxTriangles;

        vector3f* _vertexArray;
        vector2f* _texelArray;
        GLuint* _vertexIndicesArray;

        GLuint _vao;
        GLuint* _vbo;

        GLuint _skeletonVAO;
        GLuint _skeletonVBO;

        GLfloat _pointSize;
        GLfloat _lineWidth;

        /**
         * @brief the MD5 animation sequence
         */
        MD5Animation _animation;
        /**
         * @brief the MD5 skeletal bone/joint data
         */
        MD5Joint* _skeleton;
        /**
         * @brief the MD5 animation frames
         */
        MD5AnimationInfo _animationInfo;
        /**
         * @brief flag stating if the loaded MD5 model has a corresponding animation or not
         */
        bool _isAnimated;

        void _prepareMesh(const MD5Mesh* P_MESH) const;
        void _drawMesh(const MD5Mesh* P_MESH) const;
        [[nodiscard]] bool _checkAnimValidity() const;
        static void _buildFrameSkeleton(const MD5JointInfo* pJOINT_INFOS,
                                        const MD5BaseFrameJoint* pBASE_FRAME,
                                        const GLfloat* pANIM_FRAME_DATA,
                                        MD5Joint* pSkeletonFrame,
                                        GLint NUM_JOINTS);
        void _interpolateSkeletons(GLdouble interp);
        void _freeModel();
        void _freeVertexArrays();
        void _freeAnim();
    };
}

#endif//MD5_MODEL_H
