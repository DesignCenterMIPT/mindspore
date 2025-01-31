#!/usr/bin/env bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
###### Environment variable settings, need to set according to your own device ######

# ASCEND_OPP_PATH: The installation path of the OPP package, where "/usr/local/Ascend/opp" is the
#                  default installation path. If user defines the installation path, please modify it.
# Uncomment and modify it when you specified installation path of OPP.
# export ASCEND_OPP_PATH=/usr/local/Ascend/opp

# ASCEND_AICPU_PATH: The installation path of the AICPU package, where "/usr/local/Ascend" is the
#                  default installation path. If user defines the installation path, please modify it.
# Uncomment and modify it when you specified installation path of AICPU.
#export ASCEND_AICPU_PATH=/usr/local/Ascend
#export ASCEND_AICPU_PATH=/home/lll/Ascend/latest

# ASCEND_TENSOR_COMPILER_INCLUDE: The path of the header file of the Compiler package, where "/usr/local/Ascend/compiler/include" is the
#                                 default installation path. If user defines the installation path, please modify it.
# Uncomment and modify it when you specified installation path of Compiler.
# export ASCEND_TENSOR_COMPILER_INCLUDE=/usr/local/Ascend/compiler/include

# TOOLCHAIN_DIR: The path of the cross compilation tool, where "/usr/local/Ascend/toolkit/toolchain/hcc" is the
#                default installation path. If user defines the installation path, please modify it.
# Uncomment and modify it when you specified installation path of toolkit.
# export TOOLCHAIN_DIR=/usr/local/Ascend/toolkit/toolchain/hcc

# AICPU_KERNEL_TARGET: The name of the custom operator kernels library, which "cust_aicpu_kernels" is the
#                      default installation name. If user defines the installation name, please modify it.
# Uncomment and modify it when you specified the installation name of custom operator kernels library.
#export AICPU_KERNEL_TARGET=cust_aicpu_kernels

# The SYSTEM_INFO flag is the name of the form of the operator package generated by the compiler, which can be customized by the user, E.g:
#   a. If the operating system version is CentOS and the architecture is aarch64, it can be set to centos_aarch64, and the operator package name generated by compilation is custom_opp_centos_aarch64.run.
#   b. If the operating system version is CentOS and the architecture is x86_64, it can be set to centos_x86_64, and the name of the operator package generated by compilation is custom_opp_centos_x86_64.run.
#   c. If the SYSTEM_INFO environment variable is not set, the default value is used: ubuntu_x86_64, and the operator package name generated by compilation is custom_opp_ubuntu_x86_64.run.
# Uncomment and modify it when you need to specify os and architecture.
# export SYSTEM_INFO=centos_x86_64

# The version of soc.
# export AICPU_SOC_VERSION=Ascend910

###### The following logic can be used without modification ######

# Indicates the name of the supplier to which the operator belongs. This field can be customized.
vendor_name=mslite
if [[ "$vendor_name" = "" ]] || [[ ! "$vendor_name" ]]; then
  vendor_name=mslite
fi
export OPP_CUSTOM_VENDOR=$vendor_name

# parse input parameters
clean=n
while true
do
  case "$1" in
  -h | --help)
  echo "  --help                show the help info of using this shell"
  echo "  --kernel_so_name=*    so name of aicpu custom operators "
  echo "  --toolchain_dir=*     specify cross compile toolchain directory"
  echo "  clean                 clean compile result directory"
  exit 0
  ;;
  --so_name=*)
  so_name=`echo ${1} | cut -d"=" -f2`
  length=${#so_name}
  aicpu_target_name=${so_name:3:($length - 6)}
  export AICPU_KERNEL_TARGET=$aicpu_target_name
  echo "aicpu_target_name=${aicpu_target_name}"
  shift
  ;;
  clean)
  clean=y
  shift
  ;;
  --toolchain_dir=*)
  tool_dir=`echo ${1} | cut -d"=" -f2`
  export TOOLCHAIN_DIR=$tool_dir
  echo "specified TOOLCHAIN_DIR=$TOOLCHAIN_DIR"
  shift
  ;;
  -*)
  echo "  --help                show the help info of using this shell"
  echo "  --toolchain_dir=*     specify cross compile toolchain directory"
  echo "  --so_name=*           so name of aicpu custom operators generated by compilation"
  echo "  clean                 clean compile result directory"
  exit 0
  ;;
  *)
  break
  ;;
  esac
done

# The following logic can be used without modification ######
log() {
  cur_date=`date +"%Y-%m-%d %H:%M:%S"`
  echo "[$cur_date] "$1
}

project_path=$(cd "$(dirname "$0")"; pwd)
# clean compile result
if [ "x$clean" == "xy" ] 2>/dev/null; then
  rm -rf $project_path/caffe.proto 2>/dev/null
  rm -rf $project_path/caffe.proto.origin 2>/dev/null
  rm -rf $project_path/build_out 2>/dev/null
  rm -rf $project_path/framework/caffe_plugin/proto/caffe 2>/dev/null
  log "[INFO] Clean successfully."
  exit 0
fi
#vendor_name output script/install.sh
vendor_name_output() {
  scripts_file=$PWD/$1
  found_vendor_name_field=$(grep -e "vendor_name=" "$scripts_file")
  found_vendor_name=$(grep -e "vendor_name=" "$scripts_file" | cut --only-delimited -d"=" -f2-)
  if [[ $found_vendor_name_field = "" ]]; then
    sed -i "1 a vendor_name=$vendor_name" $scripts_file
  elif [ $found_vendor_name_field != "" ] && [ $found_vendor_name != $vendor_name ]; then
    sed -i "s/$found_vendor_name_field/vendor_name=$vendor_name/g" $scripts_file
  fi
}

vendor_name_output scripts/install.sh
vendor_name_output scripts/upgrade.sh

if [ ! "x$AICPU_SOC_VERSION" = "xLMIX" ];then
  # if specified cross compile toolchain directory is not exist, log error and exit
  if [[ ! -z "$TOOLCHAIN_DIR" ]] && [[ ! -d "$TOOLCHAIN_DIR" ]];then
      log "[ERROR] Specified cross compile toolchain directory is not exist"
      exit 1
  fi
fi

# set aicpu kernel implement compiler target, default to be custom_cpu_kernels
if [[ -z "${aicpu_target_name}" ]] && [[ -z "${AICPU_KERNEL_TARGET}" ]]; then
    log "[INFO] no so_name is specified, use default compile target: libcust_aicpu_kernels.so!"
    aicpu_target_name="cust_aicpu_kernels"
    export AICPU_KERNEL_TARGET=$aicpu_target_name
    #exit 1
fi

chmod -R 755 $project_path/cmake/util/
mkdir -p $project_path/build_out

###### Environment variable settings, need to set according to your own device ######

# ASCEND_TENSOR_COMPILER_INCLUDE: The path of the header file of the Compiler package, where "/usr/local/Ascend/compiler/include" is the
#                                 default installation path. If user defines the installation path, please modify it.
# Uncomment and modify it when you specified installation path of Compiler.
if [[ -z "${ASCEND_TENSOR_COMPILER_INCLUDE}" ]]; then
    if [[ -d "/usr/local/Ascend/compiler/include" ]];then
        export ASCEND_TENSOR_COMPILER_INCLUDE=/usr/local/Ascend/compiler/include
    else
        log "[ERROR] ENV ASCEND_TENSOR_COMPILER_INCLUDE is not set"
        exit 1
    fi
else
    if [[ ! -d "${ASCEND_TENSOR_COMPILER_INCLUDE}" ]];then
        log "[ERROR] ENV ASCEND_TENSOR_COMPILER_INCLUDE=${ASCEND_TENSOR_COMPILER_INCLUDE} dir is not exist"
        exit 1
    fi
fi

# STEP 1, Check plugin files and proto files
# Check for caffe_plugin files, If there is no plugin file, do not compile caffe proto.
caffe_plugin_cpp_files_num=$(ls $project_path/framework/caffe_plugin/*.cc 2> /dev/null | wc -l)
if [[ "$caffe_plugin_cpp_files_num" == 0 ]];then
  log "[INFO] No caffe plugin files."
fi  # endif compile caffe proto

# STEP 2, CMake and make
cd $project_path/build_out
rm -rf *.run
log "[INFO] Cmake begin."

if [ "x$AICPU_SOC_VERSION" = "xLMIX" ];then
     CMAKE_ARGS="-DLMIX=TRUE"
     cmake $CMAKE_ARGS ..
else
  if [ "x$AICPU_SOC_VERSION" = "xAscend310RC" ];then
    CMAKE_ARGS="-DMINRC=TRUE"
    cmake $CMAKE_ARGS ..
  else 
    cmake ..
  fi
fi
if [ $? -ne 0 ]; then
  log "[ERROR] Please check cmake result."
  exit 1
fi

log "[INFO] Make begin."
make -j
if [ $? -ne 0 ]; then
  log "[ERROR] Please check make result."
  exit 1
fi

log "[INFO] Generate akg custom op files."
ai_core_dir=${project_path}/build_out/makepkg/packages/vendors/${vendor_name}/op_impl/ai_core
config_dir=${ai_core_dir}/tbe/config
dsl_dir=${ai_core_dir}/tbe/${vendor_name}_impl
python3 $project_path/akg/gen_custom_op_files.py ${config_dir} ${dsl_dir}

log "[INFO] Build successfully, the target is in $project_path/build_out."
