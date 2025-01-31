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

#include "transform/acl_ir/acl_helper.h"
#include <set>
#include <string>
#include "include/common/utils/utils.h"
#include "include/transform/graph_ir/types.h"
#include "include/api/data_type.h"
#include "include/common/utils/anfalgo.h"
#include "include/backend/anf_runtime_algorithm.h"
#include "runtime/device/ms_device_shape_transfer.h"
#include "transform/acl_ir/ge_adapter_info.h"
#include "transform/acl_ir/acl_adapter_info.h"

namespace mindspore {
namespace transform {
namespace {
static const HashMap<GeDataType, TypeId> kGeTypeToMsType = {{GeDataType::DT_BOOL, kNumberTypeBool},
                                                            {GeDataType::DT_INT8, kNumberTypeInt8},
                                                            {GeDataType::DT_INT16, kNumberTypeInt16},
                                                            {GeDataType::DT_INT32, kNumberTypeInt32},
                                                            {GeDataType::DT_INT64, kNumberTypeInt64},
                                                            {GeDataType::DT_UINT8, kNumberTypeUInt8},
                                                            {GeDataType::DT_UINT16, kNumberTypeUInt16},
                                                            {GeDataType::DT_UINT32, kNumberTypeUInt32},
                                                            {GeDataType::DT_UINT64, kNumberTypeUInt64},
                                                            {GeDataType::DT_FLOAT16, kNumberTypeFloat16},
                                                            {GeDataType::DT_FLOAT, kNumberTypeFloat32},
                                                            {GeDataType::DT_DOUBLE, kNumberTypeFloat64},
                                                            {GeDataType::DT_STRING, kObjectTypeString},
                                                            {GeDataType::DT_COMPLEX64, kNumberTypeComplex64},
                                                            {GeDataType::DT_COMPLEX128, kNumberTypeComplex128}};

TypeId ConvertGeType(GeDataType type) {
  if (kGeTypeToMsType.count(type) != 0) {
    return kGeTypeToMsType.at(type);
  }
  return kTypeUnknown;
}
}  // namespace

bool AclHelper::CheckDefaultSupportFormat(const string &format) {
  static std::set<std::string> default_support = {kOpFormat_DEFAULT, kOpFormat_ND,    kOpFormat_NCHW,
                                                  kOpFormat_NHWC,    kOpFormat_NDHWC, kOpFormat_NCDHW};
  return default_support.find(format) != default_support.end();
}

bool AclHelper::GetMoreDataTypeSupported(TypeId data_type, const std::string &op_type) {
  if (!AclAdapterManager::GetInstance().CheckAclAdapter(op_type)) {
    return false;
  }
  auto acl_info = AclAdapterManager::GetInstance().GetOpInfo(op_type);
  if (acl_info.precision_mode() == FORCE_FP32) {
    if (data_type != kNumberTypeFloat32 && data_type != kNumberTypeFloat) {
      return false;
    }
    return true;
  }
  if (!acl_info.extra_supported_datatype().empty()) {
    if (std::any_of(acl_info.extra_supported_datatype().begin(), acl_info.extra_supported_datatype().end(),
                    [data_type](GeDataType ge_type) { return ConvertGeType(ge_type) == data_type; })) {
      return true;
    }
  }
  return false;
}

KernelType AclHelper::GetKernelInfoByInputs(const CNodePtr &cnode, const std::shared_ptr<GeAdapterInfo> &info) {
  auto input_supported_dtypes = info->input_supported_dtypes();
  size_t num_real_inputs = common::AnfAlgo::GetInputTensorNum(cnode);
  size_t ms_real_idx = 0;  // index of actual input argument
  for (size_t ms_proto_idx = 0; ms_proto_idx < info->GetNumInputsOfMsOpProto(); ++ms_proto_idx) {
    // skip attribute converted input
    if (NeedCheckAttrToInput(cnode, info->attr_input_map(), ms_proto_idx)) {
      MS_LOG(DEBUG) << "Op prototype input idx:" << ms_proto_idx << " is attr to input, skip check";
      continue;
    }

    if (ms_real_idx >= num_real_inputs) {
      break;
    }

    auto opt_ge_input_info = info->GetOptGeInputByMsInputIndex(ms_proto_idx);
    // skip input which will be converted to attribute, or some extra inputs defined by mindspore, such as AvgPoolGrad
    if (!opt_ge_input_info.has_value()) {
      MS_LOG(DEBUG) << "Unsupported op prototype input idx:" << ms_proto_idx
                    << " of node:" << cnode->fullname_with_scope();
      ms_real_idx += 1;
      continue;
    }

    auto &ge_input_info = opt_ge_input_info.value();
    auto base_type = common::AnfAlgo::GetPrevNodeOutputInferDataType(cnode, ms_real_idx);
    if (!std::any_of(
          input_supported_dtypes[ms_proto_idx].begin(), input_supported_dtypes[ms_proto_idx].end(),
          [base_type, ge_input_info](const ::ge::DataType ge_type) { return ConvertGeType(ge_type) == base_type; })) {
      if (base_type == kMetaTypeNone && ge_input_info.type == Ms2GeParamInfo::OPTIONAL) {
        MS_LOG(DEBUG) << "Input is a placeholder, continue!";
        continue;
      }
      if (GetMoreDataTypeSupported(base_type, info->op_type())) {
        continue;
      }
      MS_LOG(DEBUG) << "Unsupported input dtype:" << TypeIdLabel(base_type)
                    << " in ACL, node:" << cnode->fullname_with_scope();
      return UNKNOWN_KERNEL_TYPE;
    }

    if (ge_input_info.type == Ms2GeParamInfo::DYNAMIC) {
      std::vector<int64_t> dyn_input_sizes = {};
      if (common::AnfAlgo::HasNodeAttr(kAttrDynInputSizes, cnode)) {
        dyn_input_sizes = common::AnfAlgo::GetNodeAttr<std::vector<int64_t>>(cnode, kAttrDynInputSizes);
      }
      if (dyn_input_sizes.size() != 1) {
        MS_LOG(EXCEPTION) << "Attribute of " << cnode->fullname_with_scope() << " is " << dyn_input_sizes
                          << ", of which size is not 1";
      }
      ms_real_idx += LongToSize(dyn_input_sizes[0]);
    } else {
      ms_real_idx += 1;
    }
  }

  return ACL_KERNEL;
}

KernelType AclHelper::GetKernelInfoByOutputs(const AnfNodePtr &node, const std::shared_ptr<GeAdapterInfo> &info) {
  auto output_supported_dtypes = info->output_supported_dtypes();
  auto output_flags = info->GetOutputMappingFlags();
  size_t output_num = ((output_flags & GeTensorInfo::kDynamicParam) == 0) ? info->GetNumOutputsOfMsOpProto()
                                                                          : AnfAlgo::GetOutputTensorNum(node);

  auto is_support = [&node, &output_supported_dtypes](size_t i) {
    auto base_type = common::AnfAlgo::GetOutputInferDataType(node, i);
    if (!std::any_of(output_supported_dtypes[i].begin(), output_supported_dtypes[i].end(),
                     [base_type](const ::ge::DataType ge_type) { return ConvertGeType(ge_type) == base_type; })) {
      MS_LOG(DEBUG) << "Unsupported output dtype:" << TypeIdLabel(base_type)
                    << " in ACL, node:" << node->fullname_with_scope();
      return false;
    }
    return true;
  };

  // operator has dynamic output
  if ((info->GetOutputMappingFlags() & GeTensorInfo::kDynamicParam) != 0) {
    if (info->GetNumOutputsOfMsOpProto() == 1) {
      return is_support(0) ? ACL_KERNEL : UNKNOWN_KERNEL_TYPE;
    } else {
      MS_LOG(EXCEPTION)
        << "Now not support operator containing dynamic output mixed with other outputs, the failed not is "
        << node->fullname_with_scope();
    }
  }

  // operator does not have dynamic output
  for (size_t i = 0; i < output_num; ++i) {
    if (!is_support(i)) {
      return UNKNOWN_KERNEL_TYPE;
    }
  }

  return ACL_KERNEL;
}

KernelType AclHelper::GetKernelInfoFromGe(const AnfNodePtr &node) {
  MS_EXCEPTION_IF_NULL(node);
  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);

  static const std::set<std::string> excuded_nodes = {prim::kCTCLoss};
  std::string name = GetCNodeFuncName(cnode);
  if (excuded_nodes.count(name) != 0) {
    return KernelType::UNKNOWN_KERNEL_TYPE;
  }

  auto info = GeAdapterManager::GetInstance().GetInfo(name, true);
  if (info == nullptr) {
    MS_LOG(WARNING) << "Unsupported op type on acl, node name: " << node->fullname_with_scope();
    return UNKNOWN_KERNEL_TYPE;
  }

  // check whether all inputs are matched
  if (GetKernelInfoByInputs(cnode, info) == UNKNOWN_KERNEL_TYPE) {
    return UNKNOWN_KERNEL_TYPE;
  }

  return ACL_KERNEL;
}

namespace {
void GetInputBuildInfo(const AnfNodePtr &node, const size_t input_num, const AclAdapterInfo &acl_info,
                       const GeAdapterInfoPtr &ge_info, std::vector<std::string> *input_formats,
                       std::vector<std::string> *input_reshape_types) {
  auto input_info = acl_info.inputs();
  for (size_t i = 0; i < input_num; ++i) {
    auto ge_idx = ge_info->GetGeInputByMsInputIndex(i).index;
    if (ge_idx >= input_info.size()) {
      continue;
    }
    auto special_info = input_info.at(ge_idx);
    auto input_format = AnfAlgo::GetPrevNodeOutputFormat(node, i);
    if (!special_info.dev_format.empty()) {
      auto iter = std::find(special_info.dev_format.begin(), special_info.dev_format.end(), input_format);
      if (iter != special_info.dev_format.end()) {
        input_formats->at(i) = input_format;
      } else {
        if (AclHelper::CheckDefaultSupportFormat(input_format)) {
          input_formats->at(i) = kOpFormat_DEFAULT;
        } else {
          MS_LOG(EXCEPTION) << "Acl kernel input not support this format: " << input_format
                            << ", kernel: " << node->fullname_with_scope() << ", input idx: " << ge_idx;
        }
      }
    } else {
      if (!AclHelper::CheckDefaultSupportFormat(input_format)) {
        MS_LOG(EXCEPTION) << "Acl kernel input not support this format: " << input_format
                          << ", kernel: " << node->fullname_with_scope() << ", input idx: " << ge_idx;
      }
    }
    if (!special_info.reshape_type.empty()) {
      input_reshape_types->at(i) = special_info.reshape_type;
    }
  }
}

void GetOutputBuildInfo(const AnfNodePtr &node, const size_t output_num, const AclAdapterInfo &acl_info,
                        std::vector<std::string> *output_formats) {
  if (acl_info.output_selector() != nullptr) {
    auto data_type = common::AnfAlgo::GetOutputInferDataType(node, 0);
    std::vector<ShapeVector> input_shapes;
    for (size_t i = 0; i < common::AnfAlgo::GetInputTensorNum(node); ++i) {
      (void)input_shapes.emplace_back(common::AnfAlgo::GetPrevNodeOutputInferShape(node, i));
    }
    auto func = acl_info.output_selector();
    for (size_t i = 0; i < output_num; ++i) {
      const auto &format = func(data_type, input_shapes);
      if (format != kOpFormat_DEFAULT) {
        common::AnfAlgo::SetNodeAttr(kAttrAclSpecialFormat, MakeValue(true), node);
      }
      output_formats->at(i) = format;
    }
    return;
  }

  for (size_t i = 0; i < output_num; ++i) {
    output_formats->at(i) = kOpFormat_DEFAULT;
  }
}
}  // namespace

void AclHelper::GetValidKernelBuildInfo(const AnfNodePtr &node, std::vector<std::string> *input_formats,
                                        std::vector<std::string> *output_formats,
                                        std::vector<std::string> *input_reshape_types,
                                        std::vector<std::string> *output_reshape_types) {
  MS_EXCEPTION_IF_NULL(node);
  MS_EXCEPTION_IF_NULL(input_formats);
  MS_EXCEPTION_IF_NULL(output_formats);
  MS_EXCEPTION_IF_NULL(input_reshape_types);
  MS_EXCEPTION_IF_NULL(output_reshape_types);
  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);
  std::string name = GetCNodeFuncName(cnode);
  auto info = GeAdapterManager::GetInstance().GetInfo(name, true);
  auto op_type = info->op_type();

  input_formats->clear();
  output_formats->clear();
  input_reshape_types->clear();
  output_reshape_types->clear();
  size_t input_num = common::AnfAlgo::GetInputTensorNum(node);
  size_t output_num = AnfUtils::GetOutputTensorNum(node);
  input_formats->assign(input_num, kOpFormat_DEFAULT);
  output_formats->assign(output_num, kOpFormat_DEFAULT);
  input_reshape_types->assign(input_num, "");
  output_reshape_types->assign(output_num, "");

  if (!AclAdapterManager::GetInstance().CheckAclAdapter(op_type)) {
    for (size_t i = 0; i < input_num; ++i) {
      auto input_format = AnfAlgo::GetPrevNodeOutputFormat(node, i);
      if (!CheckDefaultSupportFormat(input_format)) {
        MS_LOG(EXCEPTION) << "Acl kernel input not support this format: " << input_format
                          << ", kernel: " << node->fullname_with_scope() << ", input idx: " << i;
      }
    }
    return;
  }

  auto acl_info = AclAdapterManager::GetInstance().GetOpInfo(op_type);
  GetInputBuildInfo(node, input_num, acl_info, info, input_formats, input_reshape_types);
  GetOutputBuildInfo(node, output_num, acl_info, output_formats);
}

std::string AclHelper::ConvertOriginShapeAndFormat(const std::string &name, size_t idx, const std::string &dev_format,
                                                   ShapeVector *shape) {
  MS_EXCEPTION_IF_NULL(shape);
  auto info = GeAdapterManager::GetInstance().GetInfo(name, true);
  auto op_type = info->op_type();
  std::string ret_format = (shape->size() == kDim4) ? kOpFormat_NCHW : kOpFormat_DEFAULT;
  // case0: normal
  if (!AclAdapterManager::GetInstance().CheckAclAdapter(op_type)) {
    return ret_format;
  }
  // case1: 3d operator
  auto acl_info = AclAdapterManager::GetInstance().GetOpInfo(op_type);
  if (acl_info.is_3d()) {
    *shape = trans::PaddingShape(*shape, kOpFormat_NCDHW);
    return kOpFormat_NCDHW;
  }
  if (acl_info.is_need_pad_no_shape() && shape->empty()) {
    shape->push_back(1);
  }
  // case2: no special config
  auto info_list = acl_info.inputs();
  if (info_list.empty()) {
    return ret_format;
  }
  auto ge_idx = info->GetGeInputByMsInputIndex(idx).index;
  auto special_iter = info_list.find(ge_idx);
  if (special_iter == info_list.end() || special_iter->second.ori_format.empty()) {
    return ret_format;
  }
  // case3: if config input ori format or dev_format is special
  if (!special_iter->second.ori_format.empty() || !CheckDefaultSupportFormat(dev_format)) {
    if (ret_format == kOpFormat_DEFAULT && shape->size() < kDim4) {
      *shape = trans::PaddingShape(*shape, kOpFormat_NCHW, special_iter->second.reshape_type);
      ret_format = kOpFormat_NCHW;
    }
  }
  return ret_format;
}

bool AclHelper::NeedCheckAttrToInput(const CNodePtr &node,
                                     const mindspore::HashMap<size_t, std::string> &attr_input_map, size_t index) {
  MS_EXCEPTION_IF_NULL(node);
  if (attr_input_map.count(index) == 0) {
    return false;
  }

  const auto &attr_name = attr_input_map.at(index);
  if (common::AnfAlgo::HasNodeAttr(attr_name, node)) {
    return true;
  }
  return false;
}

std::string AclHelper::GetFormatFromAttr(const PrimitivePtr &primitive) {
  MS_EXCEPTION_IF_NULL(primitive);
  auto attrs = primitive->attrs();
  std::string format;
  if (attrs.count("format") != 0) {
    auto attr_value = attrs.at("format");
    if (attr_value->isa<StringImm>()) {
      format = GetValue<std::string>(attr_value);
    } else {
      MS_LOG(DEBUG) << "The attr format is not a valid value.";
    }
  }
  return format;
}

int64_t AclHelper::GetFracZGroupFromAttr(const PrimitivePtr &primitive) {
  MS_EXCEPTION_IF_NULL(primitive);
  auto attrs = primitive->attrs();
  int64_t fracz_group = 1;
  if (attrs.count(kAttrFracZGroup) != 0) {
    auto attr_value = attrs.at(kAttrFracZGroup);
    if (attr_value->isa<Int64Imm>()) {
      fracz_group = GetValue<int64_t>(attr_value);
    } else {
      MS_LOG(DEBUG) << "The FracZGroup attr format is not a valid value.";
    }
  }
  return fracz_group;
}
}  // namespace transform
}  // namespace mindspore
