// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VACRENDERERSHAREDRESOURCES_H
#define VACRENDERERSHAREDRESOURCES_H

#include "OpenGL/OpenGLFunctions.h"
#include "Vac/Vac.h"
#include "Vac/KeyEdgeGLResources.h"

#include <QObject>
#include <QOpenGLShaderProgram>

#include <unordered_map>

class VacRendererSharedResources: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(VacRendererSharedResources)

public:
    VacRendererSharedResources(Vac * vac);
    ~VacRendererSharedResources();

    Vac * vac() const;

    void initialize(OpenGLFunctions * f);
    void update(OpenGLFunctions * f);
    void cleanup(OpenGLFunctions * f);

public slots:
    void onTopologyChanged(const OpenVac::TopologyEditInfo & info);
    void onGeometryChanged(const OpenVac::GeometryEditInfo & info);

private:
    void createVBO_(OpenVac::CellId id);
    void updateVBO_(OpenVac::CellId id);
    void destroyVBO_(OpenVac::CellId id);

private:
    friend class VacRenderer;

    // Vac to render
    Vac * vac_;

    // GL resources
    QOpenGLShaderProgram shaderProgram_;
    std::unordered_map<OpenVac::CellId, KeyEdgeGLSharedResources> keyEdgeGLSharedResources_;

    // Shader locations
    int vertexLoc_;
    int projMatrixLoc_;
    int viewMatrixLoc_;

    // Reference counting to initialize and cleanup only once
    int refCount_ = 0;

    // Information about what has changed in the Vac and therefore must be
    // (re-)sent to the GPU.
    //
    // This info is written in onTopologyChanged() and onGeometryChanged(),
    // and is read then cleared in update().
    //
    OpenVac::TopologyEditInfo topologyEditInfo_;
    OpenVac::GeometryEditInfo geometryEditInfo_;
};

#endif // VACRENDERERSHAREDRESOURCES_H
