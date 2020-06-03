#! /bin/bash

set -euo pipefail

CUR_DIR=$(cd "$(dirname "$0")";pwd)

TEMP_MAKE_FILE=${CUR_DIR}/CMakeLists.temp
SRC_PATH=${CUR_DIR}/src
TEST_CLIENTS_SRC_ROOT_PATH=${CUR_DIR}/tests
TEST_CLIENTS=$(ls ${CUR_DIR}/tests)
DYNAMORIO_ROOT_PATH=${CUR_DIR}/dynamorio
DYNAMORIO_EXT_PATH=${DYNAMORIO_ROOT_PATH}/ext
DYNAMORIO_CLIENT_PATH=${DYNAMORIO_ROOT_PATH}/clients

#=============================================================================
echo "Linking source files and cmakefiles.."
# link drcctlib src to dynamorio ext path
rm -rf ${DYNAMORIO_EXT_PATH}/drcctlib
ln -s ${SRC_PATH} ${DYNAMORIO_EXT_PATH}/drcctlib

# link template cmakelists to dynamorio ext path
rm -rf ${DYNAMORIO_EXT_PATH}/CMakeLists.txt
ln -s  ${TEMP_MAKE_FILE}   ${DYNAMORIO_EXT_PATH}/CMakeLists.txt

# link drcctlib test to dynamorio clients path
for TEST_CLIENT in ${TEST_CLIENTS}
do
    rm -rf ${DYNAMORIO_CLIENT_PATH}/${TEST_CLIENT}
    ln -s ${TEST_CLIENTS_SRC_ROOT_PATH}/${TEST_CLIENT} ${DYNAMORIO_CLIENT_PATH}/${TEST_CLIENT}
done

#=============================================================================

echo -e "Prepare build directory and log directory.."
# init logs directory and the name of next make log file
TIMESTAMP=$(date +%s)
BUILD_LOG_PATH=${CUR_DIR}/logs
if [ ! -d ${BUILD_LOG_PATH} ]; then
    mkdir ${BUILD_LOG_PATH}
fi
# MAKE_LOG_FILE=${BUILD_LOG_PATH}/make.log.${TIMESTAMP}
MAKE_LOG_FILE=${BUILD_LOG_PATH}/make.log
# CMAKE_LOG_FILE=${BUILD_LOG_PATH}/cmake.log.${TIMESTAMP}
CMAKE_LOG_FILE=${BUILD_LOG_PATH}/cmake.log

# init build path and go to build path
BUILD_PATH=${CUR_DIR}/build
rm -rf ${BUILD_PATH}
mkdir ${BUILD_PATH}

#=============================================================================
echo -e "Enter \033[34m${BUILD_PATH}\033[0m.."
# enter BUILD_PATH
cd ${BUILD_PATH}

echo -e "Running Cmake..(See \033[34m${CMAKE_LOG_FILE}\033[0m for detail)"
# run cmake
cmake ${DYNAMORIO_ROOT_PATH} >${CMAKE_LOG_FILE} 2>&1

echo -e "Running make..(See \033[34m${MAKE_LOG_FILE}\033[0m for detail)"
# start make
make -j >${MAKE_LOG_FILE} 2>&1

echo -e "\033[32m Build successfully! \033[0m"
echo -e "Leave \033[34m${BUILD_PATH}\033[0m.."
# leave BUILD_PATH
cd ${CUR_DIR}

APPSAMPLES=${CUR_DIR}/appsamples
APPSAMPLES_SRC=${APPSAMPLES}/src
APPSAMPLES_BUILD=${APPSAMPLES}/build
rm -rf ${APPSAMPLES_BUILD}
mkdir ${APPSAMPLES_BUILD}

echo -e "Enter \033[34m${APPSAMPLES}\033[0m.."
cd ${APPSAMPLES}
echo -e "\033[32mStart build app... \033[0m"
# build sample1
g++ -g ${APPSAMPLES_SRC}/sample/sample_cct.cxx -o ${APPSAMPLES_BUILD}/sample_cct
g++ -g ${APPSAMPLES_SRC}/sample/sample_multithread.cxx -o ${APPSAMPLES_BUILD}/sample_multithread -pthread
g++ -g ${APPSAMPLES_SRC}/sample/sample_reuse.cxx -o ${APPSAMPLES_BUILD}/sample_reuse
echo -e "\033[32m Build app successfully! \033[0m"
echo -e "Leave \033[34m${APPSAMPLES}\033[0m.."
cd ${CUR_DIR}

RUN_DIRECTORY_64=${BUILD_PATH}/bin64
RUN_DIRECTORY_32=${BUILD_PATH}/bin32
RUN_DIRECTORY=${RUN_DIRECTORY_32}
if [ ! -d ${RUN_DIRECTORY_64} ]; then
    RUN_DIRECTORY=${RUN_DIRECTORY_32}
else
    RUN_DIRECTORY=${RUN_DIRECTORY_64}
fi
echo -e "\033[32mStart test... \033[0m"
set +euo pipefail
cd ${BUILD_LOG_PATH}

echo -e "\033[32m-----Testing Dynamorio---------\033[0m" && ${RUN_DIRECTORY}/drrun echo hi > /dev/null && echo -e "\033[32m----------PASSED---------\033[0m" || (echo -e "\033[31m----------FAILED---------\033[0m"; exit -1)
echo -e "\033[32m----------Test 1---------\033[0m" && ${RUN_DIRECTORY}/drrun -unsafe_build_ldstex -t drcctlib_all_instr_cct -- ${APPSAMPLES}/build/sample_cct > /dev/null && echo -e "\033[32m----------PASSED---------\033[0m" || (echo -e "\033[31m----------FAILED---------\033[0m"; exit -1)
echo -e "\033[32m----------Test 2---------\033[0m" && ${RUN_DIRECTORY}/drrun -unsafe_build_ldstex -t drcctlib_all_instr_cct -- ${APPSAMPLES}/build/sample_multithread > /dev/null && echo -e "\033[32m----------PASSED---------\033[0m" || (echo -e "\033[31m----------FAILED---------\033[0m"; exit -1)
echo -e "\033[32m----------Test 3---------\033[0m" && ${RUN_DIRECTORY}/drrun -unsafe_build_ldstex -t drcctlib_instr_statistics -- ${APPSAMPLES}/build/sample_cct > /dev/null && echo -e "\033[32m----------PASSED---------\033[0m" || (echo -e "\033[31m----------FAILED---------\033[0m"; exit -1)
echo -e "\033[32m----------Test 4---------\033[0m" && ${RUN_DIRECTORY}/drrun -unsafe_build_ldstex -t drcctlib_instr_statistics -- ${APPSAMPLES}/build/sample_multithread > /dev/null && echo -e "\033[32m----------PASSED---------\033[0m" || (echo -e "\033[31m----------FAILED---------\033[0m"; exit -1)
echo -e "\033[32m----------Test 5---------\033[0m" && ${RUN_DIRECTORY}/drrun -unsafe_build_ldstex -t drcctlib_reuse_distance -- ${APPSAMPLES}/build/sample_reuse > /dev/null && echo -e "\033[32m----------PASSED---------\033[0m" || (echo -e "\033[31m----------FAILED---------\033[0m"; exit -1)
echo -e "\033[32m----------Test 6---------\033[0m" 
cd ${BUILD_LOG_PATH}
echo -e "\033[32m-----fine grind tool-----\033[0m" && ${RUN_DIRECTORY}/drrun -unsafe_build_ldstex -t drcctlib_all_instr_cct_hpc_fmt -- ${APPSAMPLES}/build/sample_cct > /dev/null && echo -e "\033[32m-----PASSED-----\033[0m" || (echo -e "\033[31m-----FAILED-----\033[0m"; exit -1)
cd ${CUR_DIR}
${CUR_DIR}/machine_custom_hpc_fmt.sh sample_cct ${APPSAMPLES}/build/sample_cct $APPSAMPLES_SRC ${BUILD_LOG_PATH}
echo -e "\033[32m----------Finished---------\033[0m"

echo -e "\033[32m----------Test 7---------\033[0m"
cd ${BUILD_LOG_PATH}
echo -e "\033[32m-----fine grind tool-----\033[0m" && ${RUN_DIRECTORY}/drrun -unsafe_build_ldstex -t drcctlib_reuse_distance_hpc_fmt -- ${APPSAMPLES}/build/sample_reuse > /dev/null&& echo -e "\033[32m-----PASSED-----\033[0m" || (echo -e "\033[31m-----FAILED-----\033[0m"; exit -1)
cd ${CUR_DIR}
${CUR_DIR}/machine_custom_hpc_fmt.sh sample_reuse ${APPSAMPLES}/build/sample_reuse $APPSAMPLES_SRC ${BUILD_LOG_PATH}
echo -e "\033[32m----------Finished---------\033[0m"

echo -e "\033[32m*************************************************\033[0m"
echo -e "\033[32m************* ALL TESTS Finished ****************\033[0m"
echo -e "\033[32m*************************************************\033[0m"