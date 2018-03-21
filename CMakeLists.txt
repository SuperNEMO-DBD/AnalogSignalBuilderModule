# CMAKE_COMPILER
# - Basic CMake setup
# Check version meets ou requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 3.3)
project(FalaiseAnalogSignalBuilderPlugin)

include(GNUInstallDirs)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(STATUS "${PROJECT_NAME} requires an out-of-source build.")
  message(STATUS "Please remove these files from ${CMAKE_BINARY_DIR} first:")
  message(STATUS "  CMakeCache.txt")
  message(STATUS "  CMakeFiles")
  message(STATUS "Once these files are removed, create a separate directory")
  message(STATUS "and run CMake from there, pointing it to:")
  message(STATUS "  ${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR "in-source build detected")
endif()

# - Load Custom Modules
# list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake)

set(FalaiseASB_VERSION_MAJOR 0)
set(FalaiseASB_VERSION_MINOR 1)
set(FalaiseASB_VERSION_PATCH 0)
set(FalaiseASB_VERSION_REVISION 0)
set(FalaiseASB_VERSION "${FalaiseASB_VERSION_MAJOR}.${FalaiseASB_VERSION_MINOR}.${FalaiseASB_VERSION_PATCH}")
message( STATUS "ASB version : ${FalaiseASB_VERSION}")

# # - ROOT
# # find_package(ROOT 5.34.00 REQUIRED Gui Geom GeomPainter Physics)
# set(ROOT_COMPONENTS Gui Gpad Geom GeomPainter Physics Graf Graf3d)
# find_package(ROOT 5.34.00 QUIET COMPONENTS ${ROOT_COMPONENTS} NO_MODULE)
# find_package(ROOT 5.34.00 REQUIRED ${ROOT_COMPONENTS})

find_package(Falaise 3.1)
message( STATUS "Falaise ${Falaise_VERSION} was found")

############################################################################################
# - AnalogSignalBuilder modules:
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/source/falaise)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/source)
include_directories(${Bayeux_INCLUDE_DIRS})

# - Prepare files from skelettons:
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/version.h
  @ONLY)

# - Headers:
list(APPEND FalaiseAnalogSignalBuilderPlugin_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/base_signal_generator_driver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/analog_signal_builder_module.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/calo_signal_generator_driver.h
  )

# - Sources:
list(APPEND FalaiseAnalogSignalBuilderPlugin_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/version.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/utils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/base_signal_generator_driver.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/analog_signal_builder_module.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/calo_signal_generator_driver.cc
  )

############################################################################################

# Build a dynamic library from our sources
add_library(Falaise_AnalogSignalBuilder SHARED
  ${FalaiseAnalogSignalBuilderPlugin_HEADERS}
  ${FalaiseAnalogSignalBuilderPlugin_SOURCES})
target_include_directories(Falaise_AnalogSignalBuilder
  PUBLIC
    ${PROJECT_SOURCE_DIR}/source
    ${PROJECT_SOURCE_DIR}/source/falaise
  )

target_link_libraries(Falaise_AnalogSignalBuilder PUBLIC Falaise::Falaise)
### target_link_libraries(Falaise_AnalogSignalBuilder FalaiseModule)

# target_compile_features(Falaise_AnalogSignalBuilder PUBLIC ${FALAISE_CXX_COMPILE_FEATURES})
# target_compile_definitions(Falaise_AnalogSignalBuilder PRIVATE ENABLE_BINRELOC)
# target_include_directories(Falaise_AnalogSignalBuilder PUBLIC
#   $<BUILD_INTERFACE:${FALAISE_BUILD_INCLUDEDIR}>
#   $<BUILD_INTERFACE:${PROJECT_BUILD_INCLUDEDIR}/falaise>
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
#   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/falaise>
#   )
# target_link_libraries(Falaise PUBLIC Bayeux::Bayeux)
# target_clang_format(Falaise)

# Apple linker requires dynamic lookup of symbols, so we
# add link flags on this platform
if(APPLE)
  set_target_properties(Falaise_AnalogSignalBuilder
    PROPERTIES LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

# Install it:
install(TARGETS Falaise_AnalogSignalBuilder DESTINATION ${CMAKE_INSTALL_LIBDIR}/Falaise/modules)

# Test support:
option(FalaiseAnalogSignalBuilderPlugin_ENABLE_TESTING "Build unit testing system for FalaiseAnalogSignalBuilder" ON)
if(FalaiseAnalogSignalBuilderPlugin_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(testing)
endif()
