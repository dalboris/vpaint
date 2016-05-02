// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "XmlStreamTraverser.h"

#include "XmlStreamReader.h"

XmlStreamTraverser::XmlStreamTraverser(XmlStreamReader & xml) :
    xml_(xml),
    currentDepth_(0)
{
}

XmlStreamReader & XmlStreamTraverser::xml()
{
    return xml_;
}

int XmlStreamTraverser::currentDepth()
{
    return currentDepth_;
}

void XmlStreamTraverser::traverse()
{
    begin();

    do
    {
        if (xml().readNextStartElement())
        {
            pre();
            ++currentDepth_;
        }
        else
        {
            --currentDepth_;
            post();
        }
    }
    while (currentDepth_ > 0);

    end();
}
