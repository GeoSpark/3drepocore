#  Copyright (C) 2014 3D Repo Ltd
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

# http://qt-project.org/doc/qt-5/qmake-variable-reference.html
# http://google-styleguide.googlecode.com/svn/trunk/cppguide.html

include(header.pri)
include(boost.pri)
include(assimp.pri)
include(mongo.pri)

#-------------------------------------------------------------------------------

QT  -= core gui

TEMPLATE = lib

CONFIG += build_all #debug_and_release
CONFIG += c++11

DEFINES += REPO_CORE_LIBRARY
win32:DEFINES += _WIN32

unix {
    target.path = /usr/lib
    INSTALLS += target
    LIBS += -lboost_system -lboost_thread -lboost_filesystem
}

HEADERS +=  src/repocore.h\
            src/repologger.h \
            src/repocoreglobal.h \
            src/assimpwrapper.h \
            src/mongoclientwrapper.h \
            src/graph/repo_bounding_box.h \
            src/graph/repo_graph_abstract.h \
            src/graph/repo_graph_history.h \
            src/graph/repo_graph_scene.h \
            src/graph/repo_node_abstract.h \
            src/graph/repo_node_camera.h \
            src/graph/repo_node_material.h \
            src/graph/repo_node_mesh.h \
            src/graph/repo_node_revision.h \
            src/graph/repo_node_reference.h \
            src/graph/repo_node_metadata.h \
            src/graph/repo_node_texture.h \
            src/graph/repo_node_transformation.h \
            src/primitives/repo_user.h \
            src/primitives/repo_vertex.h \
            src/primitives/repostreambuffer.h \
            src/primitives/repoabstractlistener.h \
            src/primitives/repoabstractnotifier.h \
            src/primitives/reposeverity.h \
            src/conversion/repo_transcoder_bson.h \
            src/conversion/repo_transcoder_string.h \
            src/compute/render.h \

SOURCES +=  src/repocore.cpp \
            src/repologger.cpp \
            src/assimpwrapper.cpp \
            src/mongoclientwrapper.cpp \
            src/graph/repo_bounding_box.cpp \
            src/graph/repo_graph_abstract.cpp \
            src/graph/repo_graph_history.cpp \
            src/graph/repo_graph_scene.cpp \
            src/graph/repo_node_abstract.cpp \
            src/graph/repo_node_camera.cpp \
            src/graph/repo_node_material.cpp \
            src/graph/repo_node_mesh.cpp \
            src/graph/repo_node_revision.cpp \
            src/graph/repo_node_reference.cpp \
            src/graph/repo_node_metadata.cpp \
            src/graph/repo_node_texture.cpp \
            src/graph/repo_node_transformation.cpp \
            src/primitives/repo_user.cpp \
            src/primitives/repo_vertex.cpp \
            src/primitives/repostreambuffer.cpp \
            src/primitives/repoabstractlistener.cpp \
            src/primitives/repoabstractnotifier.cpp \
            src/primitives/reposeverity.cpp \
            src/conversion/repo_transcoder_bson.cpp \
            src/conversion/repo_transcoder_string.cpp \
            src/compute/render.cpp \



