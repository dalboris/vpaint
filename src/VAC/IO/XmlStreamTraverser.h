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

#ifndef XMLSTREAMTRAVERSER_H
#define XMLSTREAMTRAVERSER_H

/// Usage:
///
/// 1) Subclass XmlStreamTraverser
/// 2) Implement pre(), post(), begin(), and end() in derived class
/// 3) Call traverse()
///
/// Example:
///
/// class MyTraverser: public XmlStreamTraverser
/// {
///     void pre()  { cout << "calling pre()"  << endl;}
///     void post() { cout << "calling post()" << endl;}
/// }
///
/// void main()
/// {
///     // ...
///     XmlStreamReader xml(file);
///     MyTraverser(file).traverse();
/// }
///
/// What happens is equivalent to the following recursive algorithm, but it is
/// implemented using an iterative algorithm to avoid stack bounds:
///
/// void recurse()
/// {
///     while (xml().readNextStartElement())
///     {
///         pre();
///         recurse();
///         post();
///     }
/// }
///
/// void traverse()
/// {
///     begin();
///     recurse();
///     end();
/// }
///
/// Note that in pre(), we just entered the XML element, and xml().name() gives
/// the name of the element we just entered. But in post(), we just left the
/// element since xml().readNextStartElement() read an ending tag instead of an
/// opening tag and returned false. So xml().name()
///
///
/// At any point during traversal, in the reimplemented pre/post/begin/end
/// callbacks, you can access the current depth of any XML element:
///   * In the begin() method, currentDepth == 0
///   * In the end() method, currentDepth == 0
///   * In the first call of pre(), currentDepth == 0, this corresponds to the XML root element
///   * In the last call of post(), currentDepth == 0, this corresponds to the XML root element
#include "vpaint_global.h"

class XmlStreamReader;

class Q_VPAINT_EXPORT XmlStreamTraverser
{
public:
    // Constructor and virtual destructor
    XmlStreamTraverser(XmlStreamReader & xml);
    virtual ~XmlStreamTraverser() {}

    // Access traversed XML
    XmlStreamReader & xml();

    // Access depth of current XML element (0 for root element)
    int currentDepth();

    // Perform traversal of XML
    void traverse();

protected:
    // Actual work to be implemented in derived classes
    virtual void pre()  {}
    virtual void post() {}
    virtual void begin() {}
    virtual void end()   {}

private:
    XmlStreamReader & xml_;
    int currentDepth_;
};

#endif // XMLSTREAMTRAVERSER_H
