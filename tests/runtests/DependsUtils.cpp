#include "DependsUtils.h"
#include "DirUtils.h"

#include <QRegularExpression>
#include <cassert>

#include <QtDebug>

DependsUtils::SDepends DependsUtils::getSourceSDepends(const QString & sourceFileContent)
{
    // RegExp to find included files
    // It's not perfect but should work in any sane case.
    // For instance, it would fail to correctly parse:
    //     #include <foo"bar.h>
    //     #include "foo<bar.h"
    //     #include /* some comment */ <Foo/Bar.h>
    // but it would be silly to have a header file with a quote or bracket character,
    // or insert a comment between include and the file path
    QRegularExpression includeRegExp("#[ \\t]*include[ \\t]+[\"<]([^\\n\"<>]*)[\">]");

    // result (concatenatind all header sdepends)
    SDepends res;

    // Find regexp matches and traverse them
    QRegularExpressionMatchIterator i = includeRegExp.globalMatch(sourceFileContent);
    while (i.hasNext())
    {
        // Get header include path (e.g., "Gui/Widgets/Subdir/Widget.h"
        QRegularExpressionMatch match = i.next();
        QString headerIncludePath = match.captured(1);

        // Get header SDEPENDS
        SDepends headerSDepends = getHeaderSDepends(headerIncludePath);

        // Concatenate with existing SDEPENDS
        foreach (QString s, headerSDepends.qt)
            if (!res.qt.contains(s))
                res.qt.append(s);
        foreach (QString s, headerSDepends.third)
            if (!res.third.contains(s))
                res.third.append(s);
        foreach (QString s, headerSDepends.lib)
            if (!res.lib.contains(s))
                res.lib.append(s);
    }

    // Return result of analysis
    return res;
}

namespace
{
// Helper function for getHeaderSDepends()
//
// Search for a library located in the given dir, given its header
// include path, assumed to be relative to the given dir. Returns
// the path of the library relative to the given dir. Returns an
// empty string if the library was not found in the given dir.
//
// Example input:
//     dir               = <root-dir>/src/libs
//     headerIncludePath = "Gui/Widgets/Subdir/Widget.h"
//
// Example output: (lib found in <root-dir>/src/libs/Gui/Widgets)
//     "Gui/Widgets"
//
QString findLibRelPath_(const QDir & dir,
                        const QString & headerIncludePath)
{
    // Split into components ["Gui", "Widgets", "Subdir", "Widget.h"]
    QStringList names = headerIncludePath.split('/');

    // Find component index. Returns 0 if not found. Returns 2 in the
    // example given in comments.
    int i = 0;
    QDir libDir = dir;
    while (i < names.size()-1 &&
           libDir.exists(names[i]))
    {
        QString dirName = names[i];
        DirUtils::cd(libDir, dirName);

        QString proFileName = dirName + ".pro";
        if (libDir.exists(proFileName))
            ++i;
        else
            break;
    }

    // Only keep relevant components. Example: ["Gui", "Widgets"]
    names = names.mid(0, i);

    // Merge back components
    return names.join('/');
}
}

DependsUtils::SDepends DependsUtils::getHeaderSDepends(const QString & headerIncludePath)
{
    // Get src/third/ dir
    QDir thirdDir = DirUtils::rootDir();
    DirUtils::cd(thirdDir, "src/third");

    // Get src/libs/ dir
    QDir libsDir = DirUtils::rootDir();
    DirUtils::cd(libsDir, "src/libs");

    // Find header in either src/third/ or src/libs/
    QString libRelPathInThird = findLibRelPath_(thirdDir, headerIncludePath);
    QString libRelPathInLibs  = findLibRelPath_(libsDir,  headerIncludePath);

    // Get SDEPENDS of found library.
    if (!libRelPathInLibs.isEmpty())
    {
        SDepends res = getInternalLibSDepends(libRelPathInLibs);
        if (!res.lib.contains(libRelPathInLibs))
            res.lib.append(libRelPathInLibs);
        return res;
    }
    else if (!libRelPathInThird.isEmpty())
    {
        SDepends res = getThirdLibSDepends(libRelPathInThird);
        if (!res.third.contains(libRelPathInThird))
            res.third.append(libRelPathInThird);
        return res;
    }
    else
    {
        return SDepends();
    }
}

DependsUtils::SDepends DependsUtils::getThirdLibSDepends(const QString & libRelPath)
{
    QString configFileContent = readThirdLibConfigFile(libRelPath);
    return getConfigFileSDepends(configFileContent);
}

DependsUtils::SDepends DependsUtils::getInternalLibSDepends(const QString & libRelPath)
{
    QString configFileContent = readInternalLibConfigFile(libRelPath);
    return getConfigFileSDepends(configFileContent);
}

DependsUtils::SDepends DependsUtils::getConfigFileSDepends(const QString & configFileContent)
{
    SDepends res;
    res.qt    = getQMakeVariable("QT_SDEPENDS",    configFileContent);
    res.third = getQMakeVariable("THIRD_SDEPENDS", configFileContent);
    res.lib   = getQMakeVariable("LIB_SDEPENDS",   configFileContent);
    return res;
}

namespace
{
// Helper function for readThirdLibConfigFile() and readThirdLibConfigFile()
//
QString readConfigFile_(
        const QDir & outDir,
        const QString & libRelPath)
{
    // Get lib out dir
    QDir libOutDir = outDir;
    DirUtils::cd(libOutDir, libRelPath);

    // Read config file
    QFile libConfigFile(libOutDir.absoluteFilePath(".config.pri"));
    assert(libConfigFile.open(QIODevice::ReadOnly));
    QTextStream libConfigTextStream(&libConfigFile);
    return libConfigTextStream.readAll();
}
}

QString DependsUtils::readThirdLibConfigFile(const QString & libRelPath)
{
    QDir thirdOutDir = DirUtils::rootOutDir();
    DirUtils::cd(thirdOutDir, "src/third");
    return readConfigFile_(thirdOutDir, libRelPath);
}

QString DependsUtils::readInternalLibConfigFile(const QString & libRelPath)
{
    QDir libsOutDir = DirUtils::rootOutDir();
    DirUtils::cd(libsOutDir, "src/libs");
    return readConfigFile_(libsOutDir, libRelPath);
}

namespace
{
// Helper function for getQMakeVariable()
//
QStringList qmakeStringToList_(const QString & string)
{
    QStringList res;

    QRegularExpression re("[/\\w']+");
    QRegularExpressionMatchIterator i = re.globalMatch(string);
    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        res << match.captured(0);
    }

    return res;
}
}

QStringList DependsUtils::getQMakeVariable(
        const QString & variableName,
        const QString & projectFileContent,
        const QStringList & initialValue)
{
    // RegExp patterns
    QString regexPattern = variableName +
            "\\s*_SIGN_([^\\n\\\\]*(\\\\[^\\S\\n]*\\n[^\\n\\\\]*)*)";

    QString regexPatternEqual = regexPattern;
    regexPatternEqual.replace("_SIGN_", "=");

    QString regexPatternPlusEqual = regexPattern;
    regexPatternPlusEqual.replace("_SIGN_", "\\+=");

    QString regexPatternMinusEqual = regexPattern;
    regexPatternMinusEqual.replace("_SIGN_", "-=");

    // RegExps
    QRegularExpression equalRegExp(regexPatternEqual);
    QRegularExpression plusEqualRegExp(regexPatternPlusEqual);
    QRegularExpression minusEqualRegExp(regexPatternMinusEqual);

    // Search for regex
    QRegularExpressionMatch equal = equalRegExp.match(projectFileContent);
    QRegularExpressionMatchIterator iPlusEqual = plusEqualRegExp.globalMatch(projectFileContent);
    QRegularExpressionMatchIterator iMinusEqual = minusEqualRegExp.globalMatch(projectFileContent);

    // Initial value
    QStringList res = initialValue;

    // Override if '=' found
    if (equal.hasMatch())
    {
        QString matched = equal.captured(1);
        res = qmakeStringToList_(matched);
    }

    // Add all '+=' found
    while (iPlusEqual.hasNext())
    {
        QRegularExpressionMatch match = iPlusEqual.next();
        QString matched = match.captured(1);
        res.append(qmakeStringToList_(matched));
    }

    // Remove all '-=' found
    while (iMinusEqual.hasNext())
    {
        QRegularExpressionMatch match = iMinusEqual.next();
        QString matched = match.captured(1);
        QStringList strings = qmakeStringToList_(matched);
        QStringList newRes;
        for (int i=0; i<res.size(); ++i)
        {
            if (!strings.contains(res[i]))
            {
                newRes.append(res[i]);
            }
        }
        res.swap(newRes);
    }

    // Return
    return res;
}
