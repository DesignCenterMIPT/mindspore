/**
 * Copyright 2023 Huawei Technologies Co., Ltd
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

#include "src/extendrt/kernel/nnacl/nnacl_lib.h"
#include <unordered_set>
#include <vector>
#include "src/litert/kernel/cpu/nnacl/nnacl_manager.h"
#include "src/common/ops/operator_populate/operator_populate_register.h"

namespace mindspore::kernel {
namespace {
TypeId GetFirstFp32Fp16OrInt8Type(const KernelAttr &attr) {
  if (attr.GetInputSize() == 0) {
    MS_LOG(WARNING) << "in tensor is empty.";
    return kTypeUnknown;
  }
  for (size_t i = 0; i < attr.GetInputSize(); i++) {
    auto dtype = attr.GetInputAttr(i).dtype;
    if (dtype == kObjectTypeTensorType) {
      MS_LOG(WARNING) << "Not support TensorType Tensor now!";
      return kTypeUnknown;
    }
    std::unordered_set<TypeId> type_set = {kNumberTypeFloat32, kNumberTypeFloat16, kNumberTypeInt8,  kNumberTypeInt32,
                                           kNumberTypeBool,    kNumberTypeUInt8,   kObjectTypeString};
    if (type_set.find(dtype) != type_set.end()) {
      return dtype;
    }
  }

  return attr.GetInputAttr(0).dtype;
}
}  // namespace

bool NNAclLib::Support(const PrimitiveType &op_type, const KernelAttr &attr, const Format &format) const {
  if (!MatchFormat(format, NHWC)) {
    MS_LOG(INFO) << "NNACL not support NHWC layout.";
    return false;
  }
  TypeId data_type = GetFirstFp32Fp16OrInt8Type(attr);
  if (data_type == kTypeUnknown) {
    MS_LOG(INFO) << "Get main datatype of kernel failed.";
    return false;
  }
  auto creator = nnacl::KernelRegistry::GetInstance()->Creator({op_type.FBType(), data_type});
  if (creator != nullptr) {
    return true;
  }
  return SupportKernelC(op_type.FBType(), data_type);
}

LiteKernel *NNAclLib::CreateKernel(const KernelSpec &spec, const std::vector<InferTensor *> &inputs,
                                   const std::vector<InferTensor *> &outputs, const InferContext *ctx) const {
  if (!MatchFormat(spec.format, NHWC)) {
    MS_LOG(INFO) << "NNACL only support NHWC layout, but got " << FormatEnumToString(spec.format);
    return nullptr;
  }
  TypeId data_type = GetFirstFp32Fp16OrInt8Type(spec.attr);
  if (data_type == kTypeUnknown) {
    MS_LOG(INFO) << "Get main datatype of kernel failed while creating nnacl kernel.";
    return nullptr;
  }
  auto op_parameter = lite::OperatorPopulateRegistry::GetInstance()->CreatePopulateByOp(spec.primitive);
  if (op_parameter == nullptr) {
    MS_LOG(INFO) << "Populate op-parameter for kernel failed, kernel-type: " << spec.op_type.PBType();
    return nullptr;
  }
  op_parameter->thread_num_ = ctx->thread_num_;
  auto creator = nnacl::KernelRegistry::GetInstance()->Creator({spec.op_type.FBType(), data_type});
  nnacl::NNACLKernel *kernel = nullptr;
  if (creator != nullptr) {
    kernel = creator(op_parameter, inputs, outputs, ctx);
  } else {
    kernel = new (std::nothrow) nnacl::NNACLKernel(op_parameter, inputs, outputs, ctx);
  }
  if (kernel == nullptr) {
    MS_LOG(INFO) << "Create nnacl kernel failed:  " << op_parameter->name_;
    free(op_parameter);
    return nullptr;
  }

  auto ret = kernel->InitKernel(data_type, ctx);
  if (ret != RET_OK) {
    MS_LOG(INFO) << "Init nnacl kernel failed:  " << op_parameter->name_;
    kernel->set_parameter(nullptr);
    delete kernel;
    free(op_parameter);
    return nullptr;
  }
  return kernel;
}

REG_KERNEL_LIB(kNNAclName, NNAclLib);
}  // namespace mindspore::kernel
