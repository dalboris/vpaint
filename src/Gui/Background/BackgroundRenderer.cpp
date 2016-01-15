// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2016 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "BackgroundRenderer.h"

#include "Background.h"

#include <QGLContext>

BackgroundRenderer::BackgroundRenderer(
        Background * background,
        QGLContext * context,
        QObject * parent) :
    QObject(parent),
    background_(background),
    context_(context)
{
    connect(background_, SIGNAL(cacheCleared()), this, SLOT(clearCache_()));
}

void BackgroundRenderer::clearCache_()
{
    // Set OpenGL context (we are likely outside paintGL())
    context_->makeCurrent();

    // Delete all textures allocated in GPU
    foreach (GLuint texId, texIds_)
    {
        context_->deleteTexture(texId);
    }

    // Clear map
    texIds_.clear();
}

GLuint BackgroundRenderer::texId_(int frame)
{
    // Avoid allocating several textures for frames sharing the same image
    frame = background_->referenceFrame(frame);

    // Load texture to GPU if not done already
    if (!texIds_.contains(frame))
    {
        // Get QImage
        QImage img = background_->image(frame);

        if (img.isNull())
        {
            // Set 0 as cached value, so we won't try to re-read the file later.
            texIds_[frame] = (GLuint) 0;
        }
        else
        {
            // Load texture to GPU.
            texIds_[frame] = context_->bindTexture(img);
        }
    }

    // Returned cached texture
    return texIds_[frame];
}

namespace
{
void computeBackgroundQuad_(
        // Input
        Background * background,
        bool showCanvas,
        double wc, double hc,
        double xc1, double xc2,
        double yc1, double yc2,
        double xSceneMin, double xSceneMax,
        double ySceneMin, double ySceneMax,

        // Output
        double & x1, double & x2,
        double & y1, double & y2,
        double & u1, double & u2,
        double & v1, double & v2,
        bool & outOfCanvas)
{
    // Get background position and size
    Eigen::Vector2d position = background->position();
    Eigen::Vector2d size = background->computedSize(Eigen::Vector2d(wc, hc));

    // Set value assuming no clamping nor repeat
    x1 = xc1 + position[0];
    y1 = yc1 + position[1];
    u1 = 0.0;
    v1 = 1.0;
    x2 = x1 + size[0];
    y2 = y1 + size[1];
    u2 = 1.0;
    v2 = 0.0;

    // Handle negative sizes
    if (x1 > x2)
    {
        std::swap(x1, x2);
        std::swap(u1, u2);
    }
    if (y1 > y2)
    {
        std::swap(y1, y2);
        std::swap(v1, v2);
    }

    // Get min and max scene coordinates where some background will be visible
    double xb1, xb2, yb1, yb2;
    if(showCanvas)
    {
        xb1 = xc1;
        xb2 = xc2;
        yb1 = yc1;
        yb2 = yc2;
    }
    else
    {
        xb1 = xSceneMin;
        xb2 = xSceneMax;
        yb1 = ySceneMin;
        yb2 = ySceneMax;
    }

    // Repeat horizontally
    if (background->repeatX())
    {
        const double dx = (x2-x1);
        const double du = (u2-u1);

        const double k1 = std::floor((xb1-x1)/dx);
        const double k2 = 1 + std::floor((xb2-x2)/dx);

        x1 += k1*dx;
        x2 += k2*dx;

        u1 += k1*du;
        u2 += k2*du;
    }

    // Repeat vertically
    if (background->repeatY())
    {
        const double dy = (y2-y1);
        const double dv = (v2-v1);

        const double k1 = std::floor((yb1-y1)/dy);
        const double k2 = 1 + std::floor((yb2-y2)/dy);

        y1 += k1*dy;
        y2 += k2*dy;

        v1 += k1*dv;
        v2 += k2*dv;
    }

    // Apply clamping
    outOfCanvas = false;
    if(showCanvas)
    {
        if ( x1 >= xc2 ||  x2 <= xc1 || y1 >= yc2 || y2 <= yc1)
        {
            outOfCanvas = true;
        }
        else
        {
            // Clamp right
            if (x2 > xc2)
            {
                u2 = u1 + (u2-u1)*(xc2-x1)/(x2-x1);
                x2 = xc2;
            }
            // Clamp left
            if (x1 < xc1)
            {
                u1 = u2 + (u1-u2)*(xc1-x2)/(x1-x2);
                x1 = xc1;
            }
            // Clamp bottom
            if (y2 > yc2)
            {
                v2 = v1 + (v2-v1)*(yc2-y1)/(y2-y1);
                y2 = yc2;
            }
            // Clamp top
            if (y1 < yc1)
            {
                v1 = v2 + (v1-v2)*(yc1-y2)/(y1-y2);
                y1 = yc1;
            }
        }
    }
}
}

void BackgroundRenderer::draw(int frame, bool showCanvas,

                              double canvasLeft, double canvasTop,
                              double canvasWidth, double canvasHeight,

                              double xSceneMin, double xSceneMax,
                              double ySceneMin, double ySceneMax)
{
    // Get canvas boundary
    const double & wc = canvasWidth;
    const double & hc = canvasHeight;
    const double & xc1 = canvasLeft;
    const double & yc1 = canvasTop;
    const double xc2 = xc1 + wc;
    const double yc2 = yc1 + hc;

    // ----- Draw background color -----

    // Set color
    glColor4d(background_->color().redF(),
              background_->color().greenF(),
              background_->color().blueF(),
              background_->color().alphaF());

    if(showCanvas)
    {
        // Draw quad covering canvas
        glBegin(GL_QUADS);
        {
            glVertex2d(xc1,yc1);
            glVertex2d(xc2,yc1);
            glVertex2d(xc2,yc2);
            glVertex2d(xc1,yc2);
        }
        glEnd();
    }
    else
    {
        // Cover the whole screen with background color.
        //
        // Note: we don't use glClear() because the background color may have
        // transparency, hence we want to perform alpha blending.

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glBegin(GL_QUADS);
        {
            glVertex2d(-1.0, -1.0);
            glVertex2d(1.0, -1.0);
            glVertex2d(1.0, 1.0);
            glVertex2d(-1.0, 1.0);
        }
        glEnd();

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    // ----- Draw background image -----

    // Get texture id
    GLuint texId = texId_(frame);

    // Draw image if non-zero
    if (texId)
    {
        // Determine background quad positions and UVs
        double x1, x2, y1, y2, u1, u2, v1, v2;
        bool outOfCanvas;
        computeBackgroundQuad_(background_, showCanvas,
                               wc, hc, xc1, xc2, yc1, yc2,
                               xSceneMin, xSceneMax, ySceneMin, ySceneMax,
                               x1, x2, y1, y2, u1, u2, v1, v2, outOfCanvas);

        // Draw textured quad
        if (!outOfCanvas)
        {
            // Set texture and modulate by opacity
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texId);
            glColor4d(1.0, 1.0, 1.0, background_->opacity());
            glColor4d(1.0, 1.0, 1.0, background_->opacity());

            // Draw quad
            glBegin(GL_QUADS);
            {
                glTexCoord2d(u1, v1); glVertex2d(x1,y1);
                glTexCoord2d(u2, v1); glVertex2d(x2,y1);
                glTexCoord2d(u2, v2); glVertex2d(x2,y2);
                glTexCoord2d(u1, v2); glVertex2d(x1,y2);
            }
            glEnd();

            // Unset texture
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        }
    }
}
