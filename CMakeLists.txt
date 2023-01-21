
cmake_minimum_required(VERSION 3.7)

set(MAJOR_VERSION  0)
set(MINOR_VERSION  0)
set(PATCH_VERSION  1)

project(z_ut CXX C)


# 创建UT的项目工作路径
if(DEFINED UT_DIR)
message("already difined work directory: ${UT_DIR}")
else()
message("not difined work directory, using default:  ${CMAKE_CURRENT_SOURCE_DIR}")
set(UT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# 创建UT的编译路径
if(DEFINED BUILD_DIR)
message("already difined build directory: ${BUILD_DIR}")
else()
message("not difined build directory, using default:  ${UT_DIR}/build/")
set(BUILD_DIR ${UT_DIR}/build/)
endif()

# 创建UT的lib输出路径
if(DEFINED LIB_DIR)
message("already difined libarary output directory: ${LIB_DIR}")
else()
message("not difined libarary output directory, using default:  ${BUILD_DIR}/lib/")
set(LIB_DIR ${BUILD_DIR}/lib/)
endif()
set(LIBRARY_OUTPUT_PATH ${LIB_DIR})

# 创建UT的头文件输出路径
if(DEFINED INCLUDE_OUTPUT_DIR)
message("already difined include file output directory: ${INCLUDE_OUTPUT_DIR}")
else()
message("not difined include file output directory, using default:  ${BUILD_DIR}/include/")
set(INCLUDE_OUTPUT_DIR ${BUILD_DIR}/include/)
endif()

# 创建UT的cmake临时文件路径
if(DEFINED TMP_FILES_DIR)
message("already difined cache file directory: ${TMP_FILES_DIR}")
else()
message("not difined cache file directory, using default:  ${UT_DIR}/CMakeCaches/")
set(TMP_FILES_DIR ${UT_DIR}/CMakeCaches/)
endif()




# 设置编译器参数
# set(CMAKE_C_COMPILER "x86_64-linux-gnu-gcc-8")
add_compile_options(-g -Wall -Werror)

# 设置include路径
include_directories(${UT_DIR}/include)

# 设置编译相关源文件
set(UTILS_LIB_SRC   ${UT_DIR}/source/ut_msg.c
                    ${UT_DIR}/source/ut_socket.c
                    ${UT_DIR}/source/ut_io.c
                    ${UT_DIR}/source/ut_string.c
                    ${UT_DIR}/source/ut_hash.c
                    ${UT_DIR}/source/ut_pri_queue.c
                    ${UT_DIR}/source/ut_select.c
                    )

# 创建动态库编译，添加编译器选项（日志等级）
add_library(z_ut SHARED ${UTILS_LIB_SRC})
target_compile_definitions(z_ut PRIVATE ENABLE_LOG=)
target_compile_definitions(z_ut PRIVATE LOG_LEVEL=1)
target_link_libraries(z_ut -lpthread)

# 创建静态库编译，添加编译器选项（日志等级）
add_library(z_ut_st STATIC ${UTILS_LIB_SRC})
target_compile_definitions(z_ut_st PRIVATE ENABLE_LOG=)
target_compile_definitions(z_ut_st PRIVATE LOG_LEVEL=1)
target_link_libraries(z_ut_st -lpthread)

set(UTILS_INC_SRC   ${UT_DIR}/include/ut/ut.h
                    ${UT_DIR}/include/ut/ut_hash.h
                    ${UT_DIR}/include/ut/ut_msg.h
                    ${UT_DIR}/include/ut/ut_socket.h
                    ${UT_DIR}/include/ut/ut_pri_queue.h
                    ${UT_DIR}/include/ut/ut_select.h
                    )

foreach(file_i ${UTILS_INC_SRC})
    file(COPY ${file_i} DESTINATION ${INCLUDE_OUTPUT_DIR})
endforeach(file_i)
