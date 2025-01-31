#ifndef SPRITE_H
#define SPRITE_H

#include "util.h"
#include "sm.h"
#include "texture.h"

enum DrawType {
    INSTANCE,
    INDIRECT
};

// A 2D sprite in the form of a rectangle.
class Sprite {
   public:
    struct IndirectDrawCommand {
        unsigned int indexCount;
        unsigned int instanceCount;
        unsigned int baseIndex;
        unsigned int baseVertex;
        unsigned int baseInstance;
    };
    Sprite(std::string nm, std::string tex_path, DrawType dt, bool flip = true) {
        name = nm;
        drawType = dt;
        texture = new Texture(SPRITEPATH(tex_path));
        texture->load(flip);
        size = vec2(texture->width_, texture->height_);
        transform = scale(mat4(1), vec3(size, 0));
        populateBuffers();
    }
    Sprite(std::string nm, std::string tex_path, int instances, DrawType dt, bool flip = true) {
        name = nm;
        drawType = dt;
        texture = new Texture(SPRITEPATH(tex_path));
        texture->load(flip);
        size = vec2(texture->width_, texture->height_);
        transform = scale(mat4(1), vec3(size, 0));
        totalInstanceCount = instances;
        populateBuffers();
    }

    ~Sprite() {}

    void draw();
    void draw(std::vector<mat4>);
    void generateCommands();
    void populateBuffers();

    unsigned VAO = 0;
    unsigned VBO = 0;
    unsigned EBO = 0;
    unsigned IBO = 0;
    unsigned int commandBuffer;  // draw command buffer object (compute shader)
    int totalInstanceCount = 0;  // number of instances across all variants
    int variantCount = 1;        // number of draw variants
    Texture* texture;
    DrawType drawType;
    std::string name;
    mat4 transform;
    vec2 size;

   private:
#define SP_POSITION_LOC 0
#define SP_COLOUR_LOC 1
#define SP_TEXTURE_LOC 2
#define SP_INSTANCE_LOC 3

    float vertices[32] = {
        // positions          // colors           // texture coords
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f    // top left
    };
    unsigned int indices[6] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };
};

#endif /* SPRITE_H */
