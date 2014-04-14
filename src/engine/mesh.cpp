/*
*/
#include <cstring>
#include <iostream>

#include "mesh.h"

namespace vortex {

Mesh::Mesh(std::string name) :
    mName(name),
    mVertices(NULL),
    mIndices(NULL)
{
}

Mesh::Mesh(std::string name, VertexData *vertices, int numVertices, int *indices, int numIndices) :
    mSelected(false),
    mSelectedFaceId(0),
    mName(name),
    mNumVertices(numVertices),
    mVertices(NULL),
    mNumIndices(numIndices),
    mIndices(NULL)
{
    mVertices = new VertexData[mNumVertices];
    assert(mVertices);
    memcpy(mVertices, vertices, mNumVertices * sizeof(VertexData));
    mIndices = new int[mNumIndices];
    assert(mIndices);
    memcpy(mIndices, indices, mNumIndices * sizeof(int));
    // compute Bbox
    for (int i = 0; i < mNumVertices; i++)
        mBbox += mVertices[i].mVertex;
}

Mesh::~Mesh()
{
    delete [] mVertices;
    delete [] mIndices;
}

void Mesh::release()
{
    glAssert(glBindBuffer(GL_ARRAY_BUFFER, 0));
    glAssert(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    glAssert(glBindVertexArray(0));

    glAssert(glDeleteBuffers(2, mVertexBufferObjects));
    glAssert(glDeleteVertexArrays(1, &mVertexArrayObject));
}
void Mesh::reset()
{
    // bind vertexdata
    glAssert(glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObjects[VBO_VERTICES]));
    glAssert(glBufferData(GL_ARRAY_BUFFER, mNumVertices * sizeof(VertexData),  mVertices, GL_STATIC_DRAW));
}

void Mesh::init()
{

    glAssert(glGenVertexArrays(1, &mVertexArrayObject));

    // bind vertex Array
    glAssert(glBindVertexArray(mVertexArrayObject));

    // always generate all buffers : one for vertexdata one for indices
    glAssert(glGenBuffers(2, mVertexBufferObjects));

    // bind vertexdata
    glAssert(glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObjects[VBO_VERTICES]));
    glAssert(glBufferData(GL_ARRAY_BUFFER, mNumVertices * sizeof(VertexData),  mVertices, GL_STATIC_DRAW));

    // global values
    GLuint stride = sizeof(VertexData);
    GLboolean normalized = GL_FALSE;
    GLenum type = GL_FLOAT;

    // mVertex
    GLuint index = 0;
    GLint size = 3;
    GLvoid *pointer = BUFFER_OFFSET(0);

    glAssert(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
    glAssert(glEnableVertexAttribArray(index));

    // mNormal
    index = 1;
    size = 3;
    pointer = BUFFER_OFFSET(sizeof(glm::vec3));

    glAssert(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
    glAssert(glEnableVertexAttribArray(index));

    // mTangente
    index = 2;
    size = 3;
    pointer = BUFFER_OFFSET(2 * sizeof(glm::vec3));

    glAssert(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
    glAssert(glEnableVertexAttribArray(index));

    // mTexCoord
    index = 3;
    size = 4;
    pointer = BUFFER_OFFSET(3 * sizeof(glm::vec3));

    glAssert(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
    glAssert(glEnableVertexAttribArray(index));

    // bind index data
    glAssert(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVertexBufferObjects[VBO_INDICES]));
    glAssert(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mNumIndices * sizeof(int),  mIndices, GL_STATIC_DRAW));

}

void Mesh::draw()
{
         //std::cerr<< "mesh draw " << std::endl;
    // enable vertex attrib
    // glEnable();
    glAssert(glBindVertexArray(mVertexArrayObject));

    // draw count elements in indices
    GLenum type = GL_UNSIGNED_INT;
    GLenum mode = GL_TRIANGLES;
    GLint count = mNumIndices;
    void *indices = NULL;
    glAssert(glDrawElements(mode, count, type, indices));
    if (this->mSelected)
        drawBbox(glm::mat4(), glm::mat4());

}

void Mesh::drawPatch()
{
#ifdef __APPLE__
#warning TODO : Tesselation not yet supported on MacOs X Mountain Lion.
#else
    glAssert(glBindVertexArray(mVertexArrayObject));
    // draw count elements in indices
    GLenum type = GL_UNSIGNED_INT;
    GLenum mode = GL_PATCHES;
    GLint count = mNumIndices;
    void *indices = NULL;
    //glAssert(glPatchParameteri(GL_PATCH_VERTICES, 3));
    glAssert(glDrawElements(mode, count, type, indices));
#endif
}


void Mesh::drawLines()
{
    glAssert(glBindVertexArray(mVertexArrayObject));
    // draw count elements in indices
    GLenum type = GL_UNSIGNED_INT;
    GLenum mode = GL_LINES;
    GLint count = mNumIndices;
    void *indices = NULL;
    glAssert(glDrawElements(mode, count, type, indices));
<<<<<<< HEAD
=======

}
>>>>>>> 0a49773ec341312329eebc1d2aff39cd6d5a0756

}

void Mesh::drawLineStrip()
{
<<<<<<< HEAD
    glAssert(glBindVertexArray(mVertexArrayObject));
    // draw count elements in indices
    GLenum type = GL_UNSIGNED_INT;
    GLenum mode = GL_LINE_STRIP;
    GLint count = mNumIndices;
    void *indices = NULL;
    glAssert(glDrawElements(mode, count, type, indices));

}

void Mesh::drawBbox()
{
    Mesh *bb = BBoxMeshBuilder().build("bbox", mBbox);
    bb->init();
    bb->drawLineStrip();
=======
    glm::vec3 min = mBbox.getMin();
    glm::vec3 max = mBbox.getMax();

    ///@todo : use retained mode

        glBegin(GL_LINE_STRIP);
        glVertex3f( min[0], min[1], min[2] );
        glVertex3f( min[0], min[1], max[2] );
        glVertex3f( min[0], max[1], max[2] );
        glVertex3f( min[0], max[1], min[2] );
        glVertex3f( min[0], min[1], min[2] );
        glVertex3f( max[0], min[1], min[2] );
        glVertex3f( max[0], min[1], max[2] );
        glVertex3f( max[0], max[1], max[2] );
        glVertex3f( max[0], max[1], min[2] );
        glVertex3f( max[0], min[1], min[2] );
        glEnd();

        glBegin(GL_LINE_STRIP);
        glVertex3f( min[0], min[1], max[2] );
        glVertex3f( max[0], min[1], max[2] );
        glVertex3f( max[0], max[1], max[2] );
        glVertex3f( min[0], max[1], max[2] );
        glVertex3f( min[0], max[1], min[2] );
        glVertex3f( max[0], max[1], min[2] );
        glEnd();
>>>>>>> 0a49773ec341312329eebc1d2aff39cd6d5a0756
}

void Mesh::drawSelectedFace()
{
    glm::vec3 v1 = mVertices[ mIndices[3*mSelectedFaceId] ].mVertex;
    glm::vec3 v2 = mVertices[ mIndices[3*mSelectedFaceId+1] ].mVertex;
    glm::vec3 v3 = mVertices[ mIndices[3*mSelectedFaceId+2] ].mVertex;
    glAssert(glPointSize(10.));
    /*  glBegin(GL_TRIANGLE_STRIP);
        glVertex3f( v1[0], v1[1], v1[2] );
        glVertex3f( v2[0], v2[1], v2[2] );
        glVertex3f( v3[0], v3[1], v3[2] );
        glEnd();
    */
}

void Mesh::drawSelectedVertice()
{
    glm::vec3 v = mVertices[ mSelectedVerticeId ].mVertex;
    /*  glBegin(GL_POINTS);
        glVertex3f( v[0], v[1], v[2] );
        glEnd();
    */
}

Mesh::VertexData::VertexData(const VertexData &from, const glm::mat4 &matrix){
    glm::mat4 normalMatrix = glm::inverse(glm::transpose(matrix));
    mVertex = glm::vec3(matrix*(glm::vec4(from.mVertex, 1.f)));
    mTexCoord = from.mTexCoord;
    mNormal = glm::vec3(normalMatrix*(glm::vec4(from.mNormal, 0.f)));
    mTangent = glm::vec3(normalMatrix*(glm::vec4(from.mTangent, 0.f)));
}

Mesh::VertexData::VertexData(const VertexData &from){
    mVertex = from.mVertex;
    mTexCoord = from.mTexCoord;
    mNormal = from.mNormal;
    mTangent = from.mTangent;
}

} // namespace vortex
