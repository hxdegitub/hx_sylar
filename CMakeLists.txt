cmake_minimum_required(VERSION 3.2) 
project(hx_sylar)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)

# set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -o0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror 
# -Wno-unused-function -Wno-builtin-macro-redefined")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS}  -O0  -ggdb -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined ") 
include_directories(.) 
include_directories(/usr/local/include)
link_directories(/usr/local/lib)
find_library(YAMLCPP yaml-cpp)
message("***", ${YAMLCPP}) 
find_package(OpenSSL REQUIRED) 
if(OPENSSL_FOUND) 
include_directories(${OPENSSL_INCLUDE_DIR}) 
endif()
set(LIB_SRC hx_sylar/config.cc hx_sylar/fiber.cc hx_sylar/hook.cc hx_sylar/iomanager.cc hx_sylar/log.cc hx_sylar/mutex.cc hx_sylar/scheduler.cc hx_sylar/thread.cc hx_sylar/timer.cc hx_sylar/util.cc hx_sylar/fd_manager.cc
    hx_sylar/address.cc
    hx_sylar/socket.cc
    hx_sylar/bytearray.cc
    hx_sylar/http/http.cc
    hx_sylar/http/http_parser.cc
    hx_sylar/http/httpclient_parser.cc
    hx_sylar/http/http11_parser.cc
    hx_sylar/stream.cc
    hx_sylar/http/tcp_server.cc
    hx_sylar/stream/socket_stream.cc
    hx_sylar/http/http_session.cc
    hx_sylar/http/http_server.cc
     hx_sylar/http/servlet.cc 
     hx_sylar/http/http_connection.cc
     hx_sylar/mutex.cc
     hx_sylar/uri.cc
)


add_library(hx_sylar SHARED ${LIB_SRC})

force_redefine_file_macro_for_sources(hx_sylar)
#
set(LIB_LIB
    hx_sylar
    pthread
    dl
    ${OPENSSL_LIBRARIES}
    ${YAMLCPP}
)

add_executable(test tests/test.cc)
add_dependencies(test hx_sylar)
force_redefine_file_macro_for_sources(test)
target_link_libraries(test  ${LIB_LIB})



add_executable(test_config tests/test_config.cc)
add_dependencies(test_config hx_sylar)
force_redefine_file_macro_for_sources(test)
target_link_libraries(test_config ${LIB_LIB})


add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread hx_sylar)
force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_thread ${LIB_LIB})


add_executable(test_util tests/test_util.cc)
add_dependencies(test_util hx_sylar)
force_redefine_file_macro_for_sources(test_util)
target_link_libraries(test_util ${LIB_LIB})

add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber hx_sylar)
force_redefine_file_macro_for_sources(test_fiber)
target_link_libraries(test_fiber ${LIB_LIB})

add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler hx_sylar)
force_redefine_file_macro_for_sources(test_scheduler)
target_link_libraries(test_scheduler ${LIB_LIB})


add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager hx_sylar)
force_redefine_file_macro_for_sources(test_iomanager)
target_link_libraries(test_iomanager ${LIB_LIB})


add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook hx_sylar)
force_redefine_file_macro_for_sources(test_hook)
target_link_libraries(test_hook ${LIB_LIB})

add_executable(test_bytearray tests/test_bytearray.cc)
add_dependencies(test_bytearray hx_sylar)
force_redefine_file_macro_for_sources(test_bytearray)
target_link_libraries(test_bytearray ${LIB_LIB})

add_executable(test_http tests/test_http.cc)
add_dependencies(test_http hx_sylar)
force_redefine_file_macro_for_sources(test_http)
target_link_libraries(test_http ${LIB_LIB})

add_executable(test_http_parser tests/test_http_parser.cc)
add_dependencies(test_http_parser hx_sylar)
force_redefine_file_macro_for_sources(test_http_parser)
target_link_libraries(test_http_parser ${LIB_LIB})

add_executable(test_http_server tests/test_http_server.cc)
add_dependencies(test_http_server hx_sylar)
force_redefine_file_macro_for_sources(test_http_server)
target_link_libraries(test_http_server ${LIB_LIB})





SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUT_PUT_PATH${PROJECT_SOURCE_DIR}/bin)

