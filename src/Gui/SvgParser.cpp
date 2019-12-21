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

#include "SvgParser.h"

#include <cmath>
#include <deque>
#include <regex>
#include <sstream>
#include <stack>
#include <vector>

#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QMessageBox>
#include <QRegExp>
#include <QStack>
#include <QString>
#include <QStringRef>
#include <QVector>
#include <QtGlobal>
#include <QtMath>

#include "Global.h"
#include "MainWindow.h"
#include "Scene.h"
#include "SvgImportParams.h"
#include "VectorAnimationComplex/EdgeGeometry.h"
#include "VectorAnimationComplex/EdgeSample.h"
#include "VectorAnimationComplex/KeyEdge.h"
#include "VectorAnimationComplex/KeyFace.h"
#include "VectorAnimationComplex/KeyVertex.h"
#include "VectorAnimationComplex/SculptCurve.h"
#include "VectorAnimationComplex/VAC.h"
#include "View.h"
#include "XmlStreamReader.h"

using VectorAnimationComplex::Cycle;
using VectorAnimationComplex::EdgeGeometry;
using VectorAnimationComplex::EdgeSample;
using VectorAnimationComplex::KeyEdge;
using VectorAnimationComplex::KeyFace;
using VectorAnimationComplex::KeyHalfedge;
using VectorAnimationComplex::KeyVertex;
using VectorAnimationComplex::LinearSpline;
using VectorAnimationComplex::VAC;

using EdgeSamples = std::vector<EdgeSample, Eigen::aligned_allocator<EdgeSample>>;

using Transform = Eigen::Affine2d;
using TransformDeque = std::deque<Transform, Eigen::aligned_allocator<Transform>>;
using TransformStack = std::stack<Transform, TransformDeque>;

namespace {

// Applies the given transform to the given width.
//
// Note that as per spec, the transform also affects stroke-width. In case of
// non-uniform scaling (or skewing), we can't really be fully compliant (see
// https://stackoverflow.com/q/10357292 for what compliance looks like in case
// of non-uniform scaling), so we just scale the stroke width by sqrt(|det(t)|),
// which is basically the geometric mean of the x-scale and y-scale. We could
// do a bit better by taking the stroke tangent into account, but this would
// complicate the architecture a bit for something which is probably a rarely
// used edge case, and would still not be 100% compliant anyway.
//
// Also note that SVG Tiny 1.2 and SVG 2 define a "non-scaling-size" vector
// effect, which makes stroke-width ignoring the current transform. We don't
// implement that, but the implementation notes on SVG 2 are where we used the
// inspiration for choosing sqrt(|det(t)|) as our scale factor:
//
// https://www.w3.org/TR/2018/CR-SVG2-20181004/coords.html#VectorEffects
//
double applyTransform(const Transform& t, double width)
{
    // Note: Ideally, we may want to cache meanScale for performance
    double meanScale = std::sqrt(std::abs(t(0,0)*t(1,1) - t(1,0)*t(0,1)));
    return meanScale * width;
}

// Applies the given transform to the given Vector2d.
//
Eigen::Vector2d applyTransform(const Transform& t, const Eigen::Vector2d& v)
{
    return t * v;
}

// Applies the given transform to the given EdgeSample.
//
EdgeSample applyTransform(const Transform& t, const EdgeSample& s)
{
    Eigen::Vector2d v = applyTransform(t, Eigen::Vector2d(s.x(), s.y()));
    double w = applyTransform(t, s.width());
    return EdgeSample(v[0], v[1], w);
}

// All possible path command types.
//
enum class SvgPathCommandType : unsigned char {
    ClosePath = 0, // Z (none)
    MoveTo    = 1, // M (x y)+
    LineTo    = 2, // L (x y)+
    HLineTo   = 3, // H x+
    VLineTo   = 4, // V y+
    CCurveTo  = 5, // C (x1 y1 x2 y2 x y)+
    SCurveTo  = 6, // S (x2 y2 x y)+
    QCurveTo  = 7, // Q (x1 y1 x y)+
    TCurveTo  = 8, // T (x y)+
    ArcTo     = 9  // A (rx ry x-axis-rotation large-arc-flag sweep-flag x y)+
};

// All possible argument types of path commands.
//
enum class SvgPathArgumentType : unsigned char {
    Number,
    Unsigned,
    Flag
};

// Returns the signature of the given path command type, that is, the
// description of the number and types of its arguments.
//
const std::vector<SvgPathArgumentType>& signature(SvgPathCommandType commandType)
{
    using a = SvgPathArgumentType;
    static const std::vector<SvgPathArgumentType> s[10] = {
        /* ClosePath */ {},
        /* MoveTo    */ {a::Number, a::Number},
        /* LineTo    */ {a::Number, a::Number},
        /* HLineTo   */ {a::Number},
        /* VLineTo   */ {a::Number},
        /* CCurveTo  */ {a::Number, a::Number, a::Number, a::Number, a::Number, a::Number},
        /* SCurveTo  */ {a::Number, a::Number, a::Number, a::Number},
        /* QCurveTo  */ {a::Number, a::Number, a::Number, a::Number},
        /* TCurveTo  */ {a::Number, a::Number},
        /* Arc       */ {a::Unsigned, a::Unsigned, a::Number, a::Flag, a::Flag, a::Number, a::Number}
    };
    return s[static_cast<unsigned char>(commandType)];
}

// Represents one path command, that is, a command character followed by all
// its arguments, possibly implicitely repeated. For example, the string
//
//   L 10 10 10 20
//
// can be represented as one SvgPathCommand, but is represented as two
// SvgPathCommands when normalized:
//
//   L 10 10 L 10 20
//
struct SvgPathCommand {
    SvgPathCommandType type;
    bool relative;
    std::vector<double> args;

    SvgPathCommand(SvgPathCommandType type, bool relative, std::vector<double>&& args) :
        type(type), relative(relative), args(args) {}
};

// Returns whether the string [it, end) starts with a number (or an unsigned
// number if `isSignAllowed` is false), as defined by the SVG 1.1 grammar:
//
//   https://www.w3.org/TR/SVG11/paths.html#PathDataBNF
//
//   number:   sign? unsigned
//   unsigned: ((digit+ "."?) | (digit* "." digit+)) exp?
//   exp:      ("e" | "E") sign? digit+
//   sign:     "+" | "-"
//   digit:    "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
//
// If a number is found, then the iterator `it` is advanced to the position
// just after the number. Otherwise, it is left unchanged.
//
// If a number is found, and the optional output parameter `number` is given,
// then it is set to the value of the number. Otherwise, it is left unchanged.
//
// Note: This function does NOT ignore leading whitespaces, that is,
// readNumber(" 42") returns false.
//
// Note: This function consumes as much as possible of the input string, as per
// the SVG grammar specification:
//
//   https://www.w3.org/TR/SVG11/paths.html#PathDataBNF
//
//   The processing of the BNF must consume as much of a given BNF production
//   as possible, stopping at the point when a character is encountered which
//   no longer satisfies the production. Thus, in the string "M 100-200", the
//   first coordinate for the "moveto" consumes the characters "100" and stops
//   upon encountering the minus sign because the minus sign cannot follow a
//   digit in the production of a "coordinate". The result is that the first
//   coordinate will be "100" and the second coordinate will be "-200".
//
//   Similarly, for the string "M 0.6.5", the first coordinate of the "moveto"
//   consumes the characters "0.6" and stops upon encountering the second
//   decimal point because the production of a "coordinate" only allows one
//   decimal point. The result is that the first coordinate will be "0.6" and
//   the second coordinate will be ".5".
//
// Note: In SVG 2, trailing commas have been disallowed, that is, "42." is a
// valid number in SVG 1.1, but invalid in SVG 2. We continue to accept them
// regardless. See:
//
//   https://svgwg.org/svg2-draft/paths.html#PathDataBNF
//
//   The grammar of previous specifications allowed a trailing decimal point
//   without any decimal digits for numbers (e.g 23.). SVG 2 harmonizes number
//   parsing with CSS [css-syntax-3], disallowing the relaxed grammar for
//   numbers. However, user agents may continue to accept numbers with trailing
//   decimal points when parsing is unambiguous. Authors and authoring tools
//   must not use the disallowed number format.
//
bool readNumber(
        bool isSignAllowed,
        std::string::const_iterator& it,
        std::string::const_iterator end,
        double* number = nullptr)
{
    // Build once and cache the signed regex
    static const std::regex sregex(
        "[+-]?(([0-9]+\\.?)|([0-9]*\\.[0-9]+))([eE][+-]?[0-9]+)?",
        std::regex::extended | // Use "leftmost longest" rule
        std::regex::optimize); // Faster searches, slower constructor

    // Build once and cache the unsigned regex
    static const std::regex uregex(
        "(([0-9]+\\.?)|([0-9]*\\.[0-9]+))([eE][+-]?[0-9]+)?",
        std::regex::extended | // Use "leftmost longest" rule
        std::regex::optimize); // Faster searches, slower constructor

    // Perform the regex search
    std::smatch match;
    bool isNumber = std::regex_search(
        it, end, match,                          // Search from it to end
        isSignAllowed ? sregex : uregex,         // Which regex to use
        std::regex_constants::match_continuous); // Only match at beginning of range

    // Advance iterator, convert to double, and return whether found.
    //
    // Note: aside from unlikely out-of-memory errors, the conversion can't
    // fail since the SVG number grammar is a subset of the C++ number grammar.
    //
    // Note: repeatedly constructing an std::istringstream is slow. For
    // performance, cppreference recommends reusing a unique istringstream, and
    // resetting its value via str(). See:
    //
    // https://en.cppreference.com/w/cpp/io/basic_istringstream/basic_istringstream
    //
    // Unfortunately, passing such pre-allocated istringstream to this function
    // would complicate its API, and globally constructing it (e.g., as a
    // static local variable), would make this function neither thread-safe nor
    // reentrant.
    //
    // Also, note that even using str(), we need first to copy the data from d
    // to a new string s, then pass this string to istringstream::str(s), which
    // performs a copy itself. This is not going to be fast either. In VGC, we
    // implement our own string to double conversion to avoid all those
    // unnecessary allocations and copies.
    //
    // Finally, note that we need to use the "C" locale for string to double
    // conversions, that is, use "." as decimal point regardless of global user
    // preferences. See:
    //
    // See: https://en.cppreference.com/w/cpp/locale/num_get
    //
    if (isNumber) {
        auto numEnd = it + match.length(0);
        if (number) {
            std::string s(it, numEnd);
            std::istringstream in(s);
            in.imbue(std::locale::classic());
            in >> *number;
        }
        it = numEnd;
    }
    return isNumber;
}

// Calls readNumber() with isSignedAllowed = true.
//
bool readNumber(
        std::string::const_iterator& it,
        std::string::const_iterator end,
        double* number = nullptr)
{
    return readNumber(true, it, end, number);
}

// Returns whether the given QString starts with a number.
//
// If a number is found, and the optional output parameter `number` is given,
// then it is set to the value of the number. Otherwise, it is left unchanged.
//
bool readNumber(const QString& s, double* number = nullptr)
{
    std::string value = s.toStdString();
    auto it = value.cbegin();
    auto end = value.cend();
    return readNumber(it, end, number);
}

// Calls readNumber() with isSignedAllowed = false.
//
bool readUnsigned(
        std::string::const_iterator& it,
        std::string::const_iterator end,
        double* number = nullptr)
{
    return readNumber(false, it, end, number);
}

// Returns whether the string [it, end) starts with a flag, that is, the
// character '0' or '1'.
//
// If a flag is found, then the iterator `it` is advanced to the position
// just after the flag. Otherwise, it is left unchanged.
//
// If a flag is found, and the optional output parameter `number` is given,
// then it is set to the value of the flag expressed as a double (0.0 or 1.0).
//
// Note: This function does NOT ignore leading whitespaces, that is,
// isFlag(" 0") returns false.
//
bool readFlag(
        std::string::const_iterator& it,
        std::string::const_iterator end,
        double* number = nullptr)
{
    if (it != end && (*it == '0' || *it == '1')) {
        if (number) {
            *number = (*it == '0') ? 0.0 : 1.0;
        }
        ++it;
        return true;
    }
    else {
        return false;
    }
}

// Returns whether the given character is a whitespace character.
//
bool isWhitespace(char c)
{
    // Note: CSS accepts form feeds ('\f' or 0xC in C++), but SVG doesn't.
    //
    return c == 0x20 || c == 0x9  || c == 0xD  || c == 0xA;
}

// Returns whether the given character is a [a-zA-Z] character.
//
bool isAlphanum(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

// Returns whether the given character is a [0-9] character.
//
bool isDigit(char c)
{
    return '0' <= c && c <= '9';
}

// Advances the given iterator `it` forward until a non-whitespace character or
// the `end` is found.
//
// Returns whether at least one character was read.
//
bool readWhitespaces(
        std::string::const_iterator& it,
        std::string::const_iterator end)
{
    auto it0 = it;
    while (it != end && isWhitespace(*it)) {
        ++it;
    }
    return it0 != it;
}

// Advances the given iterator `it` forward until an open parenthesis is found non-whitespace-non-comma
// character or the `end` is found. Only one comma is allowed, that is, if a
// second comma is encountered, it stops reading just before the second comma.
//
// Returns whether at least one character was read.
//
bool readCommaWhitespaces(
        std::string::const_iterator& it,
        std::string::const_iterator end)
{
    auto it0 = it;
    readWhitespaces(it, end);
    if (it != end && *it == ',') {
        ++it;
        readWhitespaces(it, end);
    }
    return it0 != it;
}

// Returns whether the string [it, end) starts with a function name, that is, a
// [a-zA-Z_] character, followed by any number of [a-zA-Z0-9_-] characters.
//
// If a function name is found, then the iterator `it` is advanced to the
// position just after the function name. Otherwise, it is left unchanged.
//
// If a function name is found, and the optional output parameter
// `functionName` is given, then it is set to the name of the function.
// Otherwise, it is left unchanged.
//
// Note: This function does NOT ignore leading whitespaces, that is,
// readFunctionName(" scale") returns false.
//
// Note: Unlike generic CSS functions, but like all transform functions, we do
// not accept functions starting with `--` or `-`, or including non-ASCII
// characters or espace sequences.
//
bool readFunctionName(
        std::string::const_iterator& it,
        std::string::const_iterator end,
        std::string* functionName = nullptr)
{
    auto it0 = it;

    // Read first [a-zA-Z_] characters
    if (it != end && (isAlphanum(*it) || *it == '_')) {
        ++it;
    }
    else {
        it = it0;
        return false;
    }

    // Read subsequent [a-zA-Z0-9_] characters
    while (it != end && (isAlphanum(*it) || isDigit(*it) || *it == '_' || *it == '-')) {
        ++it;
    }
    if (functionName) {
        functionName->assign(it0, it);
    }
    return true;
}

// Returns whether the string [it, end) starts with a function
// call, that is:
//
// function-name: [a-zA-Z_] [a-zA-Z0-9_-]*
// function-args: number (comma-wsp? number)*
// function-call: function-name wsp* '(' wsp* function-args? wsp* ')'
//
// If a function call is found, then the iterator `it` is advanced to the
// position just after the close parenthesis. Otherwise, it is left unchanged.
//
// If a function call is found, and the optional output parameter
// `functionName` is given, then it is set to the name of the function.
// Otherwise, it is set to an empty string.
//
// If a function call is found, and the optional output parameter `args` is
// given, then it is set to the list of arguments of the function. Otherwise,
// it is set to an empty list.
//
// Note: This function does NOT ignore leading whitespaces, that is,
// readFunctionCall(" scale(2)") returns false.
//
// Note: CSS doesn't allow for whitespaces between a function name and the open
// parenthesis, but the transform attribute of SVG does:
//
// SVG 1.1: https://www.w3.org/TR/SVG11/coords.html#TransformAttribute
// SVG 2:   https://drafts.csswg.org/css-transforms/#svg-syntax
// CSS 3:   https://drafts.csswg.org/css-syntax-3/#function-token-diagram
//
bool readFunctionCall(
        std::string::const_iterator& it,
        std::string::const_iterator end,
        std::string* functionName = nullptr,
        std::vector<double>* args = nullptr)
{
    auto it0 = it;

    // Read function name
    if (!readFunctionName(it, end, functionName)) {
        it = it0;
        if (functionName) functionName->clear();
        if (args) args->clear();
        return false;
    }

    // Read whitespaces and open parenthesis
    readWhitespaces(it, end);
    if (it != end && *it == '(') {
        ++it;
    }
    else {
        it = it0;
        if (functionName) functionName->clear();
        if (args) args->clear();
        return false;
    }

    // Read arguments
    if (args) args->clear();
    bool readArgs = true;
    bool isFirstArg = true;
    while (readArgs) {
        auto itBeforeArg = it;
        if (isFirstArg) {
            readWhitespaces(it, end);
        }
        else {
            readCommaWhitespaces(it, end);
        }
        double number;
        if (readNumber(it, end, &number)) {
            if (args) args->push_back(number);
        }
        else {
            it = itBeforeArg; // move before comma if any
            readArgs = false;
        }
        isFirstArg = false;
    }

    // Read whitespaces and close parenthesis
    readWhitespaces(it, end);
    if (it != end && *it == ')') {
        ++it;
        return true;
    }
    else {
        // => Error: invalid arg or missing close parenthesis
        it = it0;
        if (functionName) functionName->clear();
        if (args) args->clear();
        return false;
    }
}

// Parses the given string into a transform.
//
// Note that it is unclear from the SVG specification which exact syntax is
// allowed, as it has slightly changed from SVG 1.1 to SVG 2 (= CSS Transforms
// Module Level 1):
//
// https://www.w3.org/TR/SVG11/coords.html#TransformAttribute
// https://drafts.csswg.org/css-transforms/#svg-syntax
//
// - SVG 1.1 forces at least one comma-wsp between transform functions (ex1 =
//   "scale(2)scale(3)" is forbidden), but allows for multiple commas (ex2 =
//   "scale(2),,scale(3)" is allowed). On the other hand, in SVG 2, ex1 is
//   allowed, but ex2 is forbidden.
//
//   SVG 1.1:  transforms: transform | transform comma-wsp+ transforms
//   SVG 2:    transforms: transform | transform wsp* comma-wsp? transforms
//
// - In SVG 1.1, a comma-wsp is mandatory between arguments of a transform
//   function, while it is optional in SVG 2 (i.e., it allows "100-200" like in
//   path data).
//
//   SVG 1.1:  scale: "scale" wsp* "(" wsp* number (comma-wsp  number)? wsp* ")"
//   SVG 2:    scale: "scale" wsp* "(" wsp* number (comma-wsp? number)? wsp* ")"
//
// Therefore, we take a liberal approach and accept them all, using the SVG 2
// syntax for function arguments, and the following syntax for transforms:
//
//   transforms:     transform | transform comma-wsp* transforms
//   transform-list: wsp* transforms? wsp*
//
Transform parseTransform(const std::string& s)
{
    Transform res = Transform::Identity();
    auto it = s.cbegin();
    auto end = s.cend();
    bool readFunctions = true;
    bool isFirstFunction = true;
    while (readFunctions) {
        auto itBeforeFunction = it;
        if (isFirstFunction) {
            readWhitespaces(it, end);
        }
        else {
            while (readCommaWhitespaces(it, end)) {
                // keep reading comma-whitespaces
            }
        }
        std::string functionName;
        std::vector<double> args;
        if (readFunctionCall(it, end, &functionName, &args)) {
            if (functionName == "matrix") { // a b c d e f
                if (args.size() != 6) {
                    // Error: incorrect number of arguments
                    return Transform::Identity();
                }
                Eigen::Matrix<double, 2, 3> m;
                m << args[0], args[2], args[4],
                     args[1], args[3], args[5];
                res *= m;
            }
            else if (functionName == "translate") { // tx [ty=0]
                if (args.size() != 1 && args.size() != 2) {
                    // Error: incorrect number of arguments
                    return Transform::Identity();
                }
                if (args.size() == 1) {
                    args.push_back(0.0);
                }
                res *= Eigen::Translation2d(args[0], args[1]);
            }
            else if (functionName == "scale") { // sx [sy=sx]
                if (args.size() != 1 && args.size() != 2) {
                    // Error: incorrect number of arguments
                    return Transform::Identity();
                }
                if (args.size() == 1) {
                    args.push_back(args[0]);
                }
                res *= Eigen::Scaling(args[0], args[1]);
            }
            else if (functionName == "rotate") { // angle [cx=0 cy=0]
                if (args.size() != 1 && args.size() != 3) {
                    // Error: incorrect number of arguments
                    return Transform::Identity();
                }
                if (args.size() == 1) {
                    args.push_back(0.0);
                    args.push_back(0.0);
                }
                res *= Eigen::Translation2d(args[1], args[2]);
                res *= Eigen::Rotation2Dd(args[0] / 180.0 * M_PI);
                res *= Eigen::Translation2d(-args[1], args[2]);
            }
            else if (functionName == "skewX") { // angle
                if (args.size() != 1) {
                    // Error: incorrect number of arguments
                    return Transform::Identity();
                }
                double t = std::tan(args[0] / 180.0 * M_PI);
                Eigen::Matrix2d m;
                m << 1, t,
                     0, 1;
                res *= m;
            }
            else if (functionName == "skewY") { // angle
                if (args.size() != 1) {
                    // Error: incorrect number of arguments
                    return Transform::Identity();
                }
                double t = std::tan(args[0] / 180.0 * M_PI);
                Eigen::Matrix2d m;
                m << 1, 0,
                     t, 1;
                res *= m;
            }
            else {
                // Error: Unknown function
                return Transform::Identity();
            }
        }
        else {
            it = itBeforeFunction; // move before commas if any
            readFunctions = false;
        }
        isFirstFunction = false;
    }
    readWhitespaces(it, end);
    if (it != end) {
        // Error: unexpected character
        return Transform::Identity();
    }
    else {
        return res;
    }
}

// Parses the given path data string `d` into a sequence of SvgPathCommands,
// according to the SVG 1.1 grammar:
//
//   https://www.w3.org/TR/SVG11/paths.html#PathDataBNF
//
// In case of invalid syntax, an error string is written to the optional output
// parameter `error`, and the returned SvgPathCommands is the path data up to
// (but not including) the first command segment with an invalid syntax, as per
// the SVG recommendation:
//
//   https://www.w3.org/TR/SVG11/implnote.html#PathElementImplementationNotes
//   https://svgwg.org/svg2-draft/paths.html#PathDataErrorHandling
//
//   The general rule for error handling in path data is that the SVG user
//   agent shall render a ‘path’ element up to (but not including) the path
//   command containing the first error in the path data specification. This
//   will provide a visual clue to the user or developer about where the error
//   might be in the path data specification. This rule will greatly discourage
//   generation of invalid SVG path data.
//
//   If a path data command contains an incorrect set of parameters, then the
//   given path data command is rendered up to and including the last correctly
//   defined path segment, even if that path segment is a sub-component of a
//   compound path data command, such as a "lineto" with several pairs of
//   coordinates. For example, for the path data string 'M 10,10 L 20,20,30',
//   there is an odd number of parameters for the "L" command, which requires
//   an even number of parameters. The user agent is required to draw the line
//   from (10,10) to (20,20) and then perform error reporting since 'L 20 20'
//   is the last correctly defined segment of the path data specification.
//
//   Wherever possible, all SVG user agents shall report all errors to the
//   user.
//
std::vector<SvgPathCommand> parsePathData(
        const std::string& d, std::string* error = nullptr)
{
    using t = SvgPathCommandType;
    using a = SvgPathArgumentType;
    auto it = d.cbegin();
    auto end = d.cend();
    std::vector<SvgPathCommand> cmds;
    readWhitespaces(it, end);
    while (it != end) {

        // Read command type and relativeness
        SvgPathCommandType type;
        bool relative;
        switch(*it) {
        case 'Z': type = t::ClosePath; relative = false; break;
        case 'M': type = t::MoveTo;    relative = false; break;
        case 'L': type = t::LineTo;    relative = false; break;
        case 'H': type = t::HLineTo;   relative = false; break;
        case 'V': type = t::VLineTo;   relative = false; break;
        case 'C': type = t::CCurveTo;  relative = false; break;
        case 'S': type = t::SCurveTo;  relative = false; break;
        case 'Q': type = t::QCurveTo;  relative = false; break;
        case 'T': type = t::TCurveTo;  relative = false; break;
        case 'A': type = t::ArcTo;     relative = false; break;

        case 'z': type = t::ClosePath; relative = true; break;
        case 'm': type = t::MoveTo;    relative = true; break;
        case 'l': type = t::LineTo;    relative = true; break;
        case 'h': type = t::HLineTo;   relative = true; break;
        case 'v': type = t::VLineTo;   relative = true; break;
        case 'c': type = t::CCurveTo;  relative = true; break;
        case 's': type = t::SCurveTo;  relative = true; break;
        case 'q': type = t::QCurveTo;  relative = true; break;
        case 't': type = t::TCurveTo;  relative = true; break;
        case 'a': type = t::ArcTo;     relative = true; break;

        default:
            // Unknown command character, or failed to parse first arg
            // of non-first argtuple of previous command.
            if (error) {
                *error = "Failed to read command type or argument: ";
                *error += *it;
            }
            return cmds;
        }

        // Ensure first command is a MoveTo
        if (cmds.empty() && type != t::MoveTo) {
            if (error) {
                *error = "First command must be 'M' or 'm'. Found '";
                *error += *it;
                *error += "' instead.";
            }
            return cmds;
        }

        // Advance iterator on success
        ++it;

        // Read command arguments, unless the command take zero arguments.
        const std::vector<SvgPathArgumentType>& sig = signature(type);
        bool readArgtuples = (sig.size() > 0);
        bool isFirstArgtuple = true;
        bool hasError = false;
        std::vector<double> args;
        args.reserve(sig.size());
        while (readArgtuples) {
            auto itBeforeArgtuple = it;
            if (isFirstArgtuple) {
                readWhitespaces(it, end);
            }
            else {
                readCommaWhitespaces(it, end);
            }
            for (size_t i = 0; i < sig.size(); ++i) {
                if (i != 0) {
                    readCommaWhitespaces(it, end);
                }
                // Check whether next symbol is a valid argument
                bool isArg;
                double number;
                switch (sig[i]) {
                case a::Number:   isArg = readNumber(it, end, &number);   break;
                case a::Unsigned: isArg = readUnsigned(it, end, &number); break;
                case a::Flag:     isArg = readFlag(it, end, &number);     break;
                }
                if (isArg) {
                    // If there's an argument, keep reading
                    args.push_back(number);
                }
                else {
                    // If there's no valid argument, but an argument was
                    // mandatory, then drop previous args in argtuple, and
                    // report error.
                    if (i != 0 || isFirstArgtuple) {
                        hasError = true;
                        if (error) {
                            *error = "Failed to read argument.";
                        }
                        while (i > 0) {
                            args.pop_back();
                            --i;
                        }
                    }
                    // Whether it's an error or not, since there's no valid
                    // argument, we stop reading args for this command, and
                    // move on to the next command. Note that we need to
                    // move back the iterator to where it was before attempting
                    // to read arguments, since a comma may have been read, which
                    // is allowed between argtuples, but not allowed between
                    // an argtuple and the next command.
                    it = itBeforeArgtuple;
                    readArgtuples = false;
                    break;
                }
            }
            isFirstArgtuple = false;
        }

        // Add command to path data. Note that even in case of errors, we still
        // add the command if at least one argtuple was successfully read.
        if (!hasError || args.size() > 0) {
            cmds.push_back(SvgPathCommand(type, relative, std::move(args)));
        }

        // Return now in case of errors in argument parsing
        if (hasError) {
            return cmds;
        }

        // Read whitespaces and move on to the next command
        readWhitespaces(it, end);
    }
    return cmds;
}

// Populates the given `samples` with 12 new samples, tracing the line segment
// [p, q]. The new samples are not spaced uniformly, but instead they follow a
// geometric progression to avoid overshooting artifacts at the extremeties
// (i.e., samples are closer from each other at the ends of the line segments,
// so that corners stay sharp).
//
// WARNING: Be careful not to pass p or q as references to EdgeSamples within
// the `samples` vector (e.g., DO NOT do something like:
// addLineSamples(samples, samples.back(), q)), since the `samples` vector is
// populated, which may invalidate previous references.
//
void addLineSamples(EdgeSamples& samples, const EdgeSample& p, const EdgeSample& q)
{
    // We double the space each time up to u = 0.5 (then use symmetric values):
    //   u0 = 0
    //   u1 = 0.01
    //   u2 = u1 + 2*(u1-u0) = 0.03
    //   u3 = u2 + 2*(u2-u1) = 0.07
    //   ...
    //
    // Instead of using a factor 2, we may in theory even use a factor up to 8
    // without overshooting, but the lowest the factor, the less artifact you
    // get.
    //
    static const std::vector<double> u_ = {
        0.01, 0.03, 0.07, 0.15, 0.31, 0.5, 0.69, 0.85, 0.93, 0.97, 0.99, 1.0 };

    for (double u : u_) {
        samples.push_back(p.lerp(u, q));
    }
}

// This function populates the given VAC at the given time with new vertices
// and edges based on samples, nodes, pa, ctm, and closed.
//
// If samples.size() == 1, this function does nothing, which makes it correctly
// handle the first subpath and consecutive M or Z commands.
//
// If pa.fill.hasColor, this function also appends a new cycle to cycles.
//
// At the end of its processing, this function updates samples and nodes to
// make them ready for the next subpath, if any.
//
// Preconditions:
//   samples.size() > 0
//   nodes.size() > 0
//   values in nodes are strictly increasing.
//
// Postconditions:
//   samples := [samples.back()]
//   nodes   := [0]
//
void finishSubpath(
        VAC* vac,
        Time time,
        EdgeSamples& samples,
        std::vector<size_t>& nodes,
        QList<Cycle>& cycles,
        const SvgPresentationAttributes& pa,
        const Transform& ctm,
        const SvgImportParams& params,
        bool closed = false)
{
    // Notations:
    //   *    sample[j]
    //  [*]   sample[j] such that there exists i with j = nodes[i]
    //  (*)   sample[j] such that there exists i with j = nodes[i],
    //        but where no vertex is to be created there either because
    //        it is the last sample of a closed subpath, or because users
    //        prefer not to split subpaths at this node type.
    //   O    closed == false
    //   C    closed == true
    //
    // Input example #1: O [*]
    // Input example #2: C (*)
    // Input example #3: O [*][*]
    // Input example #4: C [*](*)
    // Input example #5: O [*] * (*) *  * [*] * [*]
    // Input example #6: C [*] * (*) *  * [*] * (*)
    // Input example #7: C (*) * (*) *  * [*] * (*)
    // Input example #8: C (*) * (*) *  * (*) * (*)

    // Nothing to do if this is an empty subpath (examples #1 and #4)
    if (samples.size() == 0) {
        qFatal("Empty samples in SvgParser.cpp:finishSubpath()");
        return;
    }
    if (samples.size() == 1) {
        return;
    }

    // Implicit LineTo command
    if (closed && samples.back().distanceTo(samples.front()) > 1e-6) {
        // Important: we need to create a copy of endpoints, since
        // elements are added to samples which may invalidate references.
        EdgeSample p = samples.back();
        EdgeSample q = samples.front();
        addLineSamples(samples, p, q);
        nodes.push_back(samples.size() - 1);
    }

    // Have open subpaths behave as closed subpaths if pa.fill.hasColor == true
    // and if the subpath is geometrically closed. In theory, we may want to
    // add a zero-width, zero-length open edge in this case, but in practice:
    //
    // - This results in artifacts due to our current implementation always
    //   resampling zero-length edges into non-zero-length edges.
    //
    // - The author most likely meant to actually represent a closed subpath,
    //   and either forgot the "Z", or the editor/minimizer removed it.
    //
    if (!closed && pa.fill.hasColor && samples.back().distanceTo(samples.front()) < 1e-6) {
        closed = true;
    }

    // Remember last sample (will be the first sample of next subpath)
    EdgeSample lastSample = samples.back();

    // Remove the last sample and the last node if closed == true, then detect
    // and remove the nodes where we don't want to split based on user
    // preferences (for closed subpaths, this may include the first node).
    //
    // Output:
    // #3: O [*][*]
    // #4: C [*]                           // e.g.: "M 0 0 L 0 0 Z"
    // #5: O [*] *  *  *  * [*] * [*]
    // #6: C [*] *  *  *  * [*] *
    // #7: C  *  *  *  *  * [*] *
    // #8: C  *  *  *  *  *  *  *
    //
    if (closed) {
        samples.pop_back();
        nodes.pop_back();
    }
    if (params.vertexMode == SvgImportVertexMode::Endpoints) {
        if (closed) nodes = {};
        else        nodes = {0, samples.size() - 1};
    }

    // Apply transform
    for (size_t j = 0; j < samples.size(); ++j) {
        samples[j] = applyTransform(ctm, samples[j]);
    }

    // Create vertices
    std::vector<KeyVertex*> vertices;
    vertices.reserve(nodes.size());
    for (size_t j : nodes) {
        vertices.push_back(vac->newKeyVertex(time, samples[j]));
    }

    // Create edges
    //
    // #3: O [*][*]                    => 2 vertices, 1 open edge
    // #4: C [*]                       => 1 vertex,   1 open edge
    // #5: O [*] *  *  *  * [*] * [*]  => 3 vertices, 2 open edges
    // #6: C [*] *  *  *  * [*] *      => 2 vertices, 2 open edges
    // #7: C  *  *  *  *  * [*] *      => 1 vertex,   1 open edge
    // #8: C  *  *  *  *  *  *  *      => 0 vertices, 1 closed edge
    //
    QList<KeyHalfedge> halfedges;
    EdgeSamples edgeSamples;
    if (nodes.empty()) {
        // Create closed edge
        samples.push_back(samples.front());
        LinearSpline* geometry = new LinearSpline(samples, true);
        KeyEdge* edge = vac->newKeyEdge(time, geometry);
        edge->setColor(pa.stroke.color);
        halfedges.push_back(KeyHalfedge(edge, true));
    }
    else {
        // Create open edges
        size_t numSamples = samples.size();
        size_t numVertices = vertices.size(); // == nodes.size()
        size_t numEdges = closed ? numVertices : numVertices - 1;
        for (size_t i = 0; i < numEdges; ++i) {
            size_t i1 = i;
            size_t i2 = (i+1) % numVertices;
            KeyVertex* v1 = vertices[i1];
            KeyVertex* v2 = vertices[i2];
            size_t j1 = nodes[i1];
            size_t j2 = nodes[i2];
            if (j2 <= j1) {
                // #6: when i == 1, we initially have j2 < j1
                // #7: when i == 0, we initially have j2 == j1
                j2 += numSamples;
            }
            edgeSamples.clear();
            edgeSamples.reserve(j2 - j1 + 1);
            for (size_t j = j1; j <= j2; ++j) {
                edgeSamples.push_back(samples[j % numSamples]);
            }
            LinearSpline* geometry = new LinearSpline(edgeSamples);
            KeyEdge* edge = vac->newKeyEdge(time, v1, v2, geometry);
            edge->setColor(pa.stroke.color);
            halfedges.push_back(KeyHalfedge(edge, true));
        }
    }

    // Append cycle if pa.fill.hasColor == true
    if (pa.fill.hasColor) {
        // Create zero-width straight open edge if not already closed.
        if (!halfedges.front().isClosed()) {
            KeyVertex* v1 = halfedges.back().endVertex();
            KeyVertex* v2 = halfedges.front().startVertex();
            if (v1 != v2) {
                KeyEdge* edge = vac->newKeyEdge(time, v1, v2);
                edge->setColor(pa.stroke.color);
                halfedges.push_back(KeyHalfedge(edge, true));
            }
        }
        // Append cycle
        cycles.push_back(Cycle(halfedges));
    }

    // Prepare samples and nodes for next subpath (if any)
    samples.clear();
    nodes.clear();
    samples.push_back(lastSample);
    nodes.push_back(0);
}

// Returns the angle between two vectors
//
double angle(const Eigen::Vector2d& a, const Eigen::Vector2d& b)
{
    // Note: Eigen doesn't have "2D cross product" yet :-(
    // https://eigen.tuxfamily.org/bz/show_bug.cgi?id=1037
    double dot = a.dot(b);               // = a[0]*b[0] + a[1]*b[1];
    double det = a[0]*b[1] - a[1]*b[0];  // = a."cross2"(b) = "Matrix2d(a, b)".determinant()
    return atan2(det, dot);
}

// Creates new vertices, edges, and faces from the given path data commands.
//
void importPathData(
        const std::vector<SvgPathCommand>& cmds, VAC* vac, Time time,
        SvgPresentationAttributes &pa, const Transform& ctm,
        const SvgImportParams& params)
{
    // Edge width, in local coordinates
    double width = pa.strokeWidth;

    // Previous subpaths (or empty list if no face is to be created)
    QList<Cycle> cycles;

    // Previous samples of current subpath.
    // Invariant: samples.size() > 0:
    // - samples.back() represents the current position
    // - samples is initialized as [(0, 0, w)] so that the first MoveTo is
    //   always interpreted as absolute, even 'm' is used, as per spec.
    // - if samples.size() == 1 at the end of a subpath, we simply ignore it,
    //   since this means it was either the first MoveTo, or that there were
    //   no draw commands (e.g., M directly followed by Z or by another M)
    EdgeSamples samples;
    samples.reserve(cmds.size());
    samples.push_back(EdgeSample(0.0, 0.0, width));

    // Location of path nodes in the samples, that is, the boundaries
    // between path segments.
    std::vector<size_t> nodes;
    nodes.reserve(cmds.size());
    nodes.push_back(0);

    // Previous command and last Bezier control point. This is used
    // for the "smooth" bezier curveto variants, that is, S and T.
    SvgPathCommandType previousCommandType = SvgPathCommandType::MoveTo;
    Eigen::Vector2d lastControlPoint(0.0, 0.0);

    // Argument tuple of current command segment
    std::vector<double> args;
    args.reserve(7);

    // Iterate over all commands
    for (const SvgPathCommand& cmd : cmds) {
        size_t nargs = cmd.args.size();
        size_t arity = signature(cmd.type).size();
        size_t nargtuples = (arity == 0) ? 1 : nargs/arity;
        for (size_t k = 0; k < nargtuples; ++k) {
            args.clear();
            for (size_t i = 0; i < arity; ++i) {
                args.push_back(cmd.args[k * arity + i]);
            }

            // Start and end subpaths. Note: as per spec, if a MoveTo is
            // followed by multiple pairs of coordinates, the subsequent pairs
            // are treated as implicit LineTo commands.
            if ( cmd.type == SvgPathCommandType::ClosePath ||
                (cmd.type == SvgPathCommandType::MoveTo && k == 0)) {
                bool close = (cmd.type == SvgPathCommandType::ClosePath);
                finishSubpath(vac, time, samples, nodes, cycles, pa, ctm, params, close);
                if (cmd.type == SvgPathCommandType::MoveTo) {
                    if (cmd.relative) {
                        samples[0].translate(args[0], args[1]);
                    }
                    else {
                        samples[0].setPos(args[0], args[1]);
                    }
                }
            }

            // Add lines
            else if (cmd.type == SvgPathCommandType::MoveTo || // k > 0 => LineTo
                     cmd.type == SvgPathCommandType::LineTo ||
                     cmd.type == SvgPathCommandType::HLineTo ||
                     cmd.type == SvgPathCommandType::VLineTo) {
                EdgeSample p = samples.back();
                EdgeSample q = p;
                if (cmd.relative) {
                    if (cmd.type == SvgPathCommandType::HLineTo)      q.translate(args[0], 0.0);
                    else if (cmd.type == SvgPathCommandType::VLineTo) q.translate(0.0, args[0]);
                    else /* LineTo, possibly implicit via MoveTo */   q.translate(args[0], args[1]);
                }
                else {
                    if (cmd.type == SvgPathCommandType::HLineTo)      q.setX(args[0]);
                    else if (cmd.type == SvgPathCommandType::VLineTo) q.setY(args[0]);
                    else /* LineTo, possibly implicit via MoveTo */   q.setPos(args[0], args[1]);
                }
                addLineSamples(samples, p, q);
                nodes.push_back(samples.size() - 1);
            }

            // Add cubic Bezier segments
            else if (cmd.type == SvgPathCommandType::CCurveTo ||
                     cmd.type == SvgPathCommandType::SCurveTo) {
                Eigen::Vector2d p = samples.back().pos();
                Eigen::Vector2d q, r, s;
                if (cmd.type == SvgPathCommandType::CCurveTo) {
                    q = Eigen::Vector2d(args[0], args[1]);
                    r = Eigen::Vector2d(args[2], args[3]);
                    s = Eigen::Vector2d(args[4], args[5]);
                }
                else {
                    if (previousCommandType == SvgPathCommandType::CCurveTo ||
                        previousCommandType == SvgPathCommandType::SCurveTo) {
                        q = 2 * p - lastControlPoint;
                    }
                    else {
                        q = p;
                    }
                    if (cmd.relative) {
                        q -= p;
                    }
                    r = Eigen::Vector2d(args[0], args[1]);
                    s = Eigen::Vector2d(args[2], args[3]);
                }
                if (cmd.relative) {
                    q += p;
                    r += p;
                    s += p;
                }
                lastControlPoint = r;
                // Add 8 samples. Will be resampled anyway later.
                int nsamples = 8;
                double du = 1.0 / static_cast<double>(nsamples);
                for (int j = 1; j <= nsamples; ++j) {
                    double u = j * du;
                    Eigen::Vector2d b =     (1-u) * (1-u) * (1-u) * p +
                                        3 * (1-u) * (1-u) *   u   * q +
                                        3 * (1-u) *   u   *   u   * r +
                                              u   *   u   *   u   * s;
                    samples.push_back(EdgeSample(b[0], b[1], width));
                }
                nodes.push_back(samples.size() - 1);
            }

            // Add quadratic Bezier segments
            else if (cmd.type == SvgPathCommandType::QCurveTo ||
                     cmd.type == SvgPathCommandType::TCurveTo) {
                Eigen::Vector2d p = samples.back().pos();
                Eigen::Vector2d q, r;
                if (cmd.type == SvgPathCommandType::QCurveTo) {
                    q = Eigen::Vector2d(args[0], args[1]);
                    r = Eigen::Vector2d(args[2], args[3]);
                }
                else {
                    if (previousCommandType == SvgPathCommandType::QCurveTo ||
                        previousCommandType == SvgPathCommandType::TCurveTo) {
                        q = 2 * p - lastControlPoint;
                    }
                    else {
                        q = p;
                    }
                    if (cmd.relative) {
                        q -= p;
                    }
                    r = Eigen::Vector2d(args[0], args[1]);
                }
                if (cmd.relative) {
                    q += p;
                    r += p;
                }
                lastControlPoint = q;
                // Add 8 samples. Will be resampled anyway later.
                int nsamples = 8;
                double du = 1.0 / static_cast<double>(nsamples);
                for (int j = 1; j <= nsamples; ++j) {
                    double u = j * du;
                    Eigen::Vector2d b =     (1-u) * (1-u) * p +
                                        2 * (1-u) *   u   * q +
                                              u   *   u   * r;
                    samples.push_back(EdgeSample(b[0], b[1], width));
                }
                nodes.push_back(samples.size() - 1);
            }

            // Add elliptical arcs
            // See https://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
            else {
                const double eps = 1e-6;
                double rx = std::abs(args[0]);
                double ry = std::abs(args[1]);
                double phi = args[2] / 180.0 * M_PI;
                bool fa = (args[3] > 0.5);
                bool fs = (args[4] > 0.5);
                Eigen::Vector2d p = samples.back().pos();
                Eigen::Vector2d q(args[5], args[6]);
                if (cmd.relative) {
                    q += p;
                }
                if (rx < eps || ry < eps) {
                    EdgeSample p_ = samples.back();
                    EdgeSample q_(q[0], q[1], width);
                    addLineSamples(samples, p_, q_);
                }
                else {
                    // Correction of out-of-range radii
                    double cosphi = std::cos(phi);
                    double sinphi = std::sin(phi);
                    double rx2 = rx*rx;
                    double ry2 = ry*ry;
                    Eigen::Matrix2d rot; rot << cosphi, -sinphi, sinphi, cosphi;
                    Eigen::Matrix2d rotInv; rotInv << cosphi, sinphi, -sinphi, cosphi;
                    Eigen::Vector2d p_ = rotInv * (0.5 * (p - q));
                    double px_2 = p_[0]*p_[0];
                    double py_2 = p_[1]*p_[1];
                    double D = px_2/rx2 + py_2/ry2;
                    if (D > 1) {
                        double d = std::sqrt(D);
                        rx *= d; ry *= d; rx2 = rx*rx; ry2 = ry*ry;
                    }
                    // Conversion from endpoint to center parameterization.
                    double rx2py_2 = rx2*py_2;
                    double ry2px_2 = ry2*px_2;
                    double A = (rx2*ry2 - rx2py_2 - ry2px_2) / (rx2py_2 + ry2px_2);
                    double a = std::sqrt(std::abs(A));
                    if (fa == fs) {
                        a *= -1;
                    }
                    Eigen::Vector2d c_(a*p_[1]*rx/ry, -a*p_[0]*ry/rx);
                    Eigen::Vector2d c = rot * c_ + 0.5 * (p + q);
                    Eigen::Vector2d rInv(1/rx, 1/ry);
                    Eigen::Vector2d e1(1, 0);
                    Eigen::Vector2d e2 = rInv.cwiseProduct(  p_ - c_);
                    Eigen::Vector2d e3 = rInv.cwiseProduct(- p_ - c_);
                    double theta1 = angle(e1, e2);
                    double Dtheta = angle(e2, e3);
                    if (fs == false && Dtheta > 0) {
                        Dtheta -= 2 * M_PI;
                    }
                    else if (fs == true && Dtheta < 0) {
                        Dtheta += 2 * M_PI;
                    }
                    // Add 12 samples per quarter-circle.
                    int nsamples = 1 + static_cast<int>(std::floor(24 * std::abs(Dtheta) / M_PI));
                    double dtheta = Dtheta / static_cast<double>(nsamples);
                    for (int j = 1; j <= nsamples; ++j) {
                        double theta = theta1 + j * dtheta;
                        Eigen::Vector2d b(rx * std::cos(theta), ry * std::sin(theta));
                        b = c + rot * b;
                        samples.push_back(EdgeSample(b[0], b[1], width));
                    }
                }
                nodes.push_back(samples.size() - 1);
            }
            previousCommandType = cmd.type;
        }
    }
    finishSubpath(vac, time, samples, nodes, cycles, pa, ctm, params);

    // Create face from cycles
    if (!cycles.empty()) {
        KeyFace* face = vac->newKeyFace(cycles);
        face->setColor(pa.fill.color);
    }
}

// Parses color from string, will probably be moved to a class like CSSColor
// This implements the most of the W3 specifications
// found at https://www.w3.org/TR/SVG11/types.html#DataTypeColor
// It also extends the specifications in a few minor ways
// This includes more flexible whitespace and some CSS3 features (hsl)
QColor parseColor(QString s)
{
    // Remove excess whitespace
    s = s.trimmed();
    if(s.startsWith("rgba") && s.endsWith(")") && s.contains("("))
    {
        // Remove rgba()
        s.remove(0, 4).chop(1);
        s = s.trimmed().remove(0, 1);

        // Split into elements with comma separating them
        QStringList sSplit = s.split(',');

        // If it doesn't have exactly four elements, return an invalid color
        if(sSplit.size() != 4)
        {
            return QColor();
        }

        int colors[3];

        // Handle rgb channels
        for(int i = 0; i < 3; i++)
        {
            QString element(sSplit[i]);

            // More trimming
            element = element.trimmed();

            // Determine if it is *% or * format
            if(element.endsWith("%"))
            {
                // Remove % sign
                element.chop(1);

                colors[i] = qRound(qBound(0.0, element.toDouble(), 100.0) * 2.55);
            }
            else
            {
                colors[i] = qBound(0, qRound(element.toDouble()), 255);
            }
        }
        // Alpha channel is a double from 0.0 to 1.0 inclusive
        double alpha = qBound(0.0, sSplit[3].toDouble(), 1.0);

        // Return result
        QColor color = QColor(colors[0], colors[1], colors[2]);
        color.setAlphaF(alpha);
        return color;
    }
    else if(s.startsWith("rgb") && s.endsWith(")") && s.contains("("))
    {
        // Remove rgb()
        s.remove(0, 3).chop(1);
        s = s.trimmed().remove(0, 1);

        // Split into elements with comma separating them
        QStringList sSplit = s.split(',');

        // If it doesn't have exactly three elements, return an invalid color
        if(sSplit.size() != 3)
        {
            return QColor();
        }

        int colors[3];

        for(int i = 0; i < 3; i++)
        {
            QString element(sSplit[i]);

            // More trimming
            s = s.trimmed();

            // Determine if it is *% or * format
            if(element.endsWith("%"))
            {
                // Remove % sign
                element.chop(1);

                // Convert to number and add element to list (100% -> 255)
                colors[i] = qRound(qBound(0.0, element.toDouble(), 100.0) * 2.55);
            }
            else
            {
                colors[i] = qBound(0, qRound(element.toDouble()), 255);
            }
        }

        // Return result
        return QColor(colors[0], colors[1], colors[2]);
    }
    else if(s.startsWith("hsla") && s.endsWith(")") && s.contains("("))
    {
        // Remove hsla()
        s.remove(0, 4).chop(1);
        s = s.trimmed().remove(0, 1);

        // Split into elements with comma separating them
        QStringList sSplit = s.split(',');

        // If it doesn't have exactly three elements, return an invalid color
        // If saturation and lightness are not percentages, also return invalid
        if(sSplit.size() != 4 || !sSplit[1].endsWith("%") || !sSplit[2].endsWith("%"))
        {
            return QColor();
        }

        // Hue is an angle from 0-359 inclusive
        int hue = qRound(sSplit[0].toDouble());
        // As an angle, hue loops around
        hue = ((hue % 360) + 360) % 360;

        // Saturation and lightness are read as percentages and mapped to the range 0-255
        // Remove percentage signs
        sSplit[1].chop(1);
        sSplit[2].chop(1);
        int saturation = qRound(qBound(0.0, sSplit[1].toDouble(), 100.0) * 2.55);
        int lightness = qRound(qBound(0.0, sSplit[2].toDouble(), 100.0) * 2.55);

        // Alpha channel is a double from 0.0 to 1.0 inclusive
        double alpha = qBound(0.0, sSplit[3].toDouble(), 1.0);

        // Return result
        QColor color = QColor();
        color.setHsl(hue, saturation, lightness);
        color.setAlphaF(alpha);
        return color;
    }
    else if(s.startsWith("hsl") && s.endsWith(")") && s.contains("("))
    {
        // Remove hsl()
        s.remove(0, 3).chop(1);
        s = s.trimmed().remove(0, 1);

        // Split into elements with comma separating them
        QStringList sSplit = s.split(',');

        // If it doesn't have exactly three elements, return an invalid color
        // If saturation and lightness are not percentages, also return invalid
        if(sSplit.size() != 3 || !sSplit[1].endsWith("%") || !sSplit[2].endsWith("%"))
        {
            return QColor();
        }

        // Hue is an angle from 0-359 inclusive
        int hue = qRound(sSplit[0].toDouble());
        // As an angle, hue loops around
        hue = ((hue % 360) + 360) % 360;

        // Saturation and lightness are read as percentages and mapped to the range 0-255
        // Remove percentage signs
        sSplit[1].chop(1);
        sSplit[2].chop(1);
        int saturation = qRound(qBound(0.0, sSplit[1].toDouble(), 100.0) * 2.55);
        int lightness = qRound(qBound(0.0, sSplit[2].toDouble(), 100.0) * 2.55);

        // Return result
        QColor color = QColor();
        color.setHsl(hue, saturation, lightness);
        return color;
    }
    else
    {
        // This handles named constants and #* formats
        return QColor(s);
    }
}

// https://www.w3.org/TR/SVG11/painting.html#SpecifyingPaint
SvgPaint parsePaint(QString s)
{
    // Remove excess whitespace
    s = s.trimmed();
    if(s == "none") {
        return SvgPaint();
    }
    else {
        QColor color = parseColor(s);
        if (color.isValid()) {
            return SvgPaint(color);
        }
        else {
            return SvgPaint();
        }
    }
}

bool readPath(const QXmlStreamAttributes& attrs, VAC* vac, Time t,
              SvgPresentationAttributes &pa, const Transform& ctm,
              const SvgImportParams& params)
{
    // Don't render if no path data provided
    if(!attrs.hasAttribute("d")) return true;

    // Parse path data.
    // TODO: Show errors to users as a message box rather than printing to console.
    std::string error;
    QString d = attrs.value("d").toString();
    std::vector<SvgPathCommand> cmds = parsePathData(d.toStdString(), &error);
    if (!error.empty()) {
        qDebug() << "ERROR:" << QString::fromStdString(error);
    }

    // Import path data (up to, but not including, first invalid command)
    importPathData(cmds, vac, t, pa, ctm, params);
    return error.empty();
}

// Reads a <rect> object
// https://www.w3.org/TR/SVG11/shapes.html#RectElement
// @return true on success, false on failure
bool readRect(const QXmlStreamAttributes& attrs, VAC* vac, Time t,
              SvgPresentationAttributes &pa, const Transform& ctm,
              const SvgImportParams& params)
{
    bool okay = true;

    // X position
    double x = attrs.hasAttribute("x") ? attrs.value("x").toDouble(&okay) : 0;
    if(!okay) x = 0;

    // Y position
    double y = attrs.hasAttribute("y") ? attrs.value("y").toDouble(&okay) : 0;
    if(!okay) y = 0;

    // Width
    double width = attrs.value("width").toDouble(&okay);
    // Error, width isn't a real number
    if(!okay) return false;

    // Height
    double height = attrs.value("height").toDouble(&okay);
    // Error, height isn't a real number
    if(!okay) return false;

    // Negative width or height results in an error
    if(width < 0 || height < 0) return false;

    // A width or height of 0 does not result in an error, but disables rendering of the object
    if(width == 0 || height == 0) return true;

    // The rx and ry attributes have a slightly more advanced default value, see W3 specifications for details
    double rx, ry;
    bool rxOkay = false, ryOkay = false;
    if(attrs.hasAttribute("rx"))
    {
        rx = attrs.value("rx").toDouble(&rxOkay);
    }
    if(attrs.hasAttribute("ry"))
    {
        ry = attrs.value("ry").toDouble(&ryOkay);
    }
    if(!rxOkay && !ryOkay)
    {
        rx = ry = 0;
    }
    else if(rxOkay && !ryOkay)
    {
        ry = rx;
    }
    else if(!rxOkay && ryOkay)
    {
        rx = ry;
    }
    rx = qBound(0.0, rx, width / 2);
    ry = qBound(0.0, ry, height / 2);

    // Create equivalent path and import
    std::vector<SvgPathCommand> cmds;
    if (rx > 0 && ry > 0) {
        // Rounded rect
        cmds = {
            {SvgPathCommandType::MoveTo,    false, {x+rx, y}},
            {SvgPathCommandType::HLineTo,   false, {x+width-rx}},
            {SvgPathCommandType::ArcTo,     false, {rx, ry, 0, 0, 1, x+width, y+ry}},
            {SvgPathCommandType::VLineTo,   false, {y+height-ry}},
            {SvgPathCommandType::ArcTo,     false, {rx, ry, 0, 0, 1, x+width-rx, y+height}},
            {SvgPathCommandType::HLineTo,   false, {x+rx}},
            {SvgPathCommandType::ArcTo,     false, {rx, ry, 0, 0, 1, x, y+height-ry}},
            {SvgPathCommandType::VLineTo,   false, {y+ry}},
            {SvgPathCommandType::ArcTo,     false, {rx, ry, 0, 0, 1, x+rx, y}},
            {SvgPathCommandType::ClosePath, false, {}}
        };
    }
    else {
        // Sharp rect
        cmds = {
            {SvgPathCommandType::MoveTo,    false, {x, y}},
            {SvgPathCommandType::HLineTo,   false, {x+width}},
            {SvgPathCommandType::VLineTo,   false, {y+height}},
            {SvgPathCommandType::HLineTo,   false, {x}},
            {SvgPathCommandType::ClosePath, false, {}}
        };
    }
    importPathData(cmds, vac, t, pa, ctm, params);
    return true;
}

bool readCircle(const QXmlStreamAttributes& attrs, VAC* vac, Time t,
                SvgPresentationAttributes &pa, const Transform& ctm,
                const SvgImportParams& params)
{
    bool okay = true;

    // Center X position
    double cx = attrs.hasAttribute("cx") ? attrs.value("cx").toDouble(&okay) : 0;
    if(!okay) cx = 0;

    // Center Y position
    double cy = attrs.hasAttribute("cy") ? attrs.value("cy").toDouble(&okay) : 0;
    if(!okay) cy = 0;

    // Radius
    double r = attrs.value("r").toDouble(&okay);
    // Error, radius isn't a real number
    if(!okay) return false;

    // Negative radius results in an error
    if(r < 0) return false;
    // A radius of 0 does not result in an error, but disables rendering of the object
    if(r == 0) return true;

    // Create equivalent path and import
    // Note: as per 2019-12-10, the SVG 2 draft specifies that we should
    // use sweep-flag=0. I believe this is an error in the draft and
    // that we should use sweep-flag=1 instead, like for rounded rects.
    // The code below uses sweep-flag=1.
    // See: https://github.com/w3c/svgwg/issues/765
    std::vector<SvgPathCommand> cmds = {
        {SvgPathCommandType::MoveTo,    false, {cx+r, cy}},
        {SvgPathCommandType::ArcTo,     false, {r, r, 0, 0, 1, cx, cy+r}},
        {SvgPathCommandType::ArcTo,     false, {r, r, 0, 0, 1, cx-r, cy}},
        {SvgPathCommandType::ArcTo,     false, {r, r, 0, 0, 1, cx, cy-r}},
        {SvgPathCommandType::ArcTo,     false, {r, r, 0, 0, 1, cx+r, cy}},
        {SvgPathCommandType::ClosePath, false, {}}
    };
    importPathData(cmds, vac, t, pa, ctm, params);
    return true;
}

bool readEllipse(const QXmlStreamAttributes& attrs, VAC* vac, Time t,
                 SvgPresentationAttributes &pa, const Transform& ctm,
                 const SvgImportParams& params)
{
    bool okay = true;

    // Center X position
    double cx = attrs.hasAttribute("cx") ? attrs.value("cx").toDouble(&okay) : 0;
    if(!okay) cx = 0;

    // Center Y position
    double cy = attrs.hasAttribute("cy") ? attrs.value("cy").toDouble(&okay) : 0;
    if(!okay) cy = 0;

    // X radius
    double rx = attrs.value("rx").toDouble(&okay);
    // Error, x radius isn't a real number
    if(!okay) return false;

    // Y radius
    double ry = attrs.value("ry").toDouble(&okay);
    // Error, y radius isn't a real number
    if(!okay) return false;

    // Negative x or y radius results in an error
    if(rx < 0 || ry < 0) return false;
    // A x or y radius of 0 does not result in an error, but disables rendering of the object
    if(rx == 0 || ry == 0) return true;

    // Create equivalent path and import
    // Note: as per 2019-12-10, the SVG 2 draft specifies that we should
    // use sweep-flag=0. I believe this is an error in the draft and
    // that we should use sweep-flag=1 instead, like for rounded rects.
    // The code below uses sweep-flag=1.
    // See: https://github.com/w3c/svgwg/issues/765
    std::vector<SvgPathCommand> cmds = {
        {SvgPathCommandType::MoveTo,    false, {cx+rx, cy}},
        {SvgPathCommandType::ArcTo,     false, {rx, ry, 0, 0, 1, cx, cy+ry}},
        {SvgPathCommandType::ArcTo,     false, {rx, ry, 0, 0, 1, cx-rx, cy}},
        {SvgPathCommandType::ArcTo,     false, {rx, ry, 0, 0, 1, cx, cy-ry}},
        {SvgPathCommandType::ArcTo,     false, {rx, ry, 0, 0, 1, cx+rx, cy}},
        {SvgPathCommandType::ClosePath, false, {}}
    };
    importPathData(cmds, vac, t, pa, ctm, params);
    return true;
}

bool readLine(const QXmlStreamAttributes& attrs, VAC* vac, Time t,
              SvgPresentationAttributes &pa, const Transform& ctm,
              const SvgImportParams& params)
{
    bool okay = true;

    // X position 1
    double x1 = attrs.hasAttribute("x1") ? attrs.value("x1").toDouble(&okay) : 0;
    if(!okay) x1 = 0;

    // Y position 1
    double y1 = attrs.hasAttribute("y1") ? attrs.value("y1").toDouble(&okay) : 0;
    if(!okay) y1 = 0;

    // X position 2
    double x2 = attrs.hasAttribute("x2") ? attrs.value("x2").toDouble(&okay) : 0;
    if(!okay) x2 = 0;

    // Y position 2
    double y2 = attrs.hasAttribute("y2") ? attrs.value("y2").toDouble(&okay) : 0;
    if(!okay) y2 = 0;

    // Create equivalent path and import
    std::vector<SvgPathCommand> cmds = {
        {SvgPathCommandType::MoveTo, false, {x1, y1}},
        {SvgPathCommandType::LineTo, false, {x2, y2}}
    };
    importPathData(cmds, vac, t, pa, ctm, params);
    return true;
}

bool readPolylineOrPolygon(
        const QXmlStreamAttributes& attrs, VAC* vac, Time t,
        SvgPresentationAttributes &pa, const Transform& ctm,
        const SvgImportParams& params,
        bool isPolygon)
{
    // Don't render if no points provided
    if(!attrs.hasAttribute("points")) {
        return true;
    }

    // Parse points attribute.
    //
    // TODO Make this spec-compliant. For example, "100-200" is valid as per
    // SVG 1.1, but the code below doesn't parse it properly. See:
    // https://github.com/w3c/svgwg/issues/763
    //
    // Note: If we fail to read a number, then we still render the points up to
    // the last non-erroneous point. It's not clear from the spec whether this
    // is the recommended approach (vs. not rendering anything at all), but our
    // choice is consistent with path data error handling. See:
    // https://github.com/w3c/svgwg/issues/764
    //
    QStringList coords = attrs.value("points").toString().split(QRegExp("[\\s,]+"), QString::SkipEmptyParts);
    size_t numCoords = static_cast<size_t>(coords.size());
    std::vector<double> d;
    d.reserve(numCoords);
    for (const QString& s: coords) {
        bool ok;
        double x = s.toDouble(&ok);
        if (ok) {
            d.push_back(x);
        }
        else {
            break;
        }
    }

    // Create equivalent path and import. As per spec:
    //
    // - If the number of coords is odd, it is an error
    //   but we still render, ignoring the last coord.
    //
    // - If there are no points, it is a valid element
    //   but there's nothing to render.
    //
    size_t numPoints = d.size() / 2;
    if (numPoints > 0) {
        std::vector<SvgPathCommand> cmds;
        cmds.reserve(static_cast<size_t>(numPoints));
        cmds.push_back({SvgPathCommandType::MoveTo, false, {d[0], d[1]}});
        for (size_t i = 1; i < numPoints; ++i) {
            cmds.push_back({SvgPathCommandType::LineTo, false, {d[2*i], d[2*i+1]}});
        }
        if (isPolygon) {
            cmds.push_back({SvgPathCommandType::ClosePath, false, {}});
        }
        importPathData(cmds, vac, t, pa, ctm, params);
    }
    return 2 * numPoints == numCoords;
}

bool readPolyline(const QXmlStreamAttributes& attrs, VAC* vac, Time t,
                  SvgPresentationAttributes &pa, const Transform& ctm,
                  const SvgImportParams& params)
{
    bool isPolygon = false;
    return readPolylineOrPolygon(attrs, vac, t, pa, ctm, params, isPolygon);

}

bool readPolygon(const QXmlStreamAttributes& attrs, VAC* vac, Time t,
                 SvgPresentationAttributes &pa, const Transform& ctm,
                 const SvgImportParams& params)
{
    bool isPolygon = true;
    return readPolylineOrPolygon(attrs, vac, t, pa, ctm, params, isPolygon);
}

// Basic CSS style-attribute parsing. This is not fully compliant (e.g.,
// presence of comments, or semicolon within quoted strings), but should work
// in most cases, notably files generated by Inkscape. Note that units other
// than px (em, cm, %, etc.) are not properly supported and interpreted as user
// units.
//
QMap<QString, QString> parseStyleAttribute(const QString& style)
{
    QStringList declarations = style.split(';', QString::SkipEmptyParts);
    QMap<QString, QString> res;
    for (const QString& d : declarations) {
        QStringList namevalue = d.split(':');
        if (namevalue.size() >= 2) {
            res[namevalue[0].trimmed()] = namevalue[1].trimmed();
        }
    }
    return res;
}

} // namespace

// Read the SVG.
//
// Error Handling
// --------------
//
// In case of errors in path data or basic shapes attributes, such as if
// rect.height < 0, the SVG specification mandates to stop processing the
// document, that is, not render any other XML element that might exist after
// the error. See:
//
//   https://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing
//
//   The document shall (ed: "MUST") be rendered up to, but not including, the
//   first element which has an error. Exceptions:
//
//   - If a ‘path’ element is the first element which has an error
//     and the only errors are in the path data specification, then
//     render the ‘path’ up to the point of the path data error.
//     For related information, see ‘path’ element implementation
//     notes.
//
//   - If a ‘polyline’ or ‘polygon’ element is the first element
//     which has an error and the only errors are within the
//     ‘points’ attribute, then render the ‘polyline’ or ‘polygon’
//     up to the segment with the error.
//
//   This approach will provide a visual clue to the user or
//   developer about where the error might be in the document.
//
// However, we purposefully violate this mandated behavior, that is, we keep
// reading subsequent XML elements. Indeed, we're not a "renderer" but an
// "importer", in which case the added value of providing a visual clue matters
// less than the ability to import whatever geometry exists in the document.
// Also, this makes the importer more robust to bugs in its implementation.
//
// Besides, this is the error handling policy which we will use for VGC.
// Indeed, for VGC, we will use a different error handling policy that the one
// specified by SVG. In a VGC document, if an XML element is erroneous, then it
// should simply be ignored (or be partially rendered via a well-defined
// behavior, like SVG path data), but processing should continue for other XML
// elements, as long as it isn't an XML syntax error. This makes it much more
// robust to small bugs in user scripts or implementation which invariably
// happen, especially when approaching a deadline. When producing a movie,
// things are messy, and a broken image is much more useful than no image at
// all. Especially for geometric data, where some interpolation that overshoots
// (e.g., Catmull-Rom) might easily make height < 0 temporarily, in which case
// it is really silly not to render subsequent valid elements.
//
// Of course, we should have a proper warning system to let users be aware of
// errors, which we don't have in VPaint, but we will have in VGC.
//
void SvgParser::readSvg(XmlStreamReader & xml, const SvgImportParams& params)
{
    // Ensure that this is a SVG file
    xml.readNextStartElement();
    if(xml.name() != "svg") {
        QMessageBox::warning(global()->mainWindow(),
                             QObject::tr("Not a SVG file"),
                             QObject::tr("This file doesn't seem to be a SVG file."));
        return;
    }

    // Initialize attribute stack
    QStack<SvgPresentationAttributes> attributeStack;
    SvgPresentationAttributes defaultStyle;
    attributeStack.push(defaultStyle);

    // Initialize transform stack
    TransformStack transformStack;
    transformStack.push(Transform::Identity());

    // Get to which VAC and at which time we should import
    VAC* vac = global()->scene()->activeVAC();
    Time t = global()->activeTime();

    // Iterate over all XML tokens, including the <svg> start element
    // which may have style attributes or transforms
    while (!xml.atEnd())
    {
        // Process start elements
        if(xml.isStartElement())
        {
            // Get XML attributes
            QXmlStreamAttributes attrs = xml.attributes();

            // Apply child style to current style
            SvgPresentationAttributes pa = attributeStack.top();
            pa.applyChildStyle(attrs);
            attributeStack.push(pa);

            // Apply child transform to CTM (= Current Transform Matrix)
            Transform ctm = transformStack.top();
            if (attrs.hasAttribute("transform")) {
                std::string ts = attrs.value("transform").toString().toStdString();
                ctm = ctm * parseTransform(ts);
            }
            transformStack.push(ctm);

            // STRUCTURAL ELEMENTS: svg, g, defs, symbol, use
            //
            // https://www.w3.org/TR/SVG11/struct.html
            //
            if(xml.name() == "svg") {
                // https://www.w3.org/TR/SVG11/struct.html#NewDocument
                //
                // TODO: implement x, y, width, height, viewBox and preserveAspectRatio.
                // Note that SVG elements can be nested inside other SVG elements.
                //
                // Allowed children:
                //  structural elements
                //  struct-ish elements
                //  descriptive elements
                //  shape elements
                //  text-font elements
                //  styling elements
                //  interactivity elements
                //  animation elements
            }
            else if(xml.name() == "g") {
                // https://www.w3.org/TR/SVG11/struct.html#Groups
                // We support this. We just have to keep reading its children.
                // Allowed children: same as <svg>
            }
            else if(xml.name() == "defs") {
                // https://www.w3.org/TR/SVG11/struct.html#Head
                // This is an unrendered group where to define referenced
                // content such as symbols, markers, gradients, etc. Note that
                // many referenced content can in fact be defined anywhere in
                // the document, but defining them in defs is best practice.
                // We don't support <defs> yet, but we may want to support it later.
                // Allowed children: same as <svg>
                xml.skipCurrentElement();
            }
            else if(xml.name() == "symbol") {
                // https://www.w3.org/TR/SVG11/struct.html#SymbolElement
                // This is an unrendered group to be instanciated with <use>.
                // We don't support <symbol> yet, but we may want to support it later.
                // Allowed children: same as <svg>
                xml.skipCurrentElement();
            }
            else if(xml.name() == "use") {
                // https://www.w3.org/TR/SVG11/struct.html#UseElement
                // This is for instanciating a <symbol>.
                // We don't support <use> yet, but we may want to support it later.
                // Allowed children:
                //  descriptive elements
                //  animation elements
                xml.skipCurrentElement();
            }

            // STRUCT-ISH ELEMENTS: switch, image, foreignObject
            //
            // https://www.w3.org/TR/SVG11/struct.html
            // https://www.w3.org/TR/SVG11/backward.html
            // https://www.w3.org/TR/SVG11/extend.html
            //
            else if (xml.name() == "switch") {
                // https://www.w3.org/TR/SVG11/struct.html#ConditionalProcessing
                // https://www.w3.org/TR/SVG11/struct.html#SwitchElement
                // https://www.w3.org/TR/SVG11/backward.html
                // This is for selecting which child to process based on feature availability.
                // We don't support <switch> yet, but we may want to support it later.
                // Allowed children:
                //  subset of structural elements: svg, g, use
                //  struct-ish elements
                //  descriptive elements
                //  shape elements
                //  subset of text-font elements: text
                //  subset of interactivity elements: a
                //  animation elements
                xml.skipCurrentElement();
            }
            else if (xml.name() == "image") {
                // https://www.w3.org/TR/SVG11/struct.html#ImageElement
                // This is for rendering an external image (e.g.: jpg, png, svg).
                // We don't support <image> yet, but may want to support it later.
                // Allowed children:
                //  descriptive elements
                //  animation elements
                xml.skipCurrentElement();
            }
            else if (xml.name() == "foreignObject") {
                // https://www.w3.org/TR/SVG11/extend.html#ForeignObjectElement
                // This is for inline embedding of other XML documents which aren't
                // SVG documents, such as MathML (for mathematical expressions), or
                // XHML (useful for dynamically reflowing text).
                // We don't support <foreignObject> yet, but may want to support it later,
                // notably for XML formats which we support importing by themselves (e.g.,
                // if we add support for importing standalone XHTML documents, we may want
                // to support importing XHTML as foreignObject in SVG documents).
                // Allowed children: Any elements or character data.
                xml.skipCurrentElement();
            }

            // DESCRIPTIVE ELEMENTS: desc, title, metadata
            //
            // https://www.w3.org/TR/SVG11/struct.html#DescriptionAndTitleElements
            // https://www.w3.org/TR/SVG11/metadata.html
            //
            // Allowed children: any elements or character data.
            //
            // We ignore them and all their children as they don't affect
            // geometry or rendering in any ways, and can't be meaningfully
            // imported into VPaint.
            //
            else if (xml.name() == "desc" ||
                     xml.name() == "title" ||
                     xml.name() == "metadata") {
                xml.skipCurrentElement();
            }

            // SHAPE ELEMENTS: path, rect, circle, ellipse, line, polyline, polygon
            //
            // https://www.w3.org/TR/SVG11/paths.html
            // https://www.w3.org/TR/SVG11/shapes.html
            //
            // Allowed children:
            //  descriptive elements
            //  animation elements
            //
            else if(xml.name() == "path") {
                if(!readPath(attrs, vac, t, pa, ctm, params)) return;
            }
            else if(xml.name() == "rect") {
                if(!readRect(attrs, vac, t, pa, ctm, params)) return;
            }
            else if(xml.name() == "circle") {
                if(!readCircle(attrs, vac, t, pa, ctm, params)) return;
            }
            else if(xml.name() == "ellipse") {
                if(!readEllipse(attrs, vac, t, pa, ctm, params)) return;
            }
            else if(xml.name() == "line") {
                if(!readLine(attrs, vac, t, pa, ctm, params)) return;
            }
            else if(xml.name() == "polyline") {
                if(!readPolyline(attrs, vac, t, pa, ctm, params)) return;
            }
            else if(xml.name() == "polygon") {
                if(!readPolygon(attrs, vac, t, pa, ctm, params)) return;
            }

            // TEXT-FONT ELEMENTS: text, font, font-face, altGlyphDef
            //
            // TEXT CHILD ELEMENTS:        tspan, tref, textPath, altGlyph
            // FONT CHILD ELEMENTS:        glyph, missing-glyph, hkern, vkern, font-face
            // FONT-FACE CHILD ELEMENTS:   font-face-src, font-face-uri, font-face-format, font-face-name
            // ALTGLYPHDEF CHILD ELEMENTS: glyphRef, altGlyphItem
            //
            // https://www.w3.org/TR/SVG11/text.html
            // https://www.w3.org/TR/SVG11/fonts.html
            //
            // Note: the "child elements" types listed above only include the
            // types not already listed in other categories, and they might
            // only be allowed as direct or indirect children. See the above
            // links for details on the content model.
            //
            // We don't support text-font elements for now, but we may want to
            // support them in the future.
            //
            else if (xml.name() == "text" ||
                     xml.name() == "font" ||
                     xml.name() == "font-face" ||
                     xml.name() == "altGlyphDef") {
                xml.skipCurrentElement();
            }

            // STYLING ELEMENTS: style, marker, color-profile, linearGradient, radialGradient, pattern, clipPath, mask, filter
            //
            // GRADIENT CHILD ELEMENTS:   stop
            // LIGHT SOURCE ELEMENTS:     feDistantLight, fePointLight, feSpotLight
            // FILTER PRIMITIVE ELEMENTS: feBlend, feColorMatrix, feComponentTransfer, feComposite, feConvolveMatrix,
            //                            feDiffuseLighting, feDisplacementMap, feFlood, feGaussianBlur, feImage, feMerge,
            //                            feMorphology, feOffset, feSpecularLighting, feTile, feTurbulence
            //
            // https://www.w3.org/TR/SVG11/styling.html   style
            // https://www.w3.org/TR/SVG11/painting.html  marker
            // https://www.w3.org/TR/SVG11/color.html     color-profile
            // https://www.w3.org/TR/SVG11/pservers.html  linearGradient, radialGradient, pattern
            // https://www.w3.org/TR/SVG11/masking.html   clipPath, mask
            // https://www.w3.org/TR/SVG11/filters.html   filter
            //
            // Note: the "child elements" types listed above only include the
            // types not already listed in other categories, and they might
            // only be allowed as direct or indirect children. See the above
            // links for details on the content model.
            //
            // We don't support styling elements for now, but we may want to
            // support them in the future.
            //
            else if (xml.name() == "style" ||
                     xml.name() == "marker" ||
                     xml.name() == "color-profile" ||
                     xml.name() == "linearGradient" ||
                     xml.name() == "radialGradient" ||
                     xml.name() == "pattern" ||
                     xml.name() == "clipPath" ||
                     xml.name() == "mask" ||
                     xml.name() == "filter") {
                xml.skipCurrentElement();
            }

            // INTERACTIVITY ELEMENTS: cursor, a, view, script
            //
            // https://www.w3.org/TR/SVG11/interact.html
            // https://www.w3.org/TR/SVG11/linking.html
            // https://www.w3.org/TR/SVG11/script.html
            //
            // We ignore all of these as they make no sense in VPaint.
            // We are not planning to ever support them in the future.
            //
            else if (xml.name() == "cursor") {
                // https://www.w3.org/TR/SVG11/interact.html#CursorElement
                // This is for defining a PNG image of a cursor, e.g. to define
                // what the mouse cursor looks like when hovering some elements.
                // This is irrelevant for VPaint, so we ignore it and all its children.
                // Allowed children:
                //  descriptive elements
                xml.skipCurrentElement();
            }
            else if (xml.name() == "a") {
                // https://www.w3.org/TR/SVG11/linking.html#Links
                // This is to be redirected to another URI when clicking on
                // any graphical element containted under the <a>. We ignore
                // the clicking behavior, but we still process its children as if
                // it was a normal group <g>.
                // Allowed children: same as <svg>
            }
            else if (xml.name() == "view") {
                // https://www.w3.org/TR/SVG11/linking.html#LinksIntoSVG
                // https://www.w3.org/TR/SVG11/linking.html#ViewElement
                // This is to predefine a specific viewBox or viewTarget within
                // this SVG document, that other documents can link to, for
                // example via "MyDrawing.svg#MyView", similar to the usage
                // of id-based hashtags in HTML URLs.
                // This is irrelevant for VPaint, so we ignore it and all its children.
                // Allowed children:
                //  descriptive elements
                xml.skipCurrentElement();
            }
            else if (xml.name() == "script") {
                // https://www.w3.org/TR/SVG11/script.html#ScriptElement
                // This is for running scripts, or defining script functions to
                // be run when interacting with SVG content (clicking, hovering, etc.)
                // This is irrelevant for VPaint, so we ignore it and all its children.
                // Allowed children: Any elements or character data.
                xml.skipCurrentElement();
            }

            // ANIMATION ELEMENTS: animate, set, animateMotion, animateColor, animateTransform
            //
            // https://www.w3.org/TR/SVG11/animate.html
            //
            // Allowed children:
            //  descriptive elements
            //  mpath (only for animationMotion, and at most one)
            //
            // We don't support animation elements for now. VPaint being an
            // animation tool, we obviously may want to support them in the
            // future.
            //
            else if (xml.name() == "animate" ||
                     xml.name() == "set" ||
                     xml.name() == "animateMotion" ||
                     xml.name() == "animateColor" ||
                     xml.name() == "animateTransform") {
                xml.skipCurrentElement();
            }

            // Unknown elements. These aren't part of SVG 1.1, such as
            // Inkscape's "sodipodi:namedview".
            else {
                xml.skipCurrentElement();
            }
        }

        // Process end elements.
        //
        // Note that we don't use "else if" since the current TokenType changes
        // from StartElement to EndElement when calling skipCurrentElement().
        //
        if(xml.isEndElement()) {
            attributeStack.pop();
            transformStack.pop();
        }

        xml.readNext();
    }
}

SvgPresentationAttributes::SvgPresentationAttributes() :
    fill_(Qt::black), // = {true, black}
    stroke_(),        // = {false, black}
    fillOpacity_(1.0),
    strokeOpacity_(1.0),
    strokeWidth_(1.0),
    opacity_(1.0)
{
    update_();
}

void SvgPresentationAttributes::applyChildStyle(const QXmlStreamAttributes& attrs)
{
    bool ok;
    double number;

    // Style attribute. Note: styling defined via the 'style' attribute
    // takes precedence over styling defined via presentation attributes.
    QMap<QString, QString> style;
    if(attrs.hasAttribute("style")) {
        style = parseStyleAttribute(attrs.value("style").toString());
    }

    // Stroke width
    if (style.contains("stroke-width") && readNumber(style["stroke-width"], &number)) {
        strokeWidth_ = qMax(0.0, number);
    }
    else if (attrs.hasAttribute("stroke-width") && readNumber(attrs.value("stroke-width").toString(), &number)) {
        strokeWidth_ = qMax(0.0, number);
    }

    // Fill (color)
    if(style.contains("fill")) {
        fill_ = parsePaint(style["fill"]);
    }
    else if(attrs.hasAttribute("fill")) {
        fill_ = parsePaint(attrs.value("fill").toString());
    }

    // Stroke (color)
    if(style.contains("stroke")) {
        stroke_ = parsePaint(style["stroke"]);
    }
    else if(attrs.hasAttribute("stroke")) {
        stroke_ = parsePaint(attrs.value("stroke").toString());
    }

    // Fill opacity
    if (style.contains("fill-opacity") && readNumber(style["fill-opacity"], &number)) {
        fillOpacity_ = qBound(0.0, number, 1.0);
    }
    else if (attrs.hasAttribute("fill-opacity") && readNumber(attrs.value("fill-opacity").toString(), &number)) {
        fillOpacity_ = qBound(0.0, number, 1.0);
    }

    // Stroke opacity
    if (style.contains("stroke-opacity") && readNumber(style["stroke-opacity"], &number)) {
        strokeOpacity_ = qBound(0.0, number, 1.0);
    }
    else if (attrs.hasAttribute("stroke-opacity") && readNumber(attrs.value("stroke-opacity").toString(), &number)) {
        strokeOpacity_ = qBound(0.0, number, 1.0);
    }

    // Group or Element Opacity
    //
    // Note that unlike other style attributes (including `fill-opacity` and
    // `stroke-opacity`), the `opacity` attribute is not "inherited" by
    // children. Instead, children of a group are supposed to be rendered in an
    // offscreen buffer, then the buffer should be composited with the
    // background based on its opacity.
    //
    // Example 1:
    //
    // <g opacity="0.5">                            // => opacity = 0.5  fill-opacity = 1.0
    //   <circle cx="0" cy="0" r="10" fill="red">   // => opacity = 1.0  fill-opacity = 1.0
    //   <circle cx="0" cy="0" r="10" fill="green"> // => opacity = 1.0  fill-opacity = 1.0
    // </g>
    //
    // A fully-opaque green circle is drawn over a fully opaque red circle, so
    // you get a fully opaque green circle in the offscreen buffer. After
    // applying the 50% opacity of the group, you get a semi-transparent green
    // circle: rgba(0, 255, 0, 0.5).
    //
    // Example 2:
    //
    // <g fill-opacity="0.5">                       // => opacity = 1.0  fill-opacity = 0.5
    //   <circle cx="0" cy="0" r="10" fill="red">   // => opacity = 1.0  fill-opacity = 0.5
    //   <circle cx="0" cy="0" r="10" fill="green"> // => opacity = 1.0  fill-opacity = 0.5
    // </g>
    //
    // A semi-transparent green circle:               rgba(0, 255, 0, 0.5)    = Er, Eg, Eb, Ea  - Element
    // is drawn over a semi-transparent red circle:   rgba(255, 0, 0, 0.5)    = Cr, Cg, Cb, Ca  - Canvas (before blending)
    // so you get the following circle color/opacity: rgba(127, 255, 0, 0.75) = Cr',Cg',Cb',Ea' - Canvas (after blending)
    // in the offscreen buffer after applying the alpha blending rules:
    //     https://www.w3.org/TR/SVG11/masking.html#SimpleAlphaBlending
    //     Cr' = (1 - Ea) * Cr + Er
    //     Cg' = (1 - Ea) * Cg + Eg
    //     Cb' = (1 - Ea) * Cb + Eb
    //     Ca' = 1 - (1 - Ea) * (1 - Ca)
    // After applying the 100% opacity of the group (groups ignore fill-opacity),
    // you get the following circle: rgba(127, 255, 0, 0.75)
    //
    // Unfortunately, the behavior of Example 1 is impossible to achieve with the current
    // rendering model of VPaint, since we don't use any offscreen buffers for
    // compositing purposes. Therefore, we instead compose the group opacity
    // directly into the fill/stroke-opacity of children, which is not
    // equivalent (it gives you the same result as example 2), but is at least
    // better than ignoring the property altogether.
    //
    // Nice example to test behaviour:
    // https://www.w3.org/TR/SVG11/images/masking/opacity01.svg
    //
    if (style.contains("opacity") && readNumber(style["opacity"], &number)) {
        // Compose with children (instead of inherit)
        opacity_ *= qBound(0.0, number, 1.0);
    }
    else if (attrs.hasAttribute("opacity") && readNumber(attrs.value("opacity").toString(), &number)) {
        // Compose with children (instead of inherit)
        opacity_ *= qBound(0.0, number, 1.0);
    }

    update_();
}

void SvgPresentationAttributes::update_()
{
    // Compose the different opacity attributes together. In a compliant SVG
    // renderer, we would still have this step but without the last
    // multiplication with opacity_. The opacity_ would be applied differently,
    // using an offscreen buffer.
    fill = fill_;
    stroke = stroke_;
    fill.color.setAlphaF(fill.color.alphaF() * fillOpacity_ * opacity_);
    stroke.color.setAlphaF(stroke.color.alphaF() * strokeOpacity_ * opacity_);

    // Set strokeWidth to zero if stroke = none
    strokeWidth = stroke.hasColor ? strokeWidth_ : 0.0;
}

SvgPresentationAttributes::operator QString() const
{
    return QString("SvgPresentationAttribute(Fill = %1, Stroke = %2 @ %3 px)").arg(fill.color.name(QColor::HexArgb), stroke.color.name(QColor::HexArgb), QString::number(strokeWidth));
}
