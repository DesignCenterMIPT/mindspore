/**
 * Copyright 2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NNACL_ERRORCODE_H_
#define NNACL_ERRORCODE_H_

typedef enum ErrorCodeCommonEnum {
  NNACL_OK = 0,
  NNACL_ERR = 1,
  NNACL_NULL_PTR,
  NNACL_PARAM_INVALID,
  NNACL_INFER_INVALID,
  NNACL_INPUT_TENSOR_ERROR,
  NNACL_OUTPUT_TENSOR_ERROR,
  NNACL_INPUT_OUTPUT_DATA_TYPE_UNMATCH,
  NNACL_FORMAT_ERROR,
  NNACL_BUFFER_OVERFLOW,
  NNACL_TENSOR_SIZE_INVALID,
  NNACL_UNSUPPORTED_DATA_TYPE,
  NNACL_MALLOC_BUFFER_FAILED,
  NNACL_MALLOC_SIZE_INVALID,
  NNACL_DISABLE_FP16,
  NNACL_ADDN_SHAPE_UNMATCH,
  NNACL_ACTIVATION_TYPE_INVALID,
  NNACL_ARITHMETIC_DATA_TYPE_UNMATCH,
  NNACL_ARITHMETIC_SHAPE_INVALID,
  NNACL_ARITHMETIC_SELF_DATA_TYPE_UNSUPPORT,
  NNACL_BIAS_ADD_SHAPE_NOT_MATCH,
  NNACL_BIAS_ADD_SHAPE_OVERFLOW,
  NNACL_CLIP_DATA_TYPE_INVALID,
  NNACL_CLIP_MINMAX_VALUE_INVALID,
  NNACL_CONCAT_AXIS_INVALID,
  NNACL_CONCAT_F16_INVALID_DATA_TYPE,
  NNACL_CONCAT_F16_OUTPUT_DATA_INVALID,
  NNACL_CONCAT_SHAPE_INVALID,
  NNACL_CONVOLUTION_INPUT_CHANNEL_UNMATCH,
  NNACL_CONVOLUTION_INPUT_HW_OVERFLOW,
  NNACL_CONVOLUTION_KERNEL_HW_OVERFLOW,
  NNACL_CONVOLUTION_OUTPUT_HW_OVERFLOW,
  NNACL_CONVOLUTION_WEIGHT_DATATYPE_INVALID,
  NNACL_CONVOLUTION_WEIGHT_SHAPE_INVALID,
  NNACL_CONVOLUTION_AVX512_UNSUPPORT_FORMAT,
  NNACL_CONVOLUTION_WEIGHT_DATA_INVALID,
  NNACL_CONVOLUTION_BIAS_DATATYPE_INVALID,
  NNACL_ELTWISE_INVALID_MOD,
  NNACL_FILL_DATA_TYPE_INVALID,
  NNACL_GATHER_INDICES_DATA_TYPE_INVALID,
  NNACL_GATHER_AXIS_INVALID,
  NNACL_GATHER_INPUT_TENSOR_INVALID,
  NNACL_GATHER_OUTPUT_TENSOR_INVALID,
  NNACL_GATHER_D_AXIS_INVALID,
  NNACL_GROUP_CONVOLUTION_GROUP_INVALID,
  NNACL_GROUP_CONVOLUTION_SHAPE_INVALID,
  NNACL_GROUP_NORM_NUM_GROUPS_INVALID,
  NNACL_GROUP_NORM_SHAPE_SIZE_INVALID,
  NNACL_GROUP_NORM_FORMAT_INVALID,
  NNACL_SOFTMAX_AXIS_INVALID,
  NNACL_MATMUL_ACT_TYPE_INVALID,
  NNACL_MATMUL_BIAS_INVALID,
  NNACL_REDUCE_AXIS_SIZE_ERROR,
  NNACL_REDUCE_AXES_TENSOR_ERROR,
  NNACL_REDUCE_UNSUPPORTED_DATA_TYPE,
  NNACL_REDUCE_INPUT_SHAPE_SIZE_INVALID,
  NNACL_REDUCE_COEFF_DATA_TYPE_INVALID,
  NNACL_STACK_TENSOR_SHAPE_INVALID,
  NNACL_STRIDED_SLICE_INVALID_SHAPE_SIZE,
  NNACL_STRIDED_SLICE_INVALID_DATA_SIZE,
  NNACL_STRIDED_SLICE_UNSUPPORTED_DATA_TYPE,
  NNACL_STRIDED_SLICE_INVALID_PARALLEL_MOD,
  NNACL_STRIDED_SLICE_UNSUPPORTED_MAX_8D,
  NNACL_TILE_INPUT_SHAPE_INVALID,
  NNACL_TILE_SECOND_INPUT_NUM_INVALID,
  NNACL_TILE_SECOND_INPUT_VALUE_INVALID,
  NNACL_TILE_SECOND_INPUT_DATA_TYPE_INVALID,
  NNACL_TILE_RESIZE_IN_RUNTIME_FAILED,
  NNACL_TRANSPOSE_INSHAPE_OUT_OF_RANGE,
  NNACL_TRANSPOSE_INPUT_TENSOR_NUM_INVALID,
  NNACL_TRANSPOSE_INPUT_TENSOR_VALUD_INVALID,
  NNACL_TRANSPOSE_PERM_DIMS_INVALID,
  NNACL_TRANSPOSE_PERM_TENSOR_INVALID,
  NNACL_TRANSPOSE_PERM_TENSOR_VALUE_INVALID,
  NNACL_TRANSPOSE_PERM_DELETE_DIMENSION_FAILED,
  NNACL_COMMON_END = 9999
} ErrorCodeCommonEnum;

typedef enum ErrorCodeFp32OpEnum {
  NNACL_ERRCODE_OP_FP32_START = 10000,
  NNACL_ERRCODE_STRASSEN_RECURSION_MALLOC,
  NNACL_ERRCODE_REVERSE_MALLOC,
  NNACL_ERRCODE_SQRT_NEGATIVE,
  NNACL_ERRCODE_RSQRT_NEGATIVE,
  NNACL_ERRCODE_RSQRT_NEGATIVE_OR_ZERO,
  NNACL_ERRCODE_LOG_NEGATIVE_OR_ZERO,
  NNACL_ERRCODE_DIVISOR_ZERO,
  NNACL_ERRCODE_INDEX_OUT_OF_RANGE,
  NNACL_ERRCODE_WINOGRAD_GENERATOR_ERROR,
  NNACL_ERRCODE_OP_FP32_END = 19999
} ErrorCodeFp32OpEnum;

typedef enum ErrorCodeFp16OpEnum {
  NNACL_ERRCODE_OP_FP16_START = 20000,
  NNACL_ERRCODE_OP_FP16_WINOGRAD_GENERATOR,
  NNACL_ERRCODE_OP_FP16_END = 29999
} ErrorCodeFp16OpEnum;

typedef enum ErrorCodeUint8OpEnum {
  NNACL_ERRCODE_OP_UINT8_START = 30000,
  NNACL_ERRCODE_OP_UINT8_END = 39999
} ErrorCodeUint8OpEnum;

typedef enum ErrorCodeInt8OpEnum {
  NNACL_ERRCODE_OP_INT8_START = 40000,
  NNACL_ERRCODE_ADD_OVERFLOW,
  NNACL_ERRCODE_MUL_OVERFLOW,
  NNACL_ERRCODE_OP_INT8_END = 49999
} ErrorCodeInt8OpEnums;

#endif  // NNACL_ERRORCODE_H_
