# **********************************************************
# Copyright (c) 2020 Xuhpclab. All rights reserved.
# Licensed under the MIT License.
# See LICENSE file for more information.
#  **********************************************************

cmake_minimum_required(VERSION 3.17)
include(../../make/policies.cmake NO_POLICY_SCOPE)
set(CMAKE_CXX_STANDARD 17)
find_package(CUDAToolkit REQUIRED)

include(CheckLanguage)
check_language(CUDA)
if (NOT CMAKE_CUDA_COMPILER)
    if (${CUDAToolkit_NVCC_EXECUTABLE})
        set(ENV{CUDACXX} ${CUDAToolkit_NVCC_EXECUTABLE})
    elseif (UNIX AND EXISTS "/usr/local/cuda/bin/nvcc")
        set(ENV{CUDACXX} "/usr/local/cuda/bin/nvcc")
        message(WARNING "CMAKE_CUDA_COMPILER guessed: " $ENV{CUDACXX} "\n"
                "Please fix your cuda installation: https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html#mandatory-post")
    endif ()
endif ()
enable_language(CUDA)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)



set(SANITIZER_PATH ${CUDAToolkit_TARGET_DIR}/compute-sanitizer)
include_directories(${SANITIZER_PATH}/include/ ${CUDAToolkit_TARGET_DIR}/include/)
message(INFO ${SANITIZER_PATH} ${CUDAToolkit_NVCC_EXECUTABLE})
if (UNIX)
  add_compile_options(-Wno-deprecated)
  add_compile_options(-Wno-unused-result)
  add_compile_options(-Wno-unused-variable)
  add_compile_options(-Wno-unused-local-typedefs)
  add_compile_options(-Wno-unused-function)
  add_compile_options(-Werror=sign-compare)
  add_compile_options(-Werror=narrowing)

  if (DEBUG)
    add_compile_options(-g3)
  endif (DEBUG)
endif (UNIX)

# add third-party libraries

add_library(drcctlib_gpupunk SHARED
  drcctlib_gpupunk.cpp gpupunk_api.cpp )

configure_DynamoRIO_client(drcctlib_gpupunk)
use_DynamoRIO_extension(drcctlib_gpupunk drcctlib)
place_shared_lib_in_lib_dir(drcctlib_gpupunk)

add_dependencies(drcctlib_gpupunk api_headers)

# Provide a hint for how to use the client
if (NOT DynamoRIO_INTERNAL OR NOT "${CMAKE_GENERATOR}" MATCHES "Ninja")
  add_custom_command(TARGET drcctlib_gpupunk
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E echo "Usage: pass to drconfig or drrun: -t drcctlib_gpupunk"
    VERBATIM)
endif ()

install_target(drcctlib_gpupunk ${INSTALL_CLIENTS_LIB})

set(INSTALL_CCTTEST_CONFIG ${INSTALL_CLIENTS_BASE})

function (write_config_file dst bindir libdir)
  file(WRITE  ${dst} "# drcctlib_gpupunk tool config file\n")
  file(APPEND ${dst} "CLIENT_REL=${libdir}/${LIB_PFX}drcctlib_gpupunk${LIB_EXT}\n")
endfunction ()

if (X64)
  set(CONFIG_INSTALL ${PROJECT_BINARY_DIR}/drcctlib_gpupunk.drrun64)
  set(CONFIG_BUILD ${PROJECT_BINARY_DIR}/tools/drcctlib_gpupunk.drrun64)
else (X64)
  set(CONFIG_INSTALL ${PROJECT_BINARY_DIR}/drcctlib_gpupunk.drrun32)
  set(CONFIG_BUILD ${PROJECT_BINARY_DIR}/tools/drcctlib_gpupunk.drrun32)
endif (X64)

set(BUILD_CLIENTS_BIN clients/${INSTALL_BIN})
set(BUILD_CLIENTS_LIB clients/${INSTALL_LIB})

write_config_file(${CONFIG_INSTALL} ${INSTALL_CLIENTS_BIN} ${INSTALL_CLIENTS_LIB})
write_config_file(${CONFIG_BUILD} ${BUILD_CLIENTS_BIN} ${BUILD_CLIENTS_LIB})

DR_install(FILES "${CONFIG_INSTALL}" DESTINATION ${INSTALL_CCTTEST_CONFIG})
register_tool_file("drcctlib_gpupunk")

##################################################
# Documentation

