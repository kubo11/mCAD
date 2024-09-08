file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE mge nlohmann_json::nlohmann_json)

if (WIN32)
    set(BIN_NAME ${PROJECT_NAME}.exe)
    set(BIN_PATH ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
else ()
    set(BIN_NAME ${PROJECT_NAME})
    set(BIN_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif ()

add_custom_target(copy_binary ALL
        COMMAND ${CMAKE_COMMAND} -E copy ${BIN_PATH}/${BIN_NAME} ${BUILD_ROOT}/${BIN_NAME})

add_dependencies(copy_binary ${PROJECT_NAME})