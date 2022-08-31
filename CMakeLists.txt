
cmake_minimum_required(VERSION 3.7)

set(MAJOR_VERSION  0)
set(MINOR_VERSION  0)
set(PATCH_VERSION  1)

project(chatroom CXX C)

set(PROJECT_DIR     ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR       ${PROJECT_DIR}/build/)
set(TMP_FILES_DIR   ${BUILD_DIR}/CMakeCaches/)
set(EXECUTABLE_OUTPUT_PATH ${BUILD_DIR}/exec)
set(LIBRARY_OUTPUT_PATH ${BUILD_DIR}/lib)

link_directories(${BUILD_DIR}/lib)

set(CMAKE_C_COMPILER "x86_64-linux-gnu-gcc-8")
add_compile_options(-g -Wall -Werror)


include_directories(${PROJECT_DIR}/include)

set(UTILS_LIB_SRC   ${PROJECT_DIR}/source/ut_msg.c
                    ${PROJECT_DIR}/source/ut_socket.c
                    ${PROJECT_DIR}/source/utils.c
                    ${PROJECT_DIR}/source/ut_hash.c
                    ${PROJECT_DIR}/source/ut_pri_queue.c
                    ${PROJECT_DIR}/source/ut_select.c
                    )


add_library(z_ut SHARED ${UTILS_LIB_SRC})
target_compile_definitions(z_ut PRIVATE ENABLE_LOG=)
target_compile_definitions(z_ut PRIVATE LOG_LEVEL=1)
target_link_libraries(z_ut -lpthread)