void KawaseBloom(
    gl::Texture& bloomTex,                  // result texture containing the bloom you will need to composite with the final image
    gl::Program& dualFilterUpProg,          // Kawase upsampling program
    gl::Program& dualFilterDownProg,        // Kawase downsampling program
    gl::Texture& fboTex,                    // input texture containing the data to bloom
    gl::Texture& bloomTexTmp,               // temporary ping-pong texture used by compute pass.  should be identical to bloomTex
    unsigned int downsamplePasses = 4u,
    bool brightPassFirstDownsample = true,
    float kawaseOffset = 2.0f
    )
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Kawase Bloom");

    ///\todo configurable shader generation
    const int blockSizeX = 16;
    const int blockSizeY = 16;
    const GLenum textureFormat = GL_R11F_G11F_B10F;

    const int bloomMipLevel = 1;

    GLint fboW = fboTex.GetLevelParameter<GLint>(0, (GLenum)GL_TEXTURE_WIDTH);
    GLint fboH = fboTex.GetLevelParameter<GLint>(0, (GLenum)GL_TEXTURE_HEIGHT);

    {// initial pass : downsample fbo texture (and possible bright pass) into the bloom mip chain texture
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Kawase Bloom First Downsample");

        glBindImageTexture(0, fboTex.name(), bloomMipLevel, GL_FALSE, 0, GL_READ_ONLY, textureFormat);
        glBindImageTexture(1, bloomTex.name(), 0, GL_FALSE, 0, GL_WRITE_ONLY, textureFormat);
        glBindImageTexture(2, bloomTexTmp.name(), 0, GL_FALSE, 0, GL_READ_WRITE, textureFormat);

        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

        glSugar::setFilterTrilinear(bloomTex);

        upsamplePasses = downsamplePasses-1;

        dualFilterDownProg.Use();
        dualFilterDownProg.Uniform1<GLint>("imgIn", 0);
        dualFilterDownProg.Uniform1<GLfloat>("offset", kawaseOffset);

        dualFilterDownProg.Uniform1<GLint>("brightPass", brightPassFirstDownsample ? 1 : 0);
        fboTex.BindUnit(0);
        dualFilterDownProg.Uniform1<GLint>("lod", 0);

        dualFilterDownProg.Uniform2<GLfloat>("texelSize", 1.0f / (fboW>>1), 1.0f / (fboH>>1));

        // write base level of bloom texture from fbo texture in initial pass
        glBindImageTexture(0, bloomTex.name(), 0, GL_FALSE, 0, GL_WRITE_ONLY, textureFormat);

        glDispatchCompute((fboW >> 1) / blockSizeX + 1, (fboH >> 1) / blockSizeY + 1, 1);

        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

        glPopDebugGroup();
    }

    { // Kawase downsample passes
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Kawase Bloom Downsampling Passes");

        bloomTex.BindUnit(0);

        dualFilterDownProg.Uniform1<GLint>("brightPass", 0);

        for (int i = 1; i < downsamplePasses; i++)
        {
            dualFilterDownProg.Uniform1<GLint>("lod", i-1);
            glBindImageTexture(0, bloomTex.name(), i, GL_FALSE, 0, GL_WRITE_ONLY, textureFormat);

            int filterW = (fboW >> (i + 1));
            int filterH = (fboH >> (i + 1));

            dualFilterDownProg.Uniform2<GLfloat>("texelSize", 1.0f / filterW, 1.0f / filterH);

            glDispatchCompute(filterW / blockSizeX + 1, filterH / blockSizeY + 1, 1);
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        }

        glPopDebugGroup();
    }

    { // Kawase upsampling passes
        
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Kawase Bloom Upsampling Passes");

        bloomTex.BindUnit(0);
        dualFilterUpProg.Use();
        dualFilterUpProg.Uniform1<GLint>("imgIn", 0);
        dualFilterUpProg.Uniform1<GLfloat>("offset", kawaseOffset);

        for (int i = downsamplePasses - 1; i > 0; i--)
        {
            dualFilterUpProg.Uniform1<GLint>("lod", i);

            glBindImageTexture(0, bloomTex.name(), i-1, GL_FALSE, 0, GL_WRITE_ONLY, textureFormat);

            int filterW = (fboW) >> (i);
            int filterH = (fboH) >> (i);

            dualFilterUpProg.Uniform2<GLfloat>("texelSize", 1.0f / filterW, 1.0f / filterH);

            glDispatchCompute(filterW / blockSizeX + 1, filterH / blockSizeY + 1, 1);
            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        }

        glPopDebugGroup();
    }

    glPopDebugGroup();
}