#!/usr/bin/env python
# -*- coding: utf-8 -*-

#---------------------------------- README ------------------------------------
#
# This script requires Python >= 2.6. I has been tested with Python 2.7.6 on
# Ubuntu and Windows. It has not yet been tested with Python 3, or on MacOS,
# but probably just works.
#
# If you edit this file, then you must manually call qmake for changes to take
# effect.
#
# What does this script do?
#     1. Generating .config.pri project files
#     2. Generating unit test folders and files
#
#
# 1. Generating '.config.pri' project files
# -----------------------------------------
#
# For each project file in src/, e.g.:
#
#          <root-dir>/src/<project-rel-dir>/project.pro
#
# This script automatically generates an additional project file:
#
#     <root-out-dir>/src/<project-rel-dir>/.config.pri
#
# Which is included at the end of its respective project.pro.
#
# Each .config.pri contains boilerplate qmake variable assignments that should
# otherwise be manually written in project.pro, such as:
#
#     1. Common configuration options (CONFIG += c++11)
#
#     2. For 'app' and 'lib' project files:
#                INCLUDEPATH, DEPENDPATH, PRE_TARGETDEPS, LIBS, etc.
#
#     3. For 'subdirs' project files:
#                 <subdirname>.subdir, <subdirname>.depends
#
# Instead of writing all this boileplate, the 'app' and 'lib' project.pro files
# only have to assign to 'LIBS_DEPENDS' and 'THIRD_DEPENDS' the list of
# internal libraries and third-party libraries this app or lib depends on.
# 'LIBS_DEPENDS' and 'THIRD_DEPENDS' are custom variables ignored by qmake, but
# parsed by this configure.py script.
#
# After being parsed, the script will compute the "transitive closure" of the
# dependencies, which is just a fancy name to say that:
#
#     if:   ( Lib_1 depends on Lib_2 ) and ( Lib_2 depends on Lib_3 )
#     then: ( Lib_1 depends on Lib_3 )
#
# Even though this "transitive dependency" on Lib_3 is implicit and should not
# be explicitely specified in Lib_1.
#
# Finally, the transitive dependencies are sorted in a particular order that
# ensure that at link time, libraries are specified from most dependent to
# least dependent.
#
#
# 2. Generating unit test folders and files
# -----------------------------------------
#
# For each 'lib' project.pro file in src/, the generated .config.pri file also
# contains a command that executes another python script. This script will
# generate all the boilerplate folders and files to easily write unit tests.
#
# One unit test is automated created for each .h file in src/, and the unit
# test will pass by default. It is not required, but highly recommended, to add
# actual testing code to this unit test.
#


#--------------------------- Required modules ---------------------------------

# Built-in modules
import sys
import os
import shutil
import errno
import re

# Custom module
import buildutils


#--------------- Arguments passed to this script by qmake ---------------------

# <root-dir> directory
rootDir = sys.argv[1]

# <root-out-dir> directory (= build directory)
rootOutDir = sys.argv[2]

# qmake CONFIG variable value
config = sys.argv[3:]


#--------------- Useful configuration variables -------------------------------

# Test if building on a Unix system
# This is determined by the presence of "unix" in CONFIG
if 'unix' in config:
    unix = True
else:
    unix = False

# Test if building on a Windows system
# This is determined by the presence of "win32" in CONFIG
if 'win32' in config:
    win32 = True
else:
    win32 = False

# Test if building in release or debug mode
# This is determined by which of 'release' and 'debug' appears last in CONFIG
release = True
debug = False
for s in config:
    if s == 'release':
        release = True
        debug = False
    elif s == 'debug':
        release = False
        debug = True

# Convenient variable holding a string equals to either 'release' or 'debug'
if release:
    releaseOrDebug = 'release'
else:
    releaseOrDebug = 'debug'

# Python command depending on OS
if win32:
    pythonCmd = 'python.exe'
else:
    pythonCmd = 'python'


#------------------------- Useful directories ---------------------------------

# Note: we use forward slashes even on Windows, since this is what qmake
# expects. For this reason, we do not use os.path.join.

buildtoolsDir = rootDir + '/buildtools'

srcDir   = rootDir + '/src'
appDir   = rootDir + '/src/app'
libsDir  = rootDir + '/src/libs'
thirdDir = rootDir + '/src/third'
testsDir = rootDir + '/tests'

srcOutDir   = rootOutDir + '/src'
appOutDir   = rootOutDir + '/src/app'
libsOutDir  = rootOutDir + '/src/libs'
thirdOutDir = rootOutDir + '/src/third'
testsOutDir = rootOutDir + '/tests'


#---------------------------- Text content ------------------------------------

headerText = (
"""
#####################################################################
#   This file was automatically generated. Any edit will be lost.   #
#####################################################################
"""
)

makelibtestsText = (
"""
# Generate unit tests for new header files
system(%pythonCmd %buildtoolsDir/makelibtests.py %rootDir %rootOutDir $$_PRO_FILE_PWD_ $$OUT_PWD $$CONFIG)
"""
.replace('%pythonCmd',     pythonCmd)
.replace('%buildtoolsDir', buildtoolsDir)
.replace('%rootDir',       rootDir)
.replace('%rootOutDir',    rootOutDir)
)

makesubdirstestsText = (
"""
# Generate unit tests for new libraries
system(%pythonCmd %buildtoolsDir/makesubdirstests.py %rootDir %rootOutDir $$_PRO_FILE_PWD_ $$OUT_PWD $$CONFIG)
"""
.replace('%pythonCmd',     pythonCmd)
.replace('%buildtoolsDir', buildtoolsDir)
.replace('%rootDir',       rootDir)
.replace('%rootOutDir',    rootOutDir)
)

enableCpp11Text = (
"""
# Enable C++11
CONFIG += c++11
"""
)

staticLibText = (
"""
# Compile as a static library
CONFIG += staticlib
"""
)

includeText = (
"""
# Add third-party libraries and internal libraries to INCLUDEPATH.
INCLUDEPATH += %srcDir/third/
INCLUDEPATH += %srcDir/libs/
"""
.replace('%srcDir', srcDir)
)

includeTextUnixOnly = (
"""
# Consider third-party libraries like system libraries (e.g., silence warnings)
QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM %srcDir/third/
"""
.replace('%srcDir', srcDir)
)

if unix:
    includeText += includeTextUnixOnly


#------------- text to add a lib this project depends on ----------------------

addLibTextUnix = (
"""
# Add dependency to %libRelDir
LIBS += -L%libOutDir/ -l%libname
PRE_TARGETDEPS += %libOutDir/lib%libname.a
"""
)

addLibTextWin32 = (
"""
# Add dependency to %libRelDir
LIBS += -L%libOutDir/%releaseOrDebug/ -l%libname
PRE_TARGETDEPS += %libOutDir/%releaseOrDebug/%libname.lib
"""
.replace('%releaseOrDebug', releaseOrDebug)
)

if unix:
    addLibText = addLibTextUnix
else:
    addLibText = addLibTextWin32


#---------- Project class to store info parsed from the .pro files ------------

# Project class
class Project:

    def __init__(self):

        # Project-related directories and filenames

        self.name = ""      # Project
        self.relDir = ""    # Path/To/Project

        self.dir = ""       # <root-dir>/src/Path/To/Project
        self.outDir = ""    # <root-out-dir>/src/Path/To/Project

        self.proFileName = ""  # Project.pro
        self.proFilePath = ""  # <root-dir>/src/Path/To/Project/Project.pro

        self.priFileName = ""  # .config.pri
        self.priFilePath = ""  # <root-out-dir>/src/Path/To/Project/.config.pri


        # Data parsed from project files

        self.template = ""        # Parsed value of TEMPLATE, i.e.: "app", "lib", or "subdirs"

        self.config   = []        # Parsed value of CONFIG,   e.g.: ['lex', 'yacc', 'debug', 'exceptions',
                                  #     'depend_includepath', 'testcase_targets', 'import_plugins',
                                  #     'import_qpa_plugin', 'qt', 'warn_on', 'release', 'link_prl',
                                  #     'incremental', 'shared', 'qpa', 'no_mocdepend', 'release',
                                  #     'qt_no_framework', 'linux', 'unix', 'posix', 'gcc']
                                  #   It is initialized by "config" passed as argument to this script

        self.qt       = []        # Parsed value of QT,       e.g.: [ "core", "gui", "widgets" ]
                                  #   Empty if CONFIG -= qt
                                  #   Contains "core" and "gui" by default unless removed via:
                                  #          QT -= core gui

        self.subdirs  = []        # Parsed value of SUBDIRS,  e.g.: [ "Widgets", "Windows"]

        self.third_depends  = []  # Parsed value of THIRD_DEPENDS, e.g.: [ "Geometry" ]
        self.lib_depends    = []  # Parsed value of LIB_DEPENDS,   e.g.: [ "Gui/Widgets" ]


        # Transitive closure of the depends relationships

        self.tdependsIsComputed      = False  # Prevent computing more than once
        self.tdependsIsBeingComputed = False  # Detect cyclic dependencies

        self.qt_tdepends    = set() # e.g., { "core", "gui", "widgets" }
        self.third_tdepends = set() # e.g., { "Geometry" }
        self.lib_tdepends   = set() # e.g., { "Gui/Widgets", "Core" }


        # Transitive closure ordered via topological sort.
        # This specifies the order in which libs should be linked against.

        self.qt_sdepends    = set() # e.g., [ "core", "gui", "widgets" ]
        self.third_sdepends = set() # e.g., [ "Geometry" ]
        self.lib_sdepends   = set() # e.g., [ "Core", "Gui/Widgets" ]


        # Parent/child relationship between projects

        self.parentProject = None
        self.subProjects = []


        # Subdir dependencies (computed from lib dependencies)

        self.subdir    = ""             # To/Project ( = dir of this project relative to parent project)
        self.subdirKey = ""             # To_Project ( = key to identify this subdir without using slashes)
        self.subdirDependsKeys = set()  # Examples:
                                        #   subdirs of src:
                                        #     {}                   for src/third/third.pro
                                        #     { "third" }          for src/libs/libs.pro
                                        #     { "third", "libs" }  for src/app/app.pro
                                        #
                                        #   subdirs of src/third:
                                        #     {}                   for src/third/Geometry/Geometry.pro
                                        #
                                        #   subdirs of src/libs:
                                        #     {}                   for src/libs/Core/Core.pro
                                        #     { "Core" }           for src/libs/Gui/Gui.pro
                                        #
                                        #   subdirs of src/libs/Gui:
                                        #     {}                   for src/libs/Gui/Widgets/Widgets.pro
                                        #     { "Widgets" }        for src/libs/Gui/Windows/Windows.pro


# Dictionary storing all projects in <root-dir>/src, accessed by their relDir
projects = {}

# Returns the internal library project corresponding to the given libname.
# libname is the path of the internal library relative to libs/
# Examples:
#     Core
#     Gui/Widgets
#     Gui/Windows
def getLibProject(libname):
    if ("libs/" + libname) in projects:
        return projects[("libs/" + libname)]
    else:
        print "Error: library libs/" + libname + " not found."

# Returns the third-party library project corresponding to the given libname.
# libname is the path of the third-party library relative to third/
# Example:
#     Geometry
def getThirdProject(libname):
    if ("third/" + libname) in projects:
        return projects[("third/" + libname)]
    else:
        print "Error: library third/" + libname + " not found."


#----------------------------- Actual script ----------------------------------

# Find all project files in src/
print "[configure.py] Finding project files"
for x in os.walk(srcDir):
    dirpath = x[0].replace('\\', '/') # manually replace backslashes with slashes for Windows
    filenames = x[2]
    for filename in filenames:
        if filename.endswith('.pro'):

            # Create project object
            project = Project()

            # Set project-related directories and file names
            project.proFilePath = dirpath + '/' + filename  # <root-dir>/src/Path/To/Project/Project.pro

            project.proFileName = filename       # Project.pro
            project.name        = filename[:-4]  # Project

            project.dir    = dirpath                  # <root-dir>/src/Path/To/Project
            project.relDir = dirpath[len(srcDir)+1:]  # Path/To/Project

            project.outDir = srcOutDir + dirpath[len(srcDir):]  # <root-out-dir>/src/Path/To/Project

            project.priFileName = ".config.pri"                               # .config.pri
            project.priFilePath = project.outDir + '/' + project.priFileName  # <root-out-dir>/src/Path/To/Project/.config.pri

            # Insert in dictionary storing all projects, using relDir as the key
            projects[project.relDir] = project


# Parse projects
print "[configure.py] Parsing project files"
for relDir in projects:
    # Get project
    project = projects[relDir]

    # Read project file, return as a string
    data = buildutils.readFromFile(project.proFilePath)

    # Parse relevant qmake variables
    project.template = buildutils.getQmakeVariable('TEMPLATE', data)[0]
    project.config   = buildutils.getQmakeVariable('CONFIG', data, config)

    if 'qt' in project.config:
        project.qt = buildutils.getQmakeVariable('QT', data, ["core", "gui"])
    else:
        project.qt = []

    project.subdirs        = buildutils.getQmakeVariable('SUBDIRS', data)
    project.third_depends  = buildutils.getQmakeVariable('THIRD_DEPENDS', data)
    project.lib_depends    = buildutils.getQmakeVariable('LIB_DEPENDS', data)


# Recursive function that computes the transitive closure of the depends
# relationships of the given project.
#
# This function terminates thanks to "leaf-projects" which do not depend on
# any other libraries, i.e. projects for which:
#     len(project.third_depends) == 0 and
#     len(project.lib_depends) == 0
#
# Cyclic dependencies are detected and reported as an error.
#
def computeTDepends(project):
    # Do nothing if already computed
    if project.tdependsIsComputed:
        pass

    # Detect cyclic dependencies
    if project.tdependsIsBeingComputed:
        print "Error:", project.name, "has a cyclic dependency."
        pass

    # Mark as being computed
    project.tdependsIsBeingComputed = True

    # Initialize
    project.qt_tdepends    = set(project.qt)
    project.third_tdepends = set(project.third_depends)
    project.lib_tdepends   = set(project.lib_depends)

    # Recurse for third-party libraries
    for libname in project.third_depends:
        thirdProject = getThirdProject(libname)
        computeTDepends(thirdProject)
        project.qt_tdepends    = project.qt_tdepends.union   (thirdProject.qt_tdepends)
        project.third_tdepends = project.third_tdepends.union(thirdProject.third_tdepends)

    # Recurse for internal libraries
    for libname in project.lib_depends:
        libProject = getLibProject(libname)
        computeTDepends(libProject)
        project.qt_tdepends    = project.qt_tdepends.union   (libProject.qt_tdepends)
        project.third_tdepends = project.third_tdepends.union(libProject.third_tdepends)
        project.lib_tdepends   = project.lib_tdepends.union  (libProject.lib_tdepends)

    # Mark as computed
    project.tdependsIsBeingComputed = False
    project.tdependsIsComputed = True


# Compute the transitive closure of all projects dependencies
for relDir in projects:
    # Get project
    project = projects[relDir]

    # Compute tDepends
    computeTDepends(project)


# Compare method to sort third-party libraries from least dependent to most dependent
def thirdLessThan(libname1, libname2):
    thirdProject1 = getThirdProject(libname1)
    thirdProject2 = getThirdProject(libname2)
    if libname1 in thirdProject2.third_tdepends:
        return -1
    elif libname2 in thirdProject1.third_tdepends:
        return +1
    else:
        return 0


# Compare method to sort internal libraries from least dependent to most dependent
def libLessThan(libname1, libname2):
    libProject1 = getLibProject(libname1)
    libProject2 = getLibProject(libname2)
    if libname1 in libProject2.lib_tdepends:
        return -1
    elif libname2 in libProject1.lib_tdepends:
        return +1
    else:
        return 0


# Sort dependencies from least dependent to most dependent
# (i.e., if lib1 depends on lib2, then lib1 appears after lib1 in the list)
for relDir in projects:
    # Get project
    project = projects[relDir]

    # Qt dependencies do not require sorting
    project.qt_sdepends = list(project.qt_tdepends)

    # Sort third-party library dependencies
    third_tdepends = list(project.third_tdepends)
    project.third_sdepends = sorted(third_tdepends, cmp=thirdLessThan)

    # Sort third-party library dependencies
    lib_tdepends = list(project.lib_tdepends)
    project.lib_sdepends = sorted(lib_tdepends, cmp=libLessThan)


# Set parent/child relationships
for relDir in projects:
    # Get project
    project = projects[relDir]

    # For all subdir in subdirs
    for subdir in project.subdirs:
        # Get relDir of subproject
        if project.relDir == "":
            subProjectRelDir = subdir                         # e.g., "app"
        else:
            subProjectRelDir = project.relDir + '/' + subdir  # e.g., "libs" + '/' + "Core"

        # Check if subProject exists
        if subProjectRelDir in projects:
            # Get subproject
            subProject = projects[subProjectRelDir]

            # Set parent/child relationships
            project.subProjects.append(subProject)
            subProject.parentProject = project
            subProject.subdir = subdir
            subProject.subdirKey = subdir.replace('/', '__')

        else:
            print ("Error: subproject", subProjectRelDir, "of project", project.relDir, "not found.")


# Returns a list of all ancestors of a project. The first element of the
# returned list is the root project of the distribution, and the last element
# is the given project.
#
def getAncestors(project):
    ancestors = [project]
    while ancestors[0].parentProject != None:
        ancestors.insert(0, ancestors[0].parentProject)
    return ancestors


# Resolve subdirs dependencies based on lib/app dependencies
for relDir in projects:
    # Get project
    project = projects[relDir]

    # Get all non-subdirs projects this project depends on
    dependeeProjects = []
    for libname in project.third_sdepends:
        dependeeProjects.append(getThirdProject(libname))
    for libname in project.lib_sdepends:
        dependeeProjects.append(getLibProject(libname))

    # Get all ancestors of this project
    projectAncestors = getAncestors(project)

    # Infer subdir dependencies from lib dependencies
    for libProject in dependeeProjects:
        # Get all ancestors of libProject
        libProjectAncestors = getAncestors(libProject)

        # Find common ancestor between this project and libProject
        indexCommonAncestor = 0
        while projectAncestors[indexCommonAncestor+1] == libProjectAncestors[indexCommonAncestor+1]:
            indexCommonAncestor += 1

        # Add subdir dependency
        dependentProject = projectAncestors[indexCommonAncestor+1]
        dependeeProject  = libProjectAncestors[indexCommonAncestor+1]
        dependentProject.subdirDependsKeys.add(dependeeProject.subdirKey)


# Generate all .config.pri files
for relDir in projects:
    # Get project
    project = projects[relDir]

    # Content to write to .config.pri
    content = ""

    # Write header
    content += headerText

    # Write computed dependencies
    if project.template == "lib" or project.template == "app":
        content += "\n# Computed dependencies (transitive + sorted). This is for info only.\n"
        content += "QT_SDEPENDS    = " + " ".join(project.qt_sdepends)    + "\n"
        content += "THIRD_SDEPENDS = " + " ".join(project.third_sdepends) + "\n"
        content += "LIB_SDEPENDS   = " + " ".join(project.lib_sdepends)   + "\n"

    # Write python calls that generates unit tests
    #if project.relDir.startswith('libs'):
    #    if project.template == "lib":
    #        content += makelibtestsText
    #    elif project.template == "subdirs":
    #        content += makesubdirstestsText

    # Enable C++11
    if project.template == "lib" or project.template == "app":
        content += enableCpp11Text

    # Adds all headers of the distribution in include path
    if project.template == "lib" or project.template == "app":
        content += includeText

    # Compile as a static library
    if project.template == "lib":
        content += staticLibText

    # If project is a subdir
    if project.template == "subdirs":
        # Override value of SUBDIRS by using keys instead of folder path
        subdirsText = ("\n" +
                       "# Override value of SUBDIRS by using keys instead of folder path\n" +
                       "SUBDIRS =")
        for subProject in project.subProjects:
            subdirsText += " \\\n    " + subProject.subdirKey
        subdirsText += "\n"
        content += subdirsText

        # Set subdirs and dependencies
        for subProject in project.subProjects:
            subdirText = ("\n" +
                          "# Set " + subProject.subdirKey + " location and dependencies\n")
            subdirText += subProject.subdirKey + ".subdir  = " + subProject.subdir + "\n"
            subdirText += subProject.subdirKey + ".depends ="
            for key in subProject.subdirDependsKeys:
                subdirText += " " + key
            subdirText += "\n"
            content += subdirText

    # Add each dependent library in order from most dependent to least dependent.

    # Internal libraries
    for libname in reversed(project.lib_sdepends):
        libProject = getLibProject(libname)
        addThisLib = (addLibText.replace('%libRelDir', libProject.relDir)
                                .replace('%libOutDir', libProject.outDir)
                                .replace('%libname',   libProject.name))
        content += addThisLib

    # Third-party libraries
    for libname in reversed(project.third_sdepends):
        libProject = getThirdProject(libname)
        addThisLib = (addLibText.replace('%libRelDir', libProject.relDir)
                                .replace('%libOutDir', libProject.outDir)
                                .replace('%libname',   libProject.name))
        content += addThisLib

    # Add depedencies to Qt libraries
    if project.template == "lib" or project.template == "app":
        content += "\n# Add dependencies to Qt libraries.\n"
        content += "QT += " + " ".join(project.qt_sdepends)    + "\n"

    # Writes content to .config.pri
    buildutils.writeToFileIfDifferent(project.priFilePath, content)
