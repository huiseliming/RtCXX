cmake_minimum_required(VERSION 3.20)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
else()
    message(FATAL_ERROR "NOT FIND ENVIRONMENT VARIABLE VCPKG_ROOT")
endif()
if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
elseif(UNIX)
    set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")
elseif(APPLE)
    set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "")
else()
    message(FATAL_ERROR "NOT SUPPORTED VCPKG_TARGET_TRIPLET")
endif(WIN32)

message("CMAKE_TOOLCHAIN_FILE: " ${CMAKE_TOOLCHAIN_FILE})
message("VCPKG_TARGET_TRIPLET: " ${VCPKG_TARGET_TRIPLET})

project(RtCXX LANGUAGES C CXX)

message("CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})
message("CMAKE_CXX_SIMULATE_ID: " ${CMAKE_CXX_SIMULATE_ID})
message("CMAKE_CXX_COMPILER_FRONTEND_VARIANT: " ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})

if(WIN32 AND NOT ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

if(WIN32)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(PlatformName "x86")
else()
    set(PlatformName "x64")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY        ${CMAKE_BINARY_DIR}/${PlatformName}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY        ${CMAKE_BINARY_DIR}/${PlatformName}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY        ${CMAKE_BINARY_DIR}/${PlatformName}/lib)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

add_subdirectory(third_party)

file(GLOB_RECURSE RtCXXSource src/*.h src/*.cpp)
file(GLOB_RECURSE RtCXXTestSource test/*.h test/*.cpp)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR} 
  PREFIX / 
  FILES ${RtCXXSource} ${RtCXXTestSource}
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
include(rtcxx-generator)
generate_translation_unit_file(RtCXX RtCXXSource)
list(APPEND RtCXXSource "${CMAKE_BINARY_DIR}/RtCXX.rtcxx.cpp")

add_library(RtCXX SHARED ${RtCXXSource})
message("rtcxx-generator -p=${CMAKE_BINARY_DIR} ${RtCXXSource} --module=RtCXX")
add_custom_target(RtCXX-rtcxx-generator
    ALL
    COMMAND rtcxx-generator -p=${CMAKE_BINARY_DIR} ${RtCXXSource} --module=RtCXX
    BYPRODUCTS ${CMAKE_BINARY_DIR}/RtCXX.rtcxx.cpp
    COMMENT "working for rtcxx-generator ..."
)
add_dependencies(RtCXX RtCXX-rtcxx-generator)

include(GenerateExportHeader)
generate_export_header(RtCXX
    EXPORT_MACRO_NAME RTCXX_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/src/RtCXX_export.h
)

target_include_directories(RtCXX PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:src>
)

set(Boost_NO_WARN_NEW_VERSIONS 1)
#set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS filesystem system serialization log)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(RtCXX PUBLIC ${Boost_LIBRARIES})
target_compile_definitions(RtCXX PUBLIC -DBOOST_USE_WINAPI_VERSION=BOOST_WINAPI_VERSION_WIN7)

target_link_libraries(RtCXX PUBLIC angelscript)
target_include_directories(RtCXX PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/angelscript_2.36.0/sdk/angelscript/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/angelscript_2.36.0/sdk/add_on>
)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(RtCXX PRIVATE nlohmann_json::nlohmann_json)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(RtCXX PUBLIC fmt::fmt)

find_package(glm CONFIG REQUIRED)
target_link_libraries(RtCXX PUBLIC glm::glm)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

generate_translation_unit_file(RtCXXTest RtCXXTestSource)
list(APPEND RtCXXTestSource "${CMAKE_BINARY_DIR}/RtCXXTest.rtcxx.cpp")

add_executable(TestRtCXX ${RtCXXTestSource})
message("rtcxx-generator -p=${CMAKE_BINARY_DIR} ${RtCXXTestSource} --module=RtCXX")
add_custom_target(TestRtCXX-rtcxx-generator
    ALL
    COMMAND rtcxx-generator -p=${CMAKE_BINARY_DIR} ${RtCXXTestSource} --module=TestRtCXX
    BYPRODUCTS ${CMAKE_BINARY_DIR}/TestRtCXX.rtcxx.cpp
    COMMENT "working for rtcxx-generator ..."
)
add_dependencies(TestRtCXX TestRtCXX-rtcxx-generator)


find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(TestRtCXX PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
target_link_libraries(TestRtCXX PUBLIC RtCXX)
