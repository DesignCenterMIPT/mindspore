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

#ifndef MINDSPORE_CCSRC_FRONTEND_PARALLEL_PIPELINE_TRANSFORMER_PIPELINE_TRANSFORMER_H_
#define MINDSPORE_CCSRC_FRONTEND_PARALLEL_PIPELINE_TRANSFORMER_PIPELINE_TRANSFORMER_H_

#include <utility>
#include <string>
#include <memory>
#include "ir/value.h"
#include "ir/graph_utils.h"
#include "base/base.h"
#include "frontend/parallel/step_parallel.h"
#include "frontend/parallel/graph_util/generate_graph.h"

namespace mindspore {
namespace parallel {
using TensorLayoutPtr = std::shared_ptr<TensorLayout>;
using TensorInfoPtr = std::shared_ptr<TensorInfo>;

typedef struct {
  ValueListPtr shape;
  TypePtr type;
  AnfNodePtr depend;
} SendAttr;

class PipelineTransformer {
 public:
  PipelineTransformer(const FuncGraphManagerPtr &manager, int stage, const FuncGraphPtr &root, int64_t global_rank,
                      int64_t per_stage_rank_num)
      : manager_(manager),
        stage_(stage),
        root_(root),
        global_rank_(global_rank),
        per_stage_rank_num_(per_stage_rank_num) {}
  virtual ~PipelineTransformer() = default;
  void Coloring();
  void BroadCastColoring();
  void HandleSharedParameter();
  void CutGraph();
  void ParameterColoring();
  void CoverSensShape();
  void ElimGraphStage();
  void ElimParameter();

 private:
  std::pair<bool, int> IsSharedNode(const AnfNodePtr &node, const AnfNodeIndexSet &node_users);
  void DoBroadCast(const FuncGraphPtr &func);
  SendAttr InsertSend(const FuncGraphPtr &graph, const AnfNodePtr &parameter, int user_node_stage, int node_stage);
  void InsertReceive(const FuncGraphPtr &graph, const AnfNodePtr &node, const AnfNodePtr &use_node, int index,
                     int user_node_stage, int node_stage);
  void CutBorder(const FuncGraphPtr &graph);
  bool IsStageNode(const CNodePtr &node);
  std::pair<OperatorInfoPtr, TensorInfoPtr> GetOpInfo(const AnfNodePtr &node);
  std::pair<OperatorInfoPtr, TensorInfoPtr> GetParameterPair(const AnfNodePtr &node);
  OperatorInfoPtr CreateOpInfo(const CNodePtr &cnode);
  bool IsPipelineCareNode(const CNodePtr &cnode);
  std::pair<CNodePtr, FuncGraphPtr> FindSensNode();
  FuncGraphManagerPtr manager_;
  int64_t stage_;
  FuncGraphPtr root_;
  int64_t global_rank_;
  int64_t per_stage_rank_num_;
  TypePtr type_ptr_;
  ValueListPtr shape_;
  AnfNodePtr virtual_param_;
};
}  // namespace parallel
}  // namespace mindspore

#endif  // MINDSPORE_CCSRC_FRONTEND_PARALLEL_PIPELINE_TRANSFORMER_PIPELINE_TRANSFORMER_H_
