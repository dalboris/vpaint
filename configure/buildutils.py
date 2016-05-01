#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import shutil
import errno
import re

runtests_pro = (
"""
#####################################################################
#   This file was automatically generated. Any edit will be lost.   #
#####################################################################

TEMPLATE = app
CONFIG  -= qt
TARGET   = runtests
SOURCES  = runtests.cpp
"""
)

runtests_cpp = (
"""
/*********************************************************************
 *   This file was automatically generated. Any edit will be lost.   *
 *********************************************************************/

#include <cstdlib>

int main()
{
    return std::system("python runtests.py");
}
"""
)

runtests_py = (
"""
#####################################################################
#   This file was automatically generated. Any edit will be lost.   #
#####################################################################

import subprocess

print "Hi from runtests.py"

#sp = subprocess.Popen("./Test_Gui_Windows_MainWindow", stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#out, err = sp.communicate()
#if out:
#    print "standard output of subprocess:"
#    print out
#if err:
#    print "standard error of subprocess:"
#    print err
#print "returncode of subprocess:"
#print sp.returncode
"""
)

# Creates a directory and all its parent (if they don't already exist)
def mkdir(dirpath):
    try:
        os.makedirs(dirpath)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(dirpath):
            pass
        else:
            raise

# Reads content from file. Crashes if file doesn't exist.
def readFromFile(filePath):
    f = open(filePath, 'r')
    content = f.read()
    f.close()
    return content

# Reads content from file. Returns "" if file doesn't exist.
def readFromFileIfExists(filePath):
    if os.path.isfile(filePath):
        return readFromFile(filePath)
    else:
        return ""

# Writes content to file, erasing previous content if file already exists.
# Never fails. Creates directories and file as necessary.
def writeToFile(filePath, fileContent):
    mkdir(os.path.dirname(filePath))
    f = open(filePath, 'w')
    f.write(fileContent)
    f.close()

# Same as above, but test first for content and only erases if content differs.
def writeToFileIfDifferent(filePath, fileContent):
    if os.path.isfile(filePath):
        existingFileContent = readFromFile(filePath)
        if fileContent != existingFileContent:
            writeToFile(filePath, fileContent)
    else:
        writeToFile(filePath, fileContent)


# Returns as a string the value of the given qmake variable 'variableName',
# defined in the given inputConfig. Returns None if the variable
# is not found.
def getVariableValueAsString(variableName, inputConfig):
    regexPattern = variableName + r"\s*=([^\n\\]*(\\[^\S\n]*\n[^\n\\]*)*)"
    match = re.search(regexPattern, inputConfig)
    if match:
        # Matched string (something like " \\\n    value1 \\\n    value2")
        return match.groups()[0]

# Returns the TEMPLATE value parsed from the given string.
def getTemplate(string):
    match = re.search(r"TEMPLATE\s*=\s*\b(\w+)\b", string)
    if match:
        return match.groups()[0]

# Returns as a list the values of the given qmake variable 'variableName',
# defined in the given inputConfig. Returns an empty list if the variable
# is not found.
#
# Example input:
#
# """
# VARIABLE_I_DONT_CARE = \
#     whatever1 \
#     whatever2 \
#     whatever3
#
# VARIABLE_NAME = \
#     value1 \
#     value2 \
#     value3
# """
#
# Example output:
#
# [ 'value1', 'value2', 'value3' ]
#
# It supports ugly formatting, if you're that kind of person:
#
# VARIABLE_NAME=value1   value2 \
#     value3 \
#   value4 value5
#
# It also supports '/' within values, to specify folders, such as:
#
# SUBDIRS = \
#     Core/Memory \
#     Core/Log \
#     Gui \
#     App
#
# However, it does not support the '+=' syntax.
#
def getVariableValuesAsList(variableName, inputConfig):
    regexPattern = variableName + r"\s*=([^\n\\]*(\\[^\S\n]*\n[^\n\\]*)*)"
    match = re.search(regexPattern, inputConfig)
    if match:
        # Matched string (something like " \\\n    value1 \\\n    value2")
        depends = match.groups()[0]

        # Convert to beautiful list (something like ["value1", "value2"])
        return re.findall(r"[/\w']+", depends)
    else:
        # Return an empty list if not found
        return []

# Returns the DEPENDS value parsed from the given string.
def getDepends(inputConfig):
    return getVariableValuesAsList('DEPENDS', inputConfig)

# Returns the SUBDIRS value parsed from the given string.
def getSubdirs(inputConfig):
    return getVariableValuesAsList('SUBDIRS', inputConfig)

def qmakeStringToList(s):
    return re.findall(r"[/\w']+", s)

# Returns as a list the values of the given qmake variable 'variableName',
# defined in the given inputConfig. Returns an empty list if the variable
# is not found. Take into account =, +=, and -=. Though, does not take
# into account order. Only the first '=' is taken into account. All +=
# are processed before all -=.
def getQmakeVariable(variableName, inputConfig, initialValues=[]):
    # Regex pattern
    regexPattern = variableName + r"\s*_SIGN_([^\n\\]*(\\[^\S\n]*\n[^\n\\]*)*)"
    regexPatternEqual      = regexPattern.replace("_SIGN_", r"=")
    regexPatternPlusEqual  = regexPattern.replace("_SIGN_", r"\+=")
    regexPatternMinusEqual = regexPattern.replace("_SIGN_", r"-=")

    # Search for regex
    matchEqual      = re.search(regexPatternEqual, inputConfig)
    matchPlusEqual  = re.findall(regexPatternPlusEqual, inputConfig)
    matchMinusEqual = re.findall(regexPatternMinusEqual, inputConfig)

    # Initial value
    values = initialValues

    # Override if '=' found
    if matchEqual:
        string = matchEqual.groups()[0]
        values = qmakeStringToList(string)

    # Add all '+=' found
    for match in matchPlusEqual:
        string = match[0]
        values.extend(qmakeStringToList(string))

    # Remove all '-=' found
    for match in matchMinusEqual:
        string = match[0]
        strings = qmakeStringToList(string)
        values = [ s for s in values if (s not in strings) ]

    return values


