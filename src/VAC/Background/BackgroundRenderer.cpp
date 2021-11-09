// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "BackgroundRenderer.h"

#include "Background.h"

#include <QOpenGLContext>
#include <QOpenGLTexture>

BackgroundRenderer::BackgroundRenderer(
        Background * background,
        QObject * parent) :
    QObject(parent),
    background_(background),
    isCacheDirty_(false)
{
    connect(background_, SIGNAL(cacheCleared()), this, SLOT(setDirty_()));
    connect(background_, SIGNAL(destroyed()), this, SLOT(onBackgroundDestroyed_()));
}

void BackgroundRenderer::cleanup()
{
    // Delete all textures allocated in GPU
    for (QOpenGLTexture * texture: textures_)
    {
        // Note 1: Qt documentation doesn't specify whether QOpenGLTexture's
        // destructor destroys the underlying OpenGL texture object, so we
        // call destroy() explicitly before destroying texture.
        //
        // Note 2: this requires a current valid OpenGL context, reason why we
        // defer calling cleanup() via the isCacheDirty_ flag.
        //
        // Note 3: texture may be nullptr, for example if users haven't set
        // a background image
        //
        if (texture)
        {
            texture->destroy();
            delete texture;
        }
    }

    // Clear map
    textures_.clear();

    // Clear isCacheDirty_ flag
    isCacheDirty_ = false;
}

void BackgroundRenderer::setDirty_()
{
    isCacheDirty_ = true;
}

void BackgroundRenderer::clearCache_()
{
    cleanup();
}

void BackgroundRenderer::onBackgroundDestroyed_()
{
    Background * b = background_;
    background_ = nullptr;
    emit backgroundDestroyed(b);
}

QOpenGLTexture * BackgroundRenderer::texture_(int frame)
{
    // Avoid allocating several textures for frames sharing the same image.
    // If users haven't set a background image at all, this sets frame to 0.
    frame = background_->referenceFrame(frame);

    // Load texture to GPU if not done already
    if (!textures_.contains(frame))
    {
        // Get QImage
        QImage img = background_->image(frame);

        if (img.isNull())
        {
            // Set nullptr as cached value, so we won't try to re-read the file later.
            // This includes the rare cases when the image couldn't be read, but
            // also includes the very common case where no background image is
            // set.
            textures_[frame] = nullptr;
        }
        else
        {
            // Load texture to GPU.
            textures_[frame] = new QOpenGLTexture(img.mirrored());
        }
    }

    // Returned cached texture
    return textures_[frame];
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
    x1 = position[0];
    y1 = position[1];
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
    if (!background_) {
        return;
    }

    if (isCacheDirty_) {
        clearCache_();
    }

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

    // Get texture
    QOpenGLTexture * texture = texture_(frame);

    // Draw image if non-zero
    if (texture)
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
            texture->bind();
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
            texture->release();
            glDisable(GL_TEXTURE_2D);
        }
    }
}
