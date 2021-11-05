set(grpc_USE_STATIC_LIBS OFF)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(grpc_CXXFLAGS "-fstack-protector-all -Wno-uninitialized -Wno-unused-parameter -fPIC -D_FORTIFY_SOURCE=2 -O2")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(grpc_CXXFLAGS "-fstack-protector-all -Wno-maybe-uninitialized -Wno-unused-parameter -D_FORTIFY_SOURCE=2 -O2")
else()
    set(grpc_CXXFLAGS "-fstack-protector-all -Wno-maybe-uninitialized -Wno-unused-parameter -D_FORTIFY_SOURCE=2 -O2 \
  -Dgrpc=mindspore_grpc -Dgrpc_impl=mindspore_grpc_impl -Dgrpc_core=mindspore_grpc_core")
    if(NOT ENABLE_GLIBCXX)
        set(grpc_CXXFLAGS "${grpc_CXXFLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
    endif()
endif()

set(grpc_LDFLAGS "-Wl,-z,relro,-z,now,-z,noexecstack")

if(EXISTS ${protobuf_ROOT}/lib64)
  set(_FINDPACKAGE_PROTOBUF_CONFIG_DIR "${protobuf_ROOT}/lib64/cmake/protobuf")
else()
  set(_FINDPACKAGE_PROTOBUF_CONFIG_DIR "${protobuf_ROOT}/lib/cmake/protobuf")
endif()
message("grpc using Protobuf_DIR : " ${_FINDPACKAGE_PROTOBUF_CONFIG_DIR})

if(EXISTS ${absl_ROOT}/lib64)
  set(_FINDPACKAGE_ABSL_CONFIG_DIR "${absl_ROOT}/lib64/cmake/absl")
else()
  set(_FINDPACKAGE_ABSL_CONFIG_DIR "${absl_ROOT}/lib/cmake/absl")
endif()
message("grpc using absl_DIR : " ${_FINDPACKAGE_ABSL_CONFIG_DIR})

if(EXISTS ${openssl_ROOT})
  set(_CMAKE_ARGS_OPENSSL_ROOT_DIR "-DOPENSSL_ROOT_DIR:PATH=${openssl_ROOT}")
endif()

if(ENABLE_GITEE)
    set(REQ_URL "https://gitee.com/mirrors/grpc/repository/archive/v1.27.3.tar.gz")
    set(MD5 "7564258d5850b7b1d67174cae564c6a4")
else()
    set(REQ_URL "https://github.com/grpc/grpc/archive/v1.27.3.tar.gz")
    set(MD5 "0c6c3fc8682d4262dd0e5e6fabe1a7e2")
endif()

mindspore_add_pkg(grpc
        VER 1.27.3
        LIBS mindspore_grpc++ mindspore_grpc mindspore_gpr mindspore_upb mindspore_address_sorting
        EXE grpc_cpp_plugin
        URL ${REQ_URL}
        MD5 ${MD5}
        PATCHES ${CMAKE_SOURCE_DIR}/third_party/patch/grpc/grpc.patch001
        CMAKE_OPTION -DCMAKE_BUILD_TYPE:STRING=Release
        -DBUILD_SHARED_LIBS=ON
        -DgRPC_INSTALL:BOOL=ON
        -DgRPC_BUILD_TESTS:BOOL=OFF
        -DgRPC_PROTOBUF_PROVIDER:STRING=package
        -DgRPC_PROTOBUF_PACKAGE_TYPE:STRING=CONFIG
        -DProtobuf_DIR:PATH=${_FINDPACKAGE_PROTOBUF_CONFIG_DIR}
        -DgRPC_ZLIB_PROVIDER:STRING=package
        -DZLIB_ROOT:PATH=${zlib_ROOT}
        -DgRPC_ABSL_PROVIDER:STRING=package
        -Dabsl_DIR:PATH=${_FINDPACKAGE_ABSL_CONFIG_DIR}
        -DgRPC_CARES_PROVIDER:STRING=package
        -Dc-ares_DIR:PATH=${c-ares_ROOT}/lib/cmake/c-ares
        -DgRPC_SSL_PROVIDER:STRING=package
        ${_CMAKE_ARGS_OPENSSL_ROOT_DIR}
        )

include_directories(${grpc_INC})

add_library(mindspore::grpc++ ALIAS grpc::mindspore_grpc++)

# link other grpc libs
target_link_libraries(grpc::mindspore_grpc++ INTERFACE grpc::mindspore_grpc grpc::mindspore_gpr grpc::mindspore_upb
  grpc::mindspore_address_sorting)

# modify mindspore macro define
add_compile_definitions(grpc=mindspore_grpc)
add_compile_definitions(grpc_impl=mindspore_grpc_impl)
add_compile_definitions(grpc_core=mindspore_grpc_core)

function(ms_grpc_generate c_var h_var)
    if(NOT ARGN)
        message(SEND_ERROR "Error: ms_grpc_generate() called without any proto files")
        return()
    endif()

    set(${c_var})
    set(${h_var})

    foreach(file ${ARGN})
        get_filename_component(abs_file ${file} ABSOLUTE)
        get_filename_component(file_name ${file} NAME_WE)
        get_filename_component(file_dir ${abs_file} PATH)
        file(RELATIVE_PATH rel_path ${CMAKE_CURRENT_SOURCE_DIR} ${file_dir})

        list(APPEND ${c_var} "${CMAKE_BINARY_DIR}/proto/${file_name}.pb.cc")
        list(APPEND ${h_var} "${CMAKE_BINARY_DIR}/proto/${file_name}.pb.h")
        list(APPEND ${c_var} "${CMAKE_BINARY_DIR}/proto/${file_name}.grpc.pb.cc")
        list(APPEND ${h_var} "${CMAKE_BINARY_DIR}/proto/${file_name}.grpc.pb.h")

        add_custom_command(
                OUTPUT "${CMAKE_BINARY_DIR}/proto/${file_name}.pb.cc"
                "${CMAKE_BINARY_DIR}/proto/${file_name}.pb.h"
                "${CMAKE_BINARY_DIR}/proto/${file_name}.grpc.pb.cc"
                "${CMAKE_BINARY_DIR}/proto/${file_name}.grpc.pb.h"
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/proto"
                COMMAND protobuf::protoc --version
                COMMAND protobuf::protoc -I${file_dir} --cpp_out=${CMAKE_BINARY_DIR}/proto
                        --grpc_out=${CMAKE_BINARY_DIR}/proto
                        --plugin=protoc-gen-grpc=$<TARGET_FILE:grpc::grpc_cpp_plugin> ${abs_file}
                DEPENDS protobuf::protoc grpc::grpc_cpp_plugin ${abs_file}
                COMMENT "Running C++ gRPC compiler on ${file}" VERBATIM)
    endforeach()

    set_source_files_properties(${${c_var}} ${${h_var}} PROPERTIES GENERATED TRUE)
    set(${c_var} ${${c_var}} PARENT_SCOPE)
    set(${h_var} ${${h_var}} PARENT_SCOPE)
endfunction()
