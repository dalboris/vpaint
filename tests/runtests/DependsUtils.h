#ifndef DEPENDENCYANALYZER_H
#define DEPENDENCYANALYZER_H

#include <QDir>
#include <QString>
#include <QStringList>

/// \class DependsUtils
/// \brief Convenient static functions related to dependency analysis.
///
class DependsUtils
{
public:
    /// Convenient struct to hold the SDEPENDS data.
    ///
    struct SDepends
    {
        QStringList qt;
        QStringList third;
        QStringList lib;
    };

    /// Gets the sorted recursive dependencies of the given source code.
    /// This relies on the .cong.pri files, so configure.py must have
    /// been executed before this is called.
    ///
    /// Here is how it works:
    ///   1. Search for include directives (e.g., #include "Module/Lib/Subdir/Foo.h")
    ///   2. For each of them:
    ///        3. Detect whether it is in src/third/, or src/libs, or none (i.e., Qt or std lib).
    ///           In the later case, we return empty dependencies. Otherwise, the steps below are done
    ///        4. Find where the inner-most project file is, i.e., "Module/Lib" ("Subdir can just be
    ///           a folder to help organization, but without .pro file, i.e., not an independent lib)
    ///        5. Parse the .config.pri file associated with the library, and extract its SDEPENDS, e.g.:
    ///               SDepends.qt    = ["core", "gui", "widgets"]
    ///               SDepends.third = ["Geometry"]
    ///               SDepends.libs  = ["Lib1", "Lib2"]
    ///        6. Add the library itself to the SDEPENDS:
    ///               SDepends.qt    = ["core", "gui", "widgets"]
    ///               SDepends.third = ["Geometry"]
    ///               SDepends.libs  = ["Lib1", "Lib2", "Module/Lib"]
    ///   7. Once this is done for each include directive, concatenate this info into
    ///      a single SDEPDENDS
    ///
    static SDepends getSourceSDepends(const QString & sourceFileContent);

    /// Gets the sorted recursive dependencies required to use the given
    /// included header. This performs the job of items 3-6 from the documentation of
    /// getSourceSDepends().
    ///
    static SDepends getHeaderSDepends(const QString & headerIncludePath);

    /// Gets all the qt/third/libs SDEPENDS of the given third-party library.
    /// libRelPath is relative to src/third/.
    /// This is steps 5-6 mentioned in the documentation of getSourceSDepends().
    ///
    static SDepends getThirdLibSDepends(const QString & libRelPath);

    /// Gets all the qt/third/libs SDEPENDS of the given internal library.
    /// libRelPath is relative to src/libs/.
    /// This is steps 5-6 mentioned in the documentation of getSourceSDepends().
    ///
    static SDepends getInternalLibSDepends(const QString & libRelPath);

    /// Gets all the qt/third/libs SDEPENDS extracted from the given config file.
    ///
    static SDepends getConfigFileSDepends(const QString & configFileContent);

    /// Reads the content of the .config.pri file associated with the given
    /// third-party library at \p libRelPath. The path is relative to src/third/
    ///
    static QString readThirdLibConfigFile(const QString & libRelPath);

    /// Reads the content of the .config.pri file associated with the given
    /// internal library at \p libRelPath. The path is relative to src/libs/
    ///
    static QString readInternalLibConfigFile(const QString & libRelPath);

    /// Gets the value of a qmake variable as defined in a given Qt project file.
    /// You can provide an initial value, which is useful when only '+=' and '-='
    /// statements are defined in the project file. Any '=' statement would make
    /// the initial value useless.
    ///
    static QStringList getQMakeVariable(const QString & variableName,
                                        const QString & projectFileContent,
                                        const QStringList & initialValue = QStringList());
};

#endif // DEPENDENCYANALYZER_H
