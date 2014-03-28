#include "bboxmeshbuilder.h"
namespace vortex {
BboxMeshBuilder::BboxMeshBuilder() : bbox(NULL),
    oldMin(glm::vec3()), oldMax(glm::vec3()), corners(std::vector<glm::vec3>())
{
    init();
}

BboxMeshBuilder::BboxMeshBuilder(BBox *b) : bbox(NULL),
    oldMin(glm::vec3()), oldMax(glm::vec3()), corners(std::vector<glm::vec3>())
{

    bbox = b;
    updateCorners();
    init();
}

void BboxMeshBuilder::init()
{

    nbVertices = 24; // vec3 * 8 coins du cube
    mVertices = new float[nbVertices];

    GLvoid *ptr = BUFFER_OFFSET(0);

    if (glIsVertexArray(mvao) == GL_TRUE)
        glDeleteVertexArrays(1, &mvao);

    glAssert( glGenVertexArrays(1, &mvao) );

    if (glIsBuffer(mvbo[0]) == GL_TRUE || glIsBuffer(mvbo[1]) == GL_TRUE)
        glDeleteBuffers(2, mvbo);

    glAssert( glGenBuffers(2, mvbo) );

    glAssert( glBindVertexArray(mvao) );
    {
        glBindBuffer(GL_ARRAY_BUFFER, mvbo[0]);
        {
            glBufferData(GL_ARRAY_BUFFER, nbVertices * sizeof(glm::mediump_float), &(corners[0]), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, ptr);
            glEnableVertexAttribArray(0);

            nbIndices  = 16;
            GLuint indices[] = {
                0,3,4,2,
                4,7,6,2,
                0,1,6,7,
                5,1,5,3
            };


            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mvbo[1]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, nbIndices * sizeof(GLuint),
                         indices, GL_STATIC_DRAW);

        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glAssert( glBindVertexArray(0) );
}

void BboxMeshBuilder::draw()
{
    if(bbox!=NULL && !bbox->isEmpty()) {
        update();
        glBindVertexArray(mvao);
        glDrawElements(GL_LINE_STRIP, nbIndices, GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
    }
}

void BboxMeshBuilder::setBBox(BBox *b)
{
    bbox = b;
    if( bbox!=NULL && bbox->isEmpty() )
        update();
}

void BboxMeshBuilder::update()
{
    // actualisation du VBO seulement s'il y a eu un changement
    if( (oldMin!=bbox->getMin() || oldMax!=bbox->getMax()) ) {
        updateCorners();

        glBindBuffer(GL_ARRAY_BUFFER, mvbo[0]);
        glBufferData(GL_ARRAY_BUFFER, nbVertices * sizeof(glm::mediump_float), NULL, GL_STATIC_DRAW);

        void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        if(ptr!=NULL)
        {
            GLuint len = nbVertices * sizeof(glm::mediump_float);
            memcpy((char*)ptr, &(corners[0]), len);
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}


void BboxMeshBuilder::updateCorners() {

    if(bbox!=NULL && !bbox->isEmpty()) {
        corners.clear();
        bbox->getCorners(corners);
        oldMin = bbox->getMin();
        oldMax = bbox->getMax();
    }

}

BboxMeshBuilder::~BboxMeshBuilder()
{
    corners.clear();
    if (glIsBuffer(mvbo[0]) == GL_TRUE || glIsBuffer(mvbo[1]) == GL_TRUE)
        glDeleteBuffers(2, mvbo);

    if (glIsVertexArray(mvao) == GL_TRUE)
        glDeleteVertexArrays(1, &mvao);

    delete [] mVertices;
}
}
