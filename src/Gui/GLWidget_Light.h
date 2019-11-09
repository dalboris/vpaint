// Copyright (C) 2012-2019 The VPaint Developers
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


#ifndef GLWIDGET_LIGHT_H
#define GLWIDGET_LIGHT_H

#include <cmath> // for sqrt
#include <QString>

class GLWidget_Light
{
public:
    // default Light
    GLWidget_Light(QString s = QString("unnammed light")) :
        name_(s),
        
        ambient_r(0.3f),
        ambient_g(0.3f),
        ambient_b(0.3f),
        ambient_a(1.0f),
        
        diffuse_r(0.5f),
        diffuse_g(0.5f),
        diffuse_b(0.5f),
        diffuse_a(1.0f),
        
        specular_r(0.5f),
        specular_g(0.5f),
        specular_b(0.5f),
        specular_a(1.0f),
        
        position_x(0.0f),
        position_y(0.0f),
        position_z(0.0f),
        position_w(1.0f),
        
        spotDirection_x(0.0f),
        spotDirection_y(0.0f),
        spotDirection_z(-1.0f),

        spotExponent(0.0f),
        spotCutoff(180.0f),
        constantAttenuation(1.0f),
        linearAttenuation(0.0f),
        quadraticAttenuation(0.0f)
    {
    }

    // diffuse Light
    GLWidget_Light(float x, float y, float z, float r, float g, float b, QString s = QString("unnammed light")) :
        name_(s),
        
        ambient_r(0.1f),
        ambient_g(0.1f),
        ambient_b(0.1f),
        ambient_a(1.0f),
        
        diffuse_r(r),
        diffuse_g(g),
        diffuse_b(b),
        diffuse_a(1.0f),
        
        specular_r(r),
        specular_g(g),
        specular_b(b),
        specular_a(1.0f),
        
        position_x(x),
        position_y(y),
        position_z(z),
        position_w(1.0f),
        
        spotDirection_x(0.0f),
        spotDirection_y(0.0f),
        spotDirection_z(-1.0f),

        spotExponent(0.0f),
        spotCutoff(180.0f),
        constantAttenuation(1.0f),
        linearAttenuation(0.0f),
        quadraticAttenuation(0.0f)
    {
        lookAt(0,0,0);
    }

    // ambiant Light
    GLWidget_Light(float r, float g, float b, QString s = QString("unnammed light")) :
        name_(s),
        
        ambient_r(r),
        ambient_g(g),
        ambient_b(b),
        ambient_a(1.0f),
        
        diffuse_r(0.0f),
        diffuse_g(0.0f),
        diffuse_b(0.0f),
        diffuse_a(1.0f),
        
        specular_r(0.5f),
        specular_g(0.5f),
        specular_b(0.5f),
        specular_a(1.0f),
        
        position_x(0.0f),
        position_y(0.0f),
        position_z(0.0f),
        position_w(1.0f),
        
        spotDirection_x(0.0f),
        spotDirection_y(0.0f),
        spotDirection_z(-1.0f),

        spotExponent(0.0f),
        spotCutoff(180.0f),
        constantAttenuation(1.0f),
        linearAttenuation(0.0f),
        quadraticAttenuation(0.0f)
    {
        lookAt(0,0,0);
    }

    void lookAt(float x, float y, float z)
    {
        spotDirection_x = x - position_x;
        spotDirection_y = y - position_y;
        spotDirection_z = z - position_z;

        float length = sqrt(
                    spotDirection_x*spotDirection_x +
                    spotDirection_y*spotDirection_y +
                    spotDirection_z*spotDirection_z);
        if(length > 0)
        {
            spotDirection_x /= length;
            spotDirection_y /= length;
            spotDirection_z /= length;
        }
        else
        {
            spotDirection_z = - 1;
        }
    }
    
    QString name() const {return name_;}

    QString name_;

    float ambient_r,
    ambient_g,
    ambient_b,
    ambient_a,

    diffuse_r,
    diffuse_g,
    diffuse_b,
    diffuse_a,

    specular_r,
    specular_g,
    specular_b,
    specular_a,

    position_x,
    position_y,
    position_z,
    position_w,

    spotDirection_x,
    spotDirection_y,
    spotDirection_z,

    spotExponent,
    spotCutoff,
    constantAttenuation,
    linearAttenuation,
    quadraticAttenuation;
};

#endif
