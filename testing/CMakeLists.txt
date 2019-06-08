# - List of test programs:
set(FalaiseAnalogSignalBuilderPlugin_TESTS
  test_version.cxx
  test_calo_signal_generator_driver.cxx
  test_tracker_signal_generator_driver.cxx
  test_drivers.cxx
  test_flasb.cxx
  test_analog_signal_builder_module.cxx
 )

# # - Use C++11
# set(CMAKE_CXX_FLAGS "-std=c++11")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(_FalaiseAnalogSignalBuilderPlugin_TEST_ENVIRONMENT
  "FALAISE_ASB_RESOURCE_DIR=${PROJECT_SOURCE_DIR}/resources/modules/AnalogSignalBuilder"
  "FALAISE_ASB_TESTING_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
  "FALAISE_ASB_SYS_LOGGING=trace"
  )

message(STATUS "[DEBUG] TEST_ENVIRONMENT= ${_FalaiseAnalogSignalBuilderPlugin_TEST_ENVIRONMENT}")
foreach(_testsource ${FalaiseAnalogSignalBuilderPlugin_TESTS})
  get_filename_component(_testname ${_testsource} NAME_WE)
  set(_testname "falaise-asb-plugin-${_testname}")
  add_executable(${_testname} ${_testsource})
  target_link_libraries(${_testname}
    Falaise_AnalogSignalBuilder
    Falaise::Falaise)
  # - On Apple, ensure dynamic_lookup of undefined symbols
  if(APPLE)
    set_target_properties(${_testname} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  endif()

  add_test(NAME ${_testname} COMMAND ${_testname})
  set_property(TEST ${_testname}
    APPEND PROPERTY ENVIRONMENT ${_FalaiseAnalogSignalBuilderPlugin_TEST_ENVIRONMENT}
    )
  # - For now, dump them into the testing output directory
  set_target_properties(${_testname}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fl-asb-tests/
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fl-asb-tests/
    )
endforeach()

# end of CMakeLists.txt
