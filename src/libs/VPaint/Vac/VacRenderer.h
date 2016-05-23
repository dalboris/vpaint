// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VACRENDERER_H
#define VACRENDERER_H

#include "OpenGL/OpenGLFunctions.h"
#include "Vac/Vac.h"
#include "Vac/KeyEdgeGLResources.h"

#include <QObject>
#include <QMatrix4x4>

#include <unordered_map>

class VacRendererSharedResources;

class VacRenderer: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(VacRenderer)

public:
    VacRenderer(VacRendererSharedResources * sharedResources);
    ~VacRenderer();

    VacRendererSharedResources * sharedResources() const;
    Vac * vac() const;

    void initialize(OpenGLFunctions * f);
    void update(OpenGLFunctions * f);
    void render2D(OpenGLFunctions * f, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix);
    void render3D(OpenGLFunctions * f);
    void cleanup(OpenGLFunctions * f);

public slots:
    void onTopologyChanged(const OpenVac::TopologyEditInfo & info);
    void onGeometryChanged(const OpenVac::GeometryEditInfo & info);

private:
    void createVAO_(OpenGLFunctions * f, OpenVac::CellId id);
    void updateVAO_(OpenGLFunctions * f, OpenVac::CellId id);
    void destroyVAO_(OpenGLFunctions * f, OpenVac::CellId id);

private:
    // Shared GPU resources across views
    VacRendererSharedResources * sharedResources_;

    // View-specific GPU resources
    std::unordered_map<OpenVac::CellId, KeyEdgeGLResources> keyEdgeGLResources_;

    // Information about what has changed in the Vac and therefore must be
    // (re-)sent to the GPU.
    //
    // This info is written in onTopologyChanged() and onGeometryChanged(),
    // and is read then cleared in update().
    //
    OpenVac::TopologyEditInfo topologyEditInfo_;
    OpenVac::GeometryEditInfo geometryEditInfo_;
};

#endif // VACRENDERER_H
