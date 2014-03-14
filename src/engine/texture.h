#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>

#include "opengl.h"


namespace vortex {

/**
 * Texture representation class
 *
 */
class Texture {
public:

    /**
     * Texture general type enumeration. Used by FBOs.
     */
    typedef enum {
        TEX_1D = 0,
        TEX_2D = 1 << 0,
        TEX_3D = 1 << 1,
        TEX_CUBE = 1 << 2,
    } TexType;

    /**
     * Constructor
     *
     * @param name Texture Object name.
     * @param target Texture target, according to OpenGL specifications.
     * @param type General type of the Texture.
     * @param zOffset Layer for 3D Textures.
     */
    Texture(std::string name, GLenum target, TexType type = TEX_2D, GLuint zOffset = 0); // default = 2D as it is the most commonly used

    /**
     * Texture OpenGL creation : the parameters accepted values are the same as for "glTexImage*".
     *
     * @param bytesperpixel Specify the number of color components in the Texture.
     * @param width Width of the Texture.
     * @param height Height of the Texture.
     * @param format Specify the texel data format.
     * @param data Specify the memory location to which the Texture have to take its texel data.
     */
    void initGL(int bytesperpixel, int width, int height, int format, int type, void * data);
    void cubeInitGL(int bytesperpixel, int width, int height, int format, int type, void** data); // for cube texture
    void volumeInitGL(int bytesperpixel, int width, int height, int depth, int format, int type, void * data);

    /**
     * Activate MipMap use for the Texture.
     *
     * @param minFilter Texture minifying function used for mipmapping.
     * @param magFilter Texture magnification function used for mipmapping.
     *
     * Note that mipmap if forbidden for rectangle target Textures.
     */
    void useMipMap(GLenum minFilter, GLenum magFilter);
    void setFilter(GLenum minFliter, GLenum magFilter);
    void setClamp(GLenum warpS, GLenum warpT);
    void setClamp(GLenum warpS, GLenum warpT, GLenum warpR);

    /**
         * Specify the OpenGL API the Texture is to be bound to the "unit"th texture unit.
         *
         * @param unit The texture unit to which the Texture is to be bound.
         */
    void bind(int unit);

    /**
     * Texture OpenGL deletion.
     */
    void deleteGL();

    /**
     * @return The OpenGL Texture id.
     */
    int getId() {
        return mTexId;
    }

    /**
     * @return The target of the Texture.
     */
    GLenum getTarget() {
        return mTarget;
    }

    /**
     * @return the general type of the Texture.
     */
    TexType getType() {
        return mType;
    }

    /**
     * @return The name of the Texture Object.
     */
    std::string getName() {
        return mName;
    }

    /**
     * @return The layer of th Texture.
     */
    GLuint getZOffset() {
        return mZOffset;
    }

    glm::vec4 getTexel(float u, float v);
private:
    std::string mName;

    unsigned char *mPixels;
    int mBytesPerPixels;
    int mWidth;
    int mHeight;

    GLuint mTexId;
    GLuint mZOffset;
    GLenum mTarget;
    TexType mType;


};

}
#endif // TEXTURE_H
