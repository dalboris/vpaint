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

#ifndef GLWIDGET_MATERIAL_H
#define GLWIDGET_MATERIAL_H

class GLWidget_Material
{
public:
    GLWidget_Material(QString s = QString("unnamed material")) :
        name_(s),
        
        ambientFront_r(0.8f),
        ambientFront_g(0.8f),
        ambientFront_b(0.8f),
        ambientFront_a(1.0f),
        
        diffuseFront_r(0.8f),
        diffuseFront_g(0.8f),
        diffuseFront_b(0.8f),
        diffuseFront_a(1.0f),

        specularFront_r(0.8f),
        specularFront_g(0.8f),
        specularFront_b(0.8f),
        specularFront_a(1.0f),

        emissionFront_r(0.0f),
        emissionFront_g(0.0f),
        emissionFront_b(0.0f),
        emissionFront_a(0.0f),

        shininessFront(96),
        
        ambientBack_r(0.8f),
        ambientBack_g(0.8f),
        ambientBack_b(0.8f),
        ambientBack_a(1.0f),
        
        diffuseBack_r(0.8f),
        diffuseBack_g(0.8f),
        diffuseBack_b(0.8f),
        diffuseBack_a(1.0f),
        
        specularBack_r(0.8f),
        specularBack_g(0.8f),
        specularBack_b(0.8f),
        specularBack_a(1.0f),
        
        emissionBack_r(0.0f),
        emissionBack_g(0.0f),
        emissionBack_b(0.0f),
        emissionBack_a(0.0f),
        
        shininessBack(96)
    {
    }


    GLWidget_Material(float r, float g, float b, QString s = QString("unnammed material"), float specular = 0.8f, float shininess = 96.0f) :
        name_(s),
        
        ambientFront_r(r),
        ambientFront_g(g),
        ambientFront_b(b),
        ambientFront_a(1.0f),
        
        diffuseFront_r(r),
        diffuseFront_g(g),
        diffuseFront_b(b),
        diffuseFront_a(1.0f),

        specularFront_r(specular),
        specularFront_g(specular),
        specularFront_b(specular),
        specularFront_a(1.0f),

        emissionFront_r(0.0f),
        emissionFront_g(0.0f),
        emissionFront_b(0.0f),
        emissionFront_a(0.0f),

        shininessFront(shininess),
        
        ambientBack_r(r),
        ambientBack_g(g),
        ambientBack_b(b),
        ambientBack_a(1.0f),
        
        diffuseBack_r(r),
        diffuseBack_g(g),
        diffuseBack_b(b),
        diffuseBack_a(1.0f),
        
        specularBack_r(specular),
        specularBack_g(specular),
        specularBack_b(specular),
        specularBack_a(1.0f),
        
        emissionBack_r(0.0f),
        emissionBack_g(0.0f),
        emissionBack_b(0.0f),
        emissionBack_a(0.0f),
        
        shininessBack(shininess)
    {
    }

    QString name() const {return name_;}

    QString name_;

    float ambientFront_r,
    ambientFront_g,
    ambientFront_b,
    ambientFront_a,

    diffuseFront_r,
    diffuseFront_g,
    diffuseFront_b,
    diffuseFront_a,

    specularFront_r,
    specularFront_g,
    specularFront_b,
    specularFront_a,

    emissionFront_r,
    emissionFront_g,
    emissionFront_b,
    emissionFront_a,

    shininessFront,


    ambientBack_r,
    ambientBack_g,
    ambientBack_b,
    ambientBack_a,

    diffuseBack_r,
    diffuseBack_g,
    diffuseBack_b,
    diffuseBack_a,

    specularBack_r,
    specularBack_g,
    specularBack_b,
    specularBack_a,

    emissionBack_r,
    emissionBack_g,
    emissionBack_b,
    emissionBack_a,

    shininessBack;
};

#endif
