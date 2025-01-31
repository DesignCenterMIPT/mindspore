/**
 * Copyright 2022 Huawei Technologies Co., Ltd
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
#include "plugin/device/ascend/optimizer/mindir/ascend_vm_op_adapter.h"

#include "include/common/utils/anfalgo.h"
#include "include/backend/anf_runtime_algorithm.h"
#include "plugin/device/ascend/optimizer/create_node_helper.h"

namespace mindspore::opt {
constexpr auto kHasRunMindIR = "HasRunMindIR";
const AnfNodePtr AscendMindIROpAdapter::Process(const FuncGraphPtr &graph, const AnfNodePtr &node,
                                                const EquivPtr &) const {
  MS_EXCEPTION_IF_NULL(graph);
  if (common::AnfAlgo::HasNodeAttr(kHasRunMindIR, node->cast<CNodePtr>())) {
    return node;
  }
  // There are other UnifyMindIR pass before AscendMindIROpAdapter which may create new nodes.
  if (graph->has_flag(kAttrMutableKernel) && AnfUtils::IsRealCNodeKernel(node)) {
    AnfAlgo::SetDynamicAttrToPrim(common::AnfAlgo::GetCNodePrimitive(node));
  }
  auto ret_node = CreateNodeHelper::CreateNodeWithCheck(node, true);
  if (ret_node != node) {
    common::AnfAlgo::SetNodeAttr(kHasRunMindIR, MakeValue(true), ret_node);
  }
  return ret_node;
}

const AnfNodePtr AscendVmOpAdapter::Process(const FuncGraphPtr &graph, const AnfNodePtr &node, const EquivPtr &) const {
  MS_EXCEPTION_IF_NULL(graph);
  // There are other UnifyMindIR pass before AscendVmOpAdapter which may create new nodes.
  if (graph->has_flag(kAttrMutableKernel) && AnfUtils::IsRealCNodeKernel(node)) {
    AnfAlgo::SetDynamicAttrToPrim(common::AnfAlgo::GetCNodePrimitive(node));
  }
  return CreateNodeHelper::CreateNodeWithCheck(node, false);
}
}  // namespace mindspore::opt
