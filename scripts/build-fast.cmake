get_filename_component(SCRIPT_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(PROJECT_ROOT "${SCRIPT_DIR}/.." ABSOLUTE)
set(BUILD_DIR "${PROJECT_ROOT}/build-fast")

message(STATUS "Configuring Sysy with fast input")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -S "${PROJECT_ROOT}"
        -B "${BUILD_DIR}"
        -DSYSY_PRINT_TOKENS=OFF
    RESULT_VARIABLE CONFIG_RESULT
)
if(NOT CONFIG_RESULT EQUAL 0)
    message(FATAL_ERROR "CMake configure failed")
endif()

message(STATUS "Building Sysy")
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}"
    RESULT_VARIABLE BUILD_RESULT
)
if(NOT BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "CMake build failed")
endif()

message(STATUS "Done: ${BUILD_DIR}")
