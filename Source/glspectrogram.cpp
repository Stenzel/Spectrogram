/* ==============================================================================
    glspectrogram.cpp
    Created: 14 Jul 2019 11:41:05am
    Author:  Stefan Stenzel
============================================================================== */

#include "glspectrogram.h"

using namespace juce::gl;

glspectrogram::glspectrogram()
{
    setPaintingIsUnclipped (true);      //maybe faster
    setOpaque (true);                   //this too
    
    cnt = 0;
    rd = 0;
    wr = 0;
   
    ocx.setContinuousRepainting(true);
    ocx.setRenderer(this);
    ocx.attachTo(*this);
}

glspectrogram::~glspectrogram()
{
    ocx.detach();
}

void glspectrogram::newOpenGLContextCreated()
{
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_BLEND);

    auto img = juce::ImageCache::getFromMemory(BinaryData::cat_jpg,BinaryData::cat_jpgSize);
    for(int i=0; i<512; i++)
    {
        for(int k=0; k<512; k++)
        {
            //texture[i][k] = 0xFF000000;
            texture[i][k] =  img.getPixelAt(i,k).getARGB(); //red/blue swapped
        }
    }
    
    glGenTextures(1, &txtID);
    glBindTexture(GL_TEXTURE_2D, txtID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void glspectrogram::renderOpenGL()
{
    glBindTexture (GL_TEXTURE_2D, txtID);
    
    if(rd != wr)
    {
        glTexSubImage2D (GL_TEXTURE_2D, 0, 0, rd, 512, 1, GL_RGBA, GL_UNSIGNED_BYTE, &texture[rd][0]);
        rd = (rd + 1) & 0x1FF;
    }
    
    const float fraction = rd * 0x1p-9f;

    glBegin( GL_TRIANGLE_STRIP );

    glTexCoord2f(1.0f ,fraction);
    glVertex2f( 1.0, -1.0f );
    
    glTexCoord2f(1.0f,fraction - 1.0f);
    glVertex2f( -1.0f, -1.0f );
   
    glTexCoord2f(0.0f,fraction);
    glVertex2f( 1.0f, 1.0f );
   
    glTexCoord2f(0.0f ,fraction - 1.0f);
    glVertex2f( -1.0f, 1.0f );

    glEnd();

    glBindTexture (GL_TEXTURE_2D, 0);
}


static const unsigned int gradient[6] = {
#if 1
    0xFF000000,
    0xFF000040,                   //dark red
    0xFF004080,                   //still dark
    0xFF0080C0,                   //
    0xFF00C0FF,                   //orange
    0xFFFFFFFF                    //white
#else
    0xFF000000,                   //a bit more colorful
    0xFF800000,                   //blu
    0xFF800080,
    0xFF0000FF,                   //red
    0xFF00FFFF,                   //yellow
    0xFFFFFFFF                    //white
#endif
};

//convert magnitudes to new horizontal line in texture
void glspectrogram::newline(const float *magnitudes)
{
    const int *src = (const int *)magnitudes;       //rough logarithm, good enough for colors
    int *dst = &texture[wr][511];
    
    for(int i=0; i<512; i++,dst--)
    {
        int index = (*src++ >> 17) - 0x01C00;
        if(index >= 0x04FF)
        {
            *dst = gradient[5];
            continue;
        }
        if(index <= 0)
        {
            *dst = gradient[0];
            continue;
        }
        //interpolate colors
        int fraction = index & 0x0FF;
        index >>= 8;
        
        int left  = gradient[index];
        int right = gradient[index + 1];
        
        int redblu = (right & 0x0FF00FF) * fraction;
        redblu    += (left  & 0x0FF00FF) * (0x100 - fraction);
        
        int green = (right & 0x000FF00) * fraction;
        green    += (left  & 0x000FF00) * (0x100 - fraction);

        int rgb = (redblu & 0xFF00FF00) | (green & 0x00FF0000);
        *dst = 0xFF000000 | (rgb >> 8);
    }
    wr = (wr + 1) & 0x1FF;
}
