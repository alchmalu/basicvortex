#ifndef BBOXMESHBUILDER_H
#define BBOXMESHBUILDER_H
#include "bbox.h"
namespace vortex {
/**
 * @brief The BboxMeshBuilder class
 * Display BBox in retained mode
 * update BBox after each call to draw() if it change
 */
class BboxMeshBuilder
{
public :
    BboxMeshBuilder();
    BboxMeshBuilder(BBox *b);
    ~BboxMeshBuilder();

    /**
     * @brief draw
     * Dessine la bbox
     * Utilisation de GL_LINE_SRTIP
     */
    void draw();
    /**
     * @brief update
     * Met à jour le VBO pour dessiner la BBox
     * seulement s'il y a eu des changments
     */
    void update();
    /**
     * @brief setBBox
     * change la BBox, puis update()
     * @param b BBox
     */
    void setBBox(BBox *b);

private :
    void init();
    void updateCorners();

    GLuint nbVertices;
    BBox *bbox;
    glm::vec3 oldMin;
    glm::vec3 oldMax;
    std::vector<glm::vec3> corners;

    GLuint nbIndices;
    GLuint mvbo[2];
    GLuint mvao;
    float *mVertices;

};

}

#endif // BBOXMESHBUILDER_H
