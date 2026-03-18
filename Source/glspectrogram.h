/* ==============================================================================
    glspectrogram.cpp
    Created: 14 Jul 2019 11:41:05am
    Author:  Stefan Stenzel
============================================================================== */

#pragma once

#include <JuceHeader.h>

class glspectrogram : public juce::Component, public juce::OpenGLRenderer
{
public:
    glspectrogram();
    ~glspectrogram();

    void newOpenGLContextCreated() final;
    void openGLContextClosing() final {};
    void renderOpenGL() final;
    void newline(const float *magnitudes);

    juce::OpenGLContext ocx;
    int texture[512][512];                // texture values
    
    GLuint txtID;
    volatile int wr;
    int rd;
    int cnt;
};

