#include "sprite.h"

void Sprite::generateCommands() {
    glGenBuffers(1, &commandBuffer);

    IndirectDrawCommand *cmds = new IndirectDrawCommand[variantCount];
    unsigned int baseVertex = 0, baseInstance = 0, baseIndex = 0;
    for (int i = 0; i < variantCount; ++i) {
        int vCount = 32;  // vertices in this sprite
        int iCount = 6;  // indices in thie sprite

        cmds[i].indexCount = iCount;
        cmds[i].instanceCount = totalInstanceCount;  // number of instances this sprite will have
        cmds[i].baseIndex = baseIndex;
        cmds[i].baseVertex = baseVertex;
        cmds[i].baseInstance = baseInstance;  // index to begin new set of sprite instances

        baseVertex += vCount;
        baseIndex += iCount;
        baseInstance += totalInstanceCount;
    }

    // send command buffers to gpu
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(IndirectDrawCommand) * variantCount, &cmds[0], GL_DYNAMIC_DRAW);
}

void Sprite::populateBuffers() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &IBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(SP_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(SP_POSITION_LOC);
    // color attribute
    glVertexAttribPointer(SP_COLOUR_LOC, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(SP_COLOUR_LOC);
    // texture coord attribute
    glVertexAttribPointer(SP_TEXTURE_LOC, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(SP_TEXTURE_LOC);

    glBindBuffer(GL_ARRAY_BUFFER, IBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * 10000, NULL, GL_DYNAMIC_DRAW);
    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(SP_INSTANCE_LOC + i);
        glVertexAttribPointer(SP_INSTANCE_LOC + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (const void*)(i * sizeof(vec4)));
        glVertexAttribDivisor(SP_INSTANCE_LOC + i, 1);  // tell OpenGL this is an instanced vertex attribute.
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    if (drawType == INDIRECT) {
        generateCommands();
    }

    glBindVertexArray(0);  // unbind
}

void Sprite::draw() {
    if (drawType == INDIRECT) {
        glBindVertexArray(VAO);
        texture->bind(GL_TEXTURE0);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,     // draw triangles
            GL_UNSIGNED_INT,  // data type in indices
            (const void *)0,  // no offset; commands already bound to buffer
            variantCount,     // number of variants
            0                 // no stride
        );
    } else {
        draw({mat4(1)});
    }
}

void Sprite::draw(std::vector<mat4> ms) {
    for (auto& m : ms) m = m * transform;
    glBindVertexArray(VAO);
    texture->bind(GL_TEXTURE0);
    
    if (drawType == INDIRECT) {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,     // draw triangles
            GL_UNSIGNED_INT,  // data type in indices
            (const void *)0,  // no offset; commands already bound to buffer
            variantCount,     // number of variants
            0                 // no stride
        );
    } else { // drawType == INSTANCE
        int n = ms.size();
        glBindBuffer(GL_ARRAY_BUFFER, IBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mat4) * n, ms.data());

        glDrawElementsInstanced(
            GL_TRIANGLES,
            6,
            GL_UNSIGNED_INT,
            0,
            n
        );        
    }

    texture->unbind(GL_TEXTURE0);
    glBindVertexArray(0);
}
