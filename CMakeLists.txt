# Settings passed on the command line:
#
# BUILD_SHARED_LIBS
# PROJECT_STATIC_RUNTIME
# CBASH_NO_BOOST_ZLIB

##############################
# General Settings
##############################

cmake_minimum_required (VERSION 3.0)
project (CBash)
include(ExternalProject)

ExternalProject_Add(zlib
                    PREFIX "external"
                    URL "https://zlib.net/zlib-1.2.11.tar.gz"
                    INSTALL_COMMAND "")
ExternalProject_Get_Property(zlib SOURCE_DIR BINARY_DIR)
set (ZLIB_INCLUDE_DIRS ${SOURCE_DIR} ${BINARY_DIR})
message("ZLIB_INCLUDE_DIRS:" ${ZLIB_INCLUDE_DIRS})
IF (MSVC)
    set (ZLIB_LIBRARIES optimized "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}"
                    debug "${BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstaticd${CMAKE_STATIC_LIBRARY_SUFFIX}")
ELSE ()
    set (ZLIB_LIBRARIES "${BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX}")
ENDIF ()

option(BUILD_SHARED_LIBS "Build a shared library" OFF)
option(PROJECT_STATIC_RUNTIME "Build with static runtime libs (/MT)" ON)


file (GLOB_RECURSE CBASH_FONV "${CMAKE_CURRENT_SOURCE_DIR}/src/FalloutNewVegas/*.cpp")
file (GLOB_RECURSE CBASH_OBLIVION "${CMAKE_CURRENT_SOURCE_DIR}/src/Oblivion/*.cpp")
file (GLOB_RECURSE CBASH_SKYRIM "${CMAKE_CURRENT_SOURCE_DIR}/src/Skyrim/*.cpp")
file (GLOB_RECURSE CBASH_SKYRIM_H "${CMAKE_CURRENT_SOURCE_DIR}/src/Skyrim/*.h")
file (GLOB_RECURSE CBASH_SKYBLIVION "${CMAKE_CURRENT_SOURCE_DIR}/src/Skyblivion/*.cpp")

source_group("FONV" FILES ${CBASH_FONV})
source_group("Oblivion" FILES ${CBASH_OBLIVION})
source_group("Skyrim\\Headers" FILES ${CBASH_SKYRIM_H})
source_group("Skyrim\\Source" FILES ${CBASH_SKYRIM})
source_group("Skyblivion" FILES ${CBASH_SKYBLIVION})

set (CBASH_SRC  "${CMAKE_CURRENT_SOURCE_DIR}/src/CBash.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/Collection.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/Common.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/GenericChunks.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/GenericRecord.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/ModFile.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/TES4Record.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/TES4RecordAPI.cpp"
				"${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/Visitors.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/CBash.rc"
                ${CBASH_FONV}
                ${CBASH_OBLIVION}
                ${CBASH_SKYRIM}
                ${CBASH_SKYBLIVION}
                )


add_definitions(-DCBASH_NO_BOOST_ZLIB)


# Include source and library directories.
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/include/cbash"
                     ${ZLIB_INCLUDE_DIRS})


##############################
# System-Specific Settings
##############################

# Settings when compiling for Windows. Since it's a Windows-only app this is always true, but useful to check for copy/paste into other projects.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions (-DUNICODE -D_UNICODE -DNDEBUG -DWIN32 -D_WINDOWS)
    IF (BUILD_SHARED_LIBS)
        add_definitions (-DCBASH_DLL)
    ELSE ()
        add_definitions (-DCBASH_STATIC)
    ENDIF ()
ENDIF ()

IF (MSVC)
	# set (CMAKE_GENERATOR_TOOLSET "v141" CACHE STRING "Platform Toolset" FORCE)
    # Force static C++ runtime linkage.
    IF (PROJECT_STATIC_RUNTIME)
        FOREACH(flag
            CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG_INIT
            CMAKE_CXX_FLAGS_RELEASE  CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_DEBUG  CMAKE_CXX_FLAGS_DEBUG_INIT)
            STRING(REPLACE "/MD"  "/MT" "${flag}" "${${flag}}")
            SET("${flag}" "${${flag}} /EHsc")
        ENDFOREACH()
    ENDIF ()

    set (CMAKE_EXE_LINKER_FLAGS "/SUBSYSTEM:WINDOWS")
ENDIF ()


##############################
# Actual Building
##############################

# Build CBash.
message("Building")
add_library           (CBash STATIC ${CBASH_SRC})
add_dependencies	  (CBash zlib)
target_link_libraries (CBash ${ZLIB_LIBRARIES})
