//
//  ImguiRender.h
//  IMGUITEST
//
//  Created by Chris Pugh on 3/31/16.
//  Copyright © 2016 VirtuosoEngine. All rights reserved.
//


///\todo combine draws + subdata
///\todo non-matrix path

#ifndef ImguiRender_h
#define ImguiRender_h

#include <imgui.h>
#include "GL_Objects/ShaderProgram.h"
#include "GL_Objects/Texture.h"
#include <array>
#include <string_view>


struct ImguiFontManager
{
protected:

    gl::Texture fontTex;
    bool dirty;

    std::unordered_map<std::string, ImFont*> fontMappings;

    static const unsigned int DefaultFontSizePixels = 16;

public:

    static ImFontConfig defaultFontConfig()
    {
        ImFontConfig config;
        config.OversampleH = 4;
        config.OversampleV = 4;
        /*config.GlyphExtraSpacing.x = 1.0f;
        */

        return config;
    }

    ImFont* addTTFFont(const std::string& filename, unsigned int sizePixels = DefaultFontSizePixels)
    {
        return addTTFFont(filename, defaultFontConfig(), sizePixels);
    }

    ImFont* addTTFFont(const std::string& filename, const ImFontConfig& fontConfig, unsigned int sizePixels = DefaultFontSizePixels)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImFont* font1 = io.Fonts->AddFontFromFileTTF(filename.c_str(), sizePixels, &fontConfig);
        return font1;
    }

    ImguiFontManager() : fontTex(GL_TEXTURE_2D), dirty(true)
    {
        gl::Texture atlas = generateFontAtlas();
        fontTex = std::move(atlas);
    }

    gl::Texture generateFontAtlas() const
    {
        ImGuiIO& io = ImGui::GetIO();

        /*** render font atlas ***/
        unsigned char* pixels = nullptr;
        int width=0, height=0;

        io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

        gl::Texture rval = glSugar::allocateTexture(width, height, GL_R8);

        rval.SubImage2D(0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, pixels);

        rval.Bind(GL_TEXTURE_2D);

        rval.GenerateMipmap();

        glSugar::setFilterBilinear(rval);

        return rval;
    }

    const gl::Texture& fontTexture()
    {
        if (dirty)
        {
            gl::Texture atlas = generateFontAtlas();
            fontTex = std::move(atlas);
            dirty = false;
        }

        assert(fontTex.Is());
        return fontTex;
    }
};


struct ImguiRenderState
{
    gl::Program imguiProg;
    gl::Buffer vertexBuffer;
    gl::Buffer indexBuffer;
    gl::VertexArray fontVAO;

    ImguiFontManager fontManager;

    static inline constexpr std::string_view DEFAULT_API_VERSION = "#version 410 core\n";

    ImguiRenderState(const std::string_view& apiVersion = DEFAULT_API_VERSION);
    
    void renderGUI(ImDrawData* data);
};

inline constexpr std::string_view imguiVert =

R"STRING(

precision highp float;

layout (location=0) in vec2 pos;
layout (location=1) in vec2 tcs;
layout (location=2) in vec4 col;

out vec2 coords;
out vec4 color;

uniform vec2 scale;
uniform mat4 mvp;

void main()
{
    coords = tcs;
    color = col;
    
    vec2 pos2 = fma(pos, scale, vec2(-1.0, 1.0));
    
    gl_Position = mvp * vec4(pos2, 0.0, 1.0);
}

)STRING";


inline constexpr const std::string_view imguiFrag =
R"STRING(

precision highp float;

in vec2 coords;
in vec4 color;
out vec4 col;

uniform sampler2D tex;

void main(void)
{
    col = color * texture(tex, coords).r;
}

)STRING";

#endif /* ImguiRender_h */


#ifdef ImguiRender_h_IMPLEMENTATION

void ImguiRenderState::renderGUI(ImDrawData* data)
{
    assert(imguiProg.Is());
    assert(vertexBuffer.Is());
    assert(indexBuffer.Is());
    assert(fontVAO.Is());


    ImVec2 clip_off = data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = data->FramebufferScale;
    int fb_height = (int)(data->DisplaySize.y * data->FramebufferScale.y);

    ImGuiIO& io = ImGui::GetIO();

    imguiProg.Use();

    imguiProg.Uniform("scale", 2.0f / io.DisplaySize.x, -2.0f / io.DisplaySize.y);

    fontVAO.Bind();

    const gl::Texture& fontTex = fontManager.fontTexture();

    glActiveTexture(GL_TEXTURE0);
    fontTex.Bind(GL_TEXTURE_2D);

    GLuint boundTex = fontTex.name();

    bool scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);

    std::array<GLint, 4> scissorBoxOld;
    glGetIntegerv(GL_SCISSOR_BOX, &scissorBoxOld[0]);

    if (!scissorEnabled)
    {
        glEnable(GL_SCISSOR_TEST);
    }

    for (int i = 0; i < data->CmdListsCount; i++)
    {
        ImDrawList& drawList = *data->CmdLists[i];

        indexBuffer.Data (sizeof(ImDrawIdx)  * drawList.IdxBuffer.size(),  drawList.IdxBuffer.Data, GL_STATIC_DRAW);

        vertexBuffer.Data(sizeof(ImDrawVert) * drawList.VtxBuffer.size() , drawList.VtxBuffer.Data, GL_STATIC_DRAW);

        GLushort* offset = 0;

        for (int cmds =0; cmds< drawList.CmdBuffer.size(); cmds++)
        {
          ImDrawCmd& cmd = drawList.CmdBuffer[cmds];

          ImDrawCmd* pcmd = &cmd;

          GLuint tex = (int)(pcmd->TextureId);
          tex = (tex == 0) ? fontTex.name() : tex;

          // prevent spamming texture state changes
          if (tex != boundTex)
          {
              glBindTexture(GL_TEXTURE_2D, tex);
              boundTex = tex;
          }

          ImVec4 clip_rect;
          clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
          clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
          clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
          clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;
          
          // Apply scissor/clipping rectangle
          glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
          
          glDrawElements(GL_TRIANGLES, cmd.ElemCount, GL_UNSIGNED_SHORT, (void*)offset);

          offset += cmd.ElemCount;
        }
    }

    // reset state

    if (!scissorEnabled)
    {
        glDisable(GL_SCISSOR_TEST);
    }

    glScissor(scissorBoxOld[0], scissorBoxOld[1], scissorBoxOld[2], scissorBoxOld[3]);
}

ImguiRenderState::ImguiRenderState(const std::string_view& apiVersion)
    :
    imguiProg(glSugar::VertFragProgram(std::string(apiVersion).append(imguiVert), std::string(apiVersion).append(imguiFrag)))
{
    std::clog << "ImDrawVert : size : " << sizeof(ImDrawVert) << std::endl;

    // enable the attribs
    fontVAO.EnableAttrib(0);
    fontVAO.EnableAttrib(1);
    fontVAO.EnableAttrib(2);

    // set the vao's vertex layout
    fontVAO.AttribFormat(0, 2, GL_FLOAT, GL_FALSE, offsetof(ImDrawVert,pos));
    fontVAO.AttribFormat(1, 2, GL_FLOAT, GL_FALSE, offsetof(ImDrawVert,uv));
    fontVAO.AttribFormat(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(ImDrawVert,col));

    // set the attibs to a vertex buffer binding point
    fontVAO.AttribBinding(0, 0);
    fontVAO.AttribBinding(1, 0);
    fontVAO.AttribBinding(2, 0);

    // set the vertex buffer, pointer, and stride.  this call is cheap to change
    fontVAO.VertexBuffer(0, vertexBuffer, 0, sizeof(ImDrawVert));

    fontVAO.ElementBuffer(indexBuffer);

    std::array < float, 16 > id = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };

    imguiProg.Uniform("tex", 0);
    
    imguiProg.UniformMatrix4("mvp", 1, false, id.data());
}


#endif
