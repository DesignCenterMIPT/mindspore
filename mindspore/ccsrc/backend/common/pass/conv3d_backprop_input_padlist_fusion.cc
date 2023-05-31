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

#include "backend/common/pass/conv3d_backprop_input_padlist_fusion.h"

#include <memory>
#include <vector>
#include <string>

#include "mindspore/core/ops/conv_pool_ops.h"
#include "include/common/utils/anfalgo.h"

namespace mindspore {
namespace opt {
namespace {
const AnfNodePtr SetConv3DBackpropPadList(const AnfNodePtr &node, const std::vector<int64_t> &dout_shape_norm,
                                          const std::vector<int64_t> &x_size_v) {
  constexpr auto kKernelSize = "kernel_size";
  constexpr auto kStride = "stride";
  constexpr auto kDilation = "dilation";
  constexpr auto kPadList = "pad_list";
  constexpr auto kPadMode = "pad_mode";
  constexpr auto kPad = "pad";
  constexpr int kConv3DBackpropFilterPadHalf = 2;

  MS_EXCEPTION_IF_NULL(node);
  const auto &origin_prim = common::AnfAlgo::GetCNodePrimitive(node);
  MS_EXCEPTION_IF_NULL(origin_prim);
  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);
  auto cnode_name = cnode->fullname_with_scope();
  auto kernel_size =
    CheckAndConvertUtils::CheckIntOrTupleInt("attribute[kernel_size]", origin_prim->GetAttr(kKernelSize), cnode_name);
  auto stride =
    CheckAndConvertUtils::CheckIntOrTupleInt("attribute[stride]", origin_prim->GetAttr(kStride), cnode_name);
  auto dilation =
    CheckAndConvertUtils::CheckIntOrTupleInt("attribute[dilation]", origin_prim->GetAttr(kDilation), cnode_name);
  // The pad_mode is valid by default. If pad_mode is not valid or same, then pad.
  int64_t pad_mode;
  CheckAndConvertUtils::GetPadModEnumValue(origin_prim->GetAttr(kPadMode), &pad_mode, False);
  ShapeVector pad_list;
  if (pad_mode == PadMode::VALID) {
    pad_list = {0, 0, 0, 0, 0, 0};
  } else if (pad_mode == PadMode::SAME) {
    auto kernel_d = kernel_size[kIndex0];
    auto kernel_h = kernel_size[kIndex1];
    auto kernel_w = kernel_size[kIndex2];
    auto stride_d = stride[kIndex2];
    auto stride_h = stride[kIndex3];
    auto stride_w = stride[kIndex4];
    auto dilation_d = dilation[kIndex2];
    auto dilation_h = dilation[kIndex3];
    auto dilation_w = dilation[kIndex4];
    int64_t pad_head = abstract::Shape::kShapeDimAny;
    int64_t pad_tail = abstract::Shape::kShapeDimAny;
    int64_t pad_top = abstract::Shape::kShapeDimAny;
    int64_t pad_bottom = abstract::Shape::kShapeDimAny;
    int64_t pad_left = abstract::Shape::kShapeDimAny;
    int64_t pad_right = abstract::Shape::kShapeDimAny;
    if (dout_shape_norm[kIndex2] != abstract::Shape::kShapeDimAny &&
        x_size_v[kIndex2] != abstract::Shape::kShapeDimAny) {
      auto pad_needed_d =
        (dout_shape_norm[kIndex2] - 1) * stride_d + dilation_d * (kernel_d - 1) + 1 - x_size_v[kIndex2];
      pad_needed_d = 0 > pad_needed_d ? 0 : pad_needed_d;
      pad_head = pad_needed_d / kConv3DBackpropFilterPadHalf;
      pad_tail = pad_needed_d - pad_head;
    }
    if (dout_shape_norm[kIndex3] != abstract::Shape::kShapeDimAny &&
        x_size_v[kIndex3] != abstract::Shape::kShapeDimAny) {
      auto pad_needed_h =
        (dout_shape_norm[kIndex3] - 1) * stride_h + dilation_h * (kernel_h - 1) + 1 - x_size_v[kIndex3];
      pad_needed_h = 0 > pad_needed_h ? 0 : pad_needed_h;
      pad_top = pad_needed_h / kConv3DBackpropFilterPadHalf;
      pad_bottom = pad_needed_h - pad_top;
    }
    if (dout_shape_norm[kIndex4] != abstract::Shape::kShapeDimAny &&
        x_size_v[kIndex4] != abstract::Shape::kShapeDimAny) {
      auto pad_needed_w =
        (dout_shape_norm[kIndex4] - 1) * stride_w + dilation_w * (kernel_w - 1) + 1 - x_size_v[kIndex4];
      pad_needed_w = 0 > pad_needed_w ? 0 : pad_needed_w;
      pad_left = pad_needed_w / kConv3DBackpropFilterPadHalf;
      pad_right = pad_needed_w - pad_left;
    }
    pad_list = {pad_head, pad_tail, pad_top, pad_bottom, pad_left, pad_right};
  } else if (pad_mode == PadMode::PAD) {
    pad_list = CheckAndConvertUtils::CheckIntOrTupleInt("attribute[pad]", origin_prim->GetAttr(kPad), cnode_name);
  }

  common::AnfAlgo::SetNodeAttr(kPadList, MakeValue(pad_list), cnode);

  return cnode;
}

std::vector<int64_t> GetForwardInputShape(const AnfNodePtr &node) {
  auto input_size_abs = node->abstract();
  MS_EXCEPTION_IF_NULL(input_size_abs);
  auto input_size_value = input_size_abs->BuildValue();
  MS_EXCEPTION_IF_NULL(input_size_value);
  auto node_name = node->fullname_with_scope();

  if (!input_size_value->isa<ValueAny>() && !input_size_value->isa<None>()) {
    if (input_size_value->isa<tensor::Tensor>()) {
      return CheckAndConvertUtils::CheckTensorIntValue("input_size", input_size_value, node_name);
    } else if (input_size_value->isa<ValueSequence>()) {
      return CheckAndConvertUtils::CheckIntOrTupleInt("input_size", input_size_value, node_name);
    }
  } else if (input_size_abs->isa<abstract::AbstractTensor>()) {
    auto abs_tensor = input_size_abs->cast<abstract::AbstractTensorPtr>();
    MS_EXCEPTION_IF_NULL(abs_tensor);
    auto abs_tensor_shape = abs_tensor->shape()->shape();
    if (abs_tensor_shape.size() != 1) {
      MS_EXCEPTION(ValueError) << "For " << node_name << ", input_size must be one-dimensional.";
    }
    if (IsDynamic(abs_tensor_shape)) {
      return std::vector<int64_t>{abstract::Shape::kShapeRankAny};
    }
    auto shape_value = abs_tensor->get_shape_value();
    if (shape_value == nullptr) {
      return std::vector<int64_t>(abs_tensor_shape[0], abstract::Shape::kShapeDimAny);
    } else {
      auto shape_vector = GetValue<std::vector<int64_t>>(shape_value);
      auto errinfo = node_name + ", input_size has illegal shape of shape value";
      MS_EXCEPTION_IF_CHECK_FAIL(LongToSize(abs_tensor_shape[0]) == shape_vector.size(), errinfo);
      return shape_vector;
    }
  } else if (input_size_abs->isa<abstract::AbstractSequence>()) {
    auto abs_seq = dyn_cast<abstract::AbstractSequence>(input_size_abs);
    MS_EXCEPTION_IF_NULL(abs_seq);
    if (abs_seq->dynamic_len()) {
      return std::vector<int64_t>{abstract::Shape::kShapeRankAny};
    }
    std::vector<int64_t> shape_vector;
    for (auto element : abs_seq->elements()) {
      auto element_val = element->BuildValue();
      if (element_val == kValueAny) {
        (void)shape_vector.emplace_back(abstract::Shape::kShapeDimAny);
      } else if (element_val->isa<Int64Imm>()) {
        (void)shape_vector.emplace_back(GetValue<int64_t>(element_val));
      } else if (element_val->isa<Int32Imm>()) {
        (void)shape_vector.emplace_back(GetValue<int32_t>(element_val));
      } else {
        MS_EXCEPTION(TypeError) << "For " << node_name
                                << " the input_size must be one of ['tuple', 'list'] with all Int elements, but got "
                                << input_size_abs->ToString();
      }
    }
    return shape_vector;
  }
  auto size_type = input_size_abs->BuildType();
  MS_EXCEPTION_IF_NULL(size_type);
  MS_EXCEPTION(TypeError) << "For " << node_name << ", the input_size must be Tensor/Tuple/List, but got"
                          << size_type->ToString() << ".";
}
}  // namespace

const BaseRef Conv3DBackpropInputPadListFusion::DefinePattern() const {
  VarPtr Xs = std::make_shared<SeqVar>();
  return VectorRef({prim::kPrimConv3DBackpropInput, Xs});
}

const AnfNodePtr Conv3DBackpropInputPadListFusion::Process(const FuncGraphPtr &graph, const AnfNodePtr &node,
                                                           const EquivPtr &) const {
  MS_EXCEPTION_IF_NULL(graph);
  MS_EXCEPTION_IF_NULL(node);

  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);
  auto input_size_node = cnode->input(kIndex3);
  MS_EXCEPTION_IF_NULL(input_size_node);
  auto forward_input_shape = GetForwardInputShape(input_size_node);
  auto filter_shape = common::AnfAlgo::GetPrevNodeOutputInferShape(cnode, kIndex0);
  auto dout_shape = common::AnfAlgo::GetPrevNodeOutputInferShape(cnode, kIndex1);
  if (IsDynamicRank(forward_input_shape) || IsDynamicRank(dout_shape) || IsDynamicRank(filter_shape)) {
    MS_LOG(ERROR) << "For '" << cnode->fullname_with_scope()
                  << "', can't add attr padlist while shape of inputs is dynamic rank.";
    return cnode;
  }
  return SetConv3DBackpropPadList(node, dout_shape, forward_input_shape);
}

const BaseRef Conv3DBackpropFilterPadListFusion::DefinePattern() const {
  VarPtr Xs = std::make_shared<SeqVar>();
  return VectorRef({prim::kPrimConv3DBackpropFilter, Xs});
}

const AnfNodePtr Conv3DBackpropFilterPadListFusion::Process(const FuncGraphPtr &graph, const AnfNodePtr &node,
                                                            const EquivPtr &) const {
  MS_EXCEPTION_IF_NULL(graph);
  MS_EXCEPTION_IF_NULL(node);

  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);
  auto input_shape = common::AnfAlgo::GetPrevNodeOutputInferShape(cnode, kIndex0);
  auto dout_shape = common::AnfAlgo::GetPrevNodeOutputInferShape(cnode, kIndex1);
  if (IsDynamicRank(input_shape) || IsDynamicRank(dout_shape)) {
    MS_LOG(ERROR) << "For '" << cnode->fullname_with_scope()
                  << "', can't add attr padlist while shape of inputs is dynamic rank.";
    return cnode;
  }
  return SetConv3DBackpropPadList(node, dout_shape, input_shape);
}
}  // namespace opt
}  // namespace mindspore
