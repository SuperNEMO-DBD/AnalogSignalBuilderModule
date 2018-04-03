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

# - Validate that certain paths are relative, otherwise relocation may fail
foreach(_dir BINDIR LIBDIR INCLUDEDIR DATAROOTDIR)
  if(IS_ABSOLUTE "${CMAKE_INSTALL_${_dir}}")
    message(FATAL_ERROR "Absolute path for CMAKE_INSTALL_${_dir} not allowed")
  endif()
endforeach()

message(STATUS "[DEBUG] CMAKE_INSTALL_BINDIR      = ${CMAKE_INSTALL_BINDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_LIBDIR      = ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_INCLUDEDIR  = ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_DATAROOTDIR = ${CMAKE_INSTALL_DATAROOTDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_FULL_LIBDIR      = ${CMAKE_INSTALL_FULL_LIBDIR}")
message(STATUS "[DEBUG] CMAKE_INSTALL_FULL_DATAROOTDIR = ${CMAKE_INSTALL_FULL_DATAROOTDIR}")

# - Load Custom Modules
# list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake)

set(FalaiseASB_VERSION_MAJOR 0)
set(FalaiseASB_VERSION_MINOR 1)
set(FalaiseASB_VERSION_PATCH 0)
set(FalaiseASB_VERSION_REVISION 0)
set(FalaiseASB_VERSION "${FalaiseASB_VERSION_MAJOR}.${FalaiseASB_VERSION_MINOR}.${FalaiseASB_VERSION_PATCH}")
message( STATUS "[DEBUG] ASB version : ${FalaiseASB_VERSION}")

# Resources directories based on name-version subdirs
set(FalaiseASB_TAG "FalaiseASB-${FalaiseASB_VERSION}")
file(RELATIVE_PATH FalaiseASB_LIBDIR_TO_RESOURCEDIR
  "${CMAKE_INSTALL_FULL_LIBDIR}"
  "${CMAKE_INSTALL_FULL_DATAROOTDIR}/${FalaiseASB_TAG}/resources")
# file(RELATIVE_PATH FalaiseASB_CMAKEDIR_TO_INCLUDEDIR
#   "${CMAKE_INSTALL_FULL_CMAKEDIR}/${FalaiseASB_TAG}"
#   "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
message( STATUS "[DEBUG] FalaiseASB_LIBDIR_TO_RESOURCEDIR : ${FalaiseASB_LIBDIR_TO_RESOURCEDIR}")

find_package(Falaise 3.1)
message( STATUS "[DEBUG] Falaise ${Falaise_VERSION} was found")

############################################################################################
# - AnalogSignalBuilder modules:
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/source/falaise)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/source)
#include_directories(${Falaise_INCLUDE_DIRS})
include_directories(${Bayeux_INCLUDE_DIRS})

# - Prepare files from skelettons:
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/version.h
  @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/resource.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/resource.cc
  @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/flasb.cc.in
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/flasb.cc
  @ONLY)

# - Generate binreloc header/source with mangled names, ensuring first
# character(s) are a valid for a C identifier
string(RANDOM LENGTH 13 MANGLE_BINRELOC_MAIN)
set(MANGLE_BINRELOC "MBFLASB${MANGLE_BINRELOC_MAIN}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/flasb_binreloc.h.in ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/flasb_binreloc.h @ONLY)

# - Headers:
list(APPEND FalaiseAnalogSignalBuilderPlugin_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/flasb_binreloc.h
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/resource.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/flasb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/detail/flasb_sys.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/geiger_analogic_regime.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/base_signal_generator_driver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/analog_signal_builder_module.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/calo_signal_generator_driver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/tracker_signal_generator_driver.h
  )

# - Sources:
list(APPEND FalaiseAnalogSignalBuilderPlugin_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/flasb_binreloc.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/version.cc
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/resource.cc
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/asb/flasb.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/detail/flasb_sys.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/utils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/geiger_analogic_regime.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/base_signal_generator_driver.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/analog_signal_builder_module.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/calo_signal_generator_driver.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/falaise/snemo/asb/tracker_signal_generator_driver.cc
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

target_link_libraries(Falaise_AnalogSignalBuilder PUBLIC Falaise::FalaiseModule)

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
install(DIRECTORY ${PROJECT_SOURCE_DIR}/resources/modules/AnalogSignalBuilder/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/Falaise/resources/modules/AnalogSignalBuilder)

# Test support:
option(FalaiseAnalogSignalBuilderPlugin_ENABLE_TESTING "Build unit testing system for FalaiseAnalogSignalBuilder" ON)
if(FalaiseAnalogSignalBuilderPlugin_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(testing)
endif()

# - end
