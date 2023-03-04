# Settings passed on the command line:
#
# BUILD_SHARED_LIBS
# PROJECT_STATIC_RUNTIME
# CBASH_NO_BOOST_ZLIB

##############################
# General Settings
##############################

cmake_minimum_required (VERSION 3.14)
include(FetchContent)
project (CBash)

option(BUILD_SHARED_LIBS "Build a shared library" OFF)
option(PROJECT_STATIC_RUNTIME "Build with static runtime libs (/MT)" ON)

IF (MSVC)
	#set (CMAKE_GENERATOR_TOOLSET "v141" CACHE STRING "Platform Toolset" FORCE)
    #Force static C++ runtime linkage.
    IF (PROJECT_STATIC_RUNTIME)
        FOREACH(flag
            CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG_INIT
            CMAKE_CXX_FLAGS_RELEASE  CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_DEBUG  CMAKE_CXX_FLAGS_DEBUG_INIT)
            STRING(REPLACE "/MD"  "/MT" "${flag}" "${${flag}}")
            SET("${flag}" "${${flag}} /EHsc /MP")
        ENDFOREACH()
    ENDIF ()
ENDIF ()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.2.13
)

FetchContent_MakeAvailable(zlib)

set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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

##############################
# System-Specific Settings
##############################

# Settings when compiling for Windows. Since it's a Windows-only app this is always true, but useful to check for copy/paste into other projects.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions (-DUNICODE -D_UNICODE -DWIN32 -D_WINDOWS)
    IF (BUILD_SHARED_LIBS)
        add_definitions (-DCBASH_DLL)
    ELSE ()
        add_definitions (-DCBASH_STATIC)
    ENDIF ()
ENDIF ()

##############################
# Actual Building
##############################

# Build CBash.
message("Building")
add_library           (CBash STATIC ${CBASH_SRC})
target_link_libraries (CBash PRIVATE zlibstatic)


# Include source and library directories.
target_include_directories ("${PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/cbash"
                     "${CMAKE_CURRENT_SOURCE_DIR}/src"
					 PRIVATE
					 ${ZLIB_INCLUDE_DIR})


enable_testing()

# Build Tests
add_executable(
  CBashTest
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/tests.cpp"
)
target_link_libraries(
  CBashTest
  CBash
)

#include(GoogleTest)
#gtest_discover_tests(CBashTest)      
