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

#include "XmlStreamTraverser.h"

#include "../XmlStreamReader.h"

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
