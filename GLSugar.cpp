#include <glad/glad.h>

#if defined(STB_BUILD)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#define VIRTUOSO_SHADERPROGRAMLIB_IMPLEMENTATION
#include "GL_Objects/ShaderProgram.h"
#undef VIRTUOSO_SHADERPROGRAMLIB_IMPLEMENTATION

#define VIRTUOSO_TEXTURELOADER_IMPLEMENTATION
#include "GL_Objects/Texture.h"
#undef VIRTUOSO_TEXTURELOADER_IMPLEMENTATION

#define ImguiRender_h_IMPLEMENTATION
#include "ImGUIRenderer.h"
#undef ImguiRender_h_IMPLEMENTATION
