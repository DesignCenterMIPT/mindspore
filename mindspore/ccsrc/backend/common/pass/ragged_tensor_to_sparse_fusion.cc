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

#include "backend/common/pass/ragged_tensor_to_sparse_fusion.h"

#include <vector>
#include <memory>

#include "mindspore/core/ops/sparse_ops.h"
#include "include/common/utils/anfalgo.h"

namespace mindspore {
namespace opt {
const BaseRef RaggedTensorToSparseFusion::DefinePattern() const {
  VarPtr Xs = std::make_shared<SeqVar>();
  return VectorRef({prim::kPrimRaggedTensorToSparse, Xs});
}

const AnfNodePtr RaggedTensorToSparseFusion::Process(const FuncGraphPtr &graph, const AnfNodePtr &node,
                                                     const EquivPtr &) const {
  constexpr auto kRaggedRank = "RAGGED_RANK";

  MS_EXCEPTION_IF_NULL(graph);
  MS_EXCEPTION_IF_NULL(node);

  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);

  auto splits_shape = common::AnfAlgo::GetPrevNodeOutputInferShape(cnode, kIndex0);

  common::AnfAlgo::SetNodeAttr(kRaggedRank, MakeValue(SizeToLong(splits_shape.size())), cnode);

  return cnode;
}
}  // namespace opt
}  // namespace mindspore
