# GLSugar : Modern OpenGL Helper Library

## Intro

This is intended to be a collection of utility code to make programming in MODERN OpenGL easier, more readable, and more efficient.  I think of it as my ongoing effort to make a modern GLU for OpenGL 4.5 with AZDO / DSA style programming.

This repository is built using [GLHPP](https://github.com/Steve132/glhpp) (an object oriented / RAII / DSA-oriented modern GL interface) as a base.

Other external depends include [GLAD](https://glad.dav1d.de/) for extension wrangling, and [GLFWPP](https://github.com/Steve132/glfwpp) for windowing, which is based on [GLFW](https://github.com/glfw/glfw).

## Contents

GL_Containers - STL Style containers built around GPU Buffers

GL_UBO - For declaring UBO structs in client code with correct layouts

GL_VAO - For automatically setting up VAO (Vertex Array Object) state from struct or instance data types, eg:

    glSugar::Vao<SpriteInstanceAnim> spriteVAO; // - this is the entirety of the setup code

    // at render time:
    spriteVAO.bind();
    spriteVAO.vertexBufferInstanced(v.buffer); // ready to render now!

GL_Objects - Additional helpers for setting up, initializing and using textures and shaders

IMGUI Renderer - rendering header backend for Dear IMGUI library using GLHPP / GLSugar.
