# This file is part of VPaint, a vector graphics editor.
#
# Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
#
# The content of this file is MIT licensed. See COPYING.MIT, or this link:
#   http://opensource.org/licenses/MIT

# Basic Qt configuration
TEMPLATE = subdirs

SUBDIRS += \
    Third/GLEW \
    Gui

Gui.depends = Third/GLEW
