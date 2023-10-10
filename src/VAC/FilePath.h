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

#ifndef FILE_PATH_H
#define FILE_PATH_H

#include <QString>

/// \class FilePath
/// \brief Manipulates a file path.
///
/// This class is similar to `std::filesystem::path`, which has the issue
/// to require C++17 and GCC >= 8, so we use our own minimal implementation
/// in the meantime.
///
class FilePath {
public:
    /// Creates a FilePath from the given string `path`.
    ///
    FilePath(const QString & path);

    /// Converts the file path to a string.
    ///
    QString toString();

    /// Returns the part of the path before the filename.
    ///
    QString prefix() const {
        return prefix_;
    }

    /// Returns the stem.
    ///
    QString stem() const {
        return stem_;
    }

    /// Returns the extension, including the leading dot if any.
    ///
    QString extension() const {
        return extension_;
    }

    /// Returns the extension, not including the leading dot (if any).
    ///
    QString extensionWithoutLeadingDot() const;

    /// Replaces the extension. Automatically adds a leading
    /// dot if the given `extension` doesn't start with one.
    ///
    void replaceExtension(const QString & extension);

    /// Replaces the stem.
    ///
    void replaceStem(const QString & stem);

private:
    QString prefix_;    // `some/dir/`
    QString stem_;      // `myfile.tar`
    QString extension_; // `.gz`
};

#endif
