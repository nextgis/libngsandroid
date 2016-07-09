################################################################################
#  Project: libngstore
#  Purpose: NextGIS store and visualisation support library
#  Author: Dmitry Baryshnikov, dmitry.baryshnikov@nextgis.com
#  Author: NikitaFeodonit, nfeodonit@yandex.com
#  Language: C/C++
################################################################################
#  GNU Lesser General Public Licens v3
#
#  Copyright (c) 2016 NextGIS, <info@nextgis.com>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

option (BUILD_SHARED_LIBS "Build as dynamic library" ON)

set(HHEADERS
    StdoutsToLogcatRedirector.h
)

set(HHEADER_PRIV
)

set(CSOURCES
    NgsCoreAndroid.cpp
    StdoutsToLogcatRedirector.cpp
)

if(UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffunction-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wl,--strip-all")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ffunction-sections -Wl,--gc-sections -fno-asynchronous-unwind-tables -Wl,--strip-all")
endif()

set(LIB_NAME)
if(BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
    set(LIB_NAME ${PROJECT_NAME})
else()
    set(LIB_TYPE STATIC)
    set(LIB_NAME ${PROJECT_NAME}static)
endif()

add_definitions(-DNGM_ABI="Android ${ANDROID_ABI}")

add_library(${LIB_NAME} ${LIB_TYPE} ${CSOURCES} ${HHEADERS} ${HHEADER_PRIV})

if(BUILD_SHARED_LIBS)
    set (LIB_VERSION ${VERSION})
    set_target_properties (${LIB_NAME} PROPERTIES VERSION ${LIB_VERSION})
    set_target_properties (${LIB_NAME} PROPERTIES SOVERSION ${SOVERSION})
    set_target_properties (${LIB_NAME} PROPERTIES DEFINE_SYMBOL NGSTOR_EXPORTS)
    if (NGSTORE_PROJECT_NAME)
        add_dependencies(${LIB_NAME} ${NGSTORE_PROJECT_NAME})
        target_link_libraries(${LIB_NAME} ${NGSTORE_PROJECT_NAME})
    endif()
else()
    set_target_properties (${LIB_NAME} PROPERTIES DEFINE_SYMBOL NGSTOR_STATIC)
    if (NGSTORE_PROJECT_NAME)
        add_dependencies(${LIB_NAME} ${NGSTORE_PROJECT_NAME}static)
        target_link_libraries(${LIB_NAME} ${NGSTORE_PROJECT_NAME}static)
    endif()
endif()

set_property(TARGET ${LIB_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${LIB_NAME} PROPERTY C_STANDARD 11)

#target_link_extlibraries(${LIB_NAME})
target_link_libraries(${LIB_NAME} android log jnigraphics EGL GLESv2)

set(TARGET_LINK_LIB ${TARGET_LINK_LIB} ${LIB_NAME} PARENT_SCOPE)
set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${LIB_NAME} PARENT_SCOPE)
set(LINK_SEARCH_PATHS ${LINK_SEARCH_PATHS} PARENT_SCOPE)
set(WITHOPT ${WITHOPT} PARENT_SCOPE)
set(EXPORTS_PATHS ${EXPORTS_PATHS} PARENT_SCOPE)

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS ${LIB_NAME}
        EXPORT ${LIB_NAME}
        RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT libraries
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT libraries
        LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT libraries
    )
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL)
    install(FILES ${HHEADERS} DESTINATION "${INSTALL_INC_DIR}"
        COMPONENT headers)
endif()
