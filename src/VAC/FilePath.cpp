// Copyright (C) 2012-2023 The VPaint Developers.
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

#include "FilePath.h"

// Note: QFileInfo has the following behavior in case of trailing dots:
//
// path                  baseName    completeSuffix
//
// foo/test.tar.         test        tar.
//
// foo/test..            test        .
// foo/test.             test        (nothing)
// foo/test              test        (nothing)
//
// foo/..                (nothing)   .
// foo/.                 (nothing)   (nothing)
// foo/                  (nothing)   (nothing)
//
// Note how with just the baseName and completeSuffix, it is not possible to
// distinguish between `test.` and `test`. This means that with this design, we
// need to somehow store this extra information.
//
// In contrast, std::filesystem::path has IMHO the better design, where the
// leading dot is considered part of the extension, so that user can explicitly
// set replace_extension(".zip") vs replace_extension(".") vs
// replace_extension(""). See:
// https://en.cppreference.com/w/cpp/filesystem/path/replace_extension
//
// However, std::filesystem::path doesn't have the concept of "complete
// extension". It would be nice but for now we only have svg and png,
// nothing like "tar.gz", so we use the same API as std::filesystem::path
// so that it's easier to migrate to std::filesystem::path later.

FilePath::FilePath(const QString & path) {

    // Extract prefix, that is everything before the filename.
    //
    // On Windows, both `/` and `\` are considered path separators.
    //
    // On Linux/macSO, only `/` is considered a path separator, and `\` can
    // be used as normal character.
    //
    int i = path.lastIndexOf('/'); // Returns -1 if not found
#ifdef Q_OS_WINDOWS
    i = (std::max)(i, path.lastIndexOf('\\'));
#endif
    prefix_ = path.left(i + 1);         // OK if i == -1
    QString filename = path.mid(i + 1); // OK if i == -1

    // Decompose filename in stem and extension
    //
    // https://en.cppreference.com/w/cpp/filesystem/path/extension
    //
    // Path                     Extension
    //
    // "/foo/bar.txt"           ".txt"
    // "/foo/bar."              "."
    // "/foo/bar"               ""
    // "/foo/bar.txt/bar.cc"    ".cc"
    // "/foo/bar.txt/bar."      "."
    // "/foo/bar.txt/bar"       ""
    // "/foo/."                 ""
    // "/foo/.."                ""
    // "/foo/.hidden"           ""
    // "/foo/..bar"             ".bar"
    //
    int j = filename.lastIndexOf('.');
    if (j == -1 || j == 0 || filename == "." || filename == "..") {
        stem_ = filename;
        extension_ = "";
    }
    else {
        stem_ = filename.left(j);
        extension_ = filename.mid(j);
    }
}

QString FilePath::toString() {
    return prefix_ + stem_ + extension_;
}

QString FilePath::extensionWithoutLeadingDot() const {
    QString res = extension_;
    if (res.startsWith(".")) {
        res.remove(0, 1); // Remove first character
    }
    return res;
}

void FilePath::replaceExtension(const QString & extension) {
    if (!extension.isEmpty() && extension.front() != '.') {
        extension_ = "." + extension;
    }
    else {
        extension_ = extension;
    }
}

void FilePath::replaceStem(const QString & stem) {
    stem_ = stem;
}
