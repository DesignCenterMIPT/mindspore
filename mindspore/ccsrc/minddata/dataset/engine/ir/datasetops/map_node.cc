/**
 * Copyright 2020-2021 Huawei Technologies Co., Ltd
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

#include "minddata/dataset/engine/ir/datasetops/map_node.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifndef ENABLE_ANDROID
#include "minddata/dataset/engine/serdes.h"
#endif
#include "minddata/dataset/engine/datasetops/map_op/map_op.h"
#include "minddata/dataset/engine/opt/pass.h"
#include "minddata/dataset/kernels/ir/tensor_operation.h"
#include "minddata/dataset/util/status.h"
namespace mindspore {
namespace dataset {

MapNode::MapNode(std::shared_ptr<DatasetNode> child, std::vector<std::shared_ptr<TensorOperation>> operations,
                 std::vector<std::string> input_columns, std::vector<std::string> output_columns,
                 const std::vector<std::string> &project_columns, std::shared_ptr<DatasetCache> cache,
                 std::vector<std::shared_ptr<DSCallback>> callbacks, bool offload)
    : operations_(operations),
      input_columns_(input_columns),
      output_columns_(output_columns),
      project_columns_(project_columns),
      DatasetNode(std::move(cache)),
      callbacks_(callbacks),
      offload_(offload) {
  this->AddChild(child);
}

std::shared_ptr<DatasetNode> MapNode::Copy() {
  std::vector<std::shared_ptr<TensorOperation>> operations = operations_;
  auto node = std::make_shared<MapNode>(nullptr, operations, input_columns_, output_columns_, project_columns_, cache_,
                                        callbacks_, offload_);
  return node;
}

void MapNode::Print(std::ostream &out) const {
  out << (Name() + "(<ops>" + ",input:" + PrintColumns(input_columns_) + ",output:" + PrintColumns(output_columns_) +
          ",<project_cols>" + ",num_tensor_ops:")
      << operations_.size() << ",...)";
}

Status MapNode::Build(std::vector<std::shared_ptr<DatasetOp>> *const node_ops) {
  RETURN_UNEXPECTED_IF_NULL(node_ops);
  std::vector<std::shared_ptr<TensorOp>> tensor_ops;

  // Build tensorOp from tensorOperation vector
  // This is to ensure each iterator hold its own copy of the tensorOp objects.
  (void)std::transform(
    operations_.begin(), operations_.end(), std::back_inserter(tensor_ops),
    [](std::shared_ptr<TensorOperation> operation) -> std::shared_ptr<TensorOp> { return operation->Build(); });

  // This is temporary code.
  // Because the randomness of its tensor operations is not known in TensorOperation form until we convert them
  // to TensorOp, we need to check the randomness here.
  // When TensorOperation captures the randomness behaviour, remove this code
  // and the temporary code in CacheValidation pre pass in IR optimizer.
  if (IsDescendantOfCache()) {
    auto itr = std::find_if(tensor_ops.begin(), tensor_ops.end(), [](const auto &it) { return !it->Deterministic(); });
    if (itr != tensor_ops.end()) {
      RETURN_STATUS_UNEXPECTED("MapNode containing random operation is not supported as a descendant of cache.");
    }
  }
  // This parameter will be removed with next rebase
  std::vector<std::string> col_orders;
  auto map_op = std::make_shared<MapOp>(input_columns_, output_columns_, tensor_ops, num_workers_, connector_que_size_);

  if (!callbacks_.empty()) {
    map_op->AddCallbacks(callbacks_);
  }

  if (!project_columns_.empty()) {
    auto project_op = std::make_shared<ProjectOp>(project_columns_);
    project_op->SetTotalRepeats(GetTotalRepeats());
    project_op->SetNumRepeatsPerEpoch(GetNumRepeatsPerEpoch());
    node_ops->push_back(project_op);
  }
  map_op->SetTotalRepeats(GetTotalRepeats());
  map_op->SetNumRepeatsPerEpoch(GetNumRepeatsPerEpoch());
  node_ops->push_back(map_op);
  return Status::OK();
}

Status MapNode::ValidateParams() {
  RETURN_IF_NOT_OK(DatasetNode::ValidateParams());
  if (operations_.empty()) {
    std::string err_msg = "MapNode: No operation is specified.";
    LOG_AND_RETURN_STATUS_SYNTAX_ERROR(err_msg);
  }
  for (const auto &op : operations_) {
    if (op == nullptr) {
      std::string err_msg = "MapNode: operation must not be nullptr.";
      LOG_AND_RETURN_STATUS_SYNTAX_ERROR(err_msg);
    } else {
      RETURN_IF_NOT_OK(op->ValidateParams());
    }
  }
  if (!input_columns_.empty()) {
    RETURN_IF_NOT_OK(ValidateDatasetColumnParam("MapNode", "input_columns", input_columns_));
  }

  if (!output_columns_.empty()) {
    RETURN_IF_NOT_OK(ValidateDatasetColumnParam("MapNode", "output_columns", output_columns_));
  }

  if (!project_columns_.empty()) {
    RETURN_IF_NOT_OK(ValidateDatasetColumnParam("MapNode", "project_columns", project_columns_));
  }

  return Status::OK();
}

// Visitor accepting method for IRNodePass
Status MapNode::Accept(IRNodePass *const p, bool *const modified) {
  RETURN_UNEXPECTED_IF_NULL(p);
  RETURN_UNEXPECTED_IF_NULL(modified);
  // Downcast shared pointer then call visitor
  return p->Visit(shared_from_base<MapNode>(), modified);
}

// Visitor accepting method for IRNodePass
Status MapNode::AcceptAfter(IRNodePass *const p, bool *const modified) {
  RETURN_UNEXPECTED_IF_NULL(p);
  RETURN_UNEXPECTED_IF_NULL(modified);
  // Downcast shared pointer then call visitor
  return p->VisitAfter(shared_from_base<MapNode>(), modified);
}

void MapNode::setOperations(const std::vector<std::shared_ptr<TensorOperation>> &operations) {
  operations_ = operations;
}
std::vector<std::shared_ptr<TensorOperation>> MapNode::operations() { return operations_; }

void MapNode::SetOffload(bool offload) { offload_ = offload; }

Status MapNode::to_json(nlohmann::json *out_json) {
  RETURN_UNEXPECTED_IF_NULL(out_json);
  nlohmann::json args;
  args["num_parallel_workers"] = num_workers_;
  args["input_columns"] = input_columns_;
  args["output_columns"] = output_columns_;
  args["project_columns"] = project_columns_;
  if (cache_ != nullptr) {
    nlohmann::json cache_args;
    RETURN_IF_NOT_OK(cache_->to_json(&cache_args));
    args["cache"] = cache_args;
  }
  std::vector<nlohmann::json> ops;
  std::vector<int32_t> cbs;
  for (auto op : operations_) {
    RETURN_UNEXPECTED_IF_NULL(op);
    nlohmann::json op_args;
    RETURN_IF_NOT_OK(op->to_json(&op_args));
    if (op->Name() == "PyFuncOp") {
      ops.push_back(op_args);
    } else {
      nlohmann::json op_item;
      op_item["tensor_op_params"] = op_args;
      op_item["tensor_op_name"] = op->Name();
      ops.push_back(op_item);
    }
  }
  args["operations"] = ops;
  (void)std::transform(callbacks_.begin(), callbacks_.end(), std::back_inserter(cbs),
                       [](std::shared_ptr<DSCallback> cb) -> int32_t { return cb != nullptr ? cb->step_size() : 0; });
  args["callback"] = cbs;

  *out_json = args;
  return Status::OK();
}

#ifndef ENABLE_ANDROID
Status MapNode::from_json(nlohmann::json json_obj, std::shared_ptr<DatasetNode> ds,
                          std::shared_ptr<DatasetNode> *result) {
  CHECK_FAIL_RETURN_UNEXPECTED(json_obj.find("num_parallel_workers") != json_obj.end(),
                               "Failed to find num_parallel_workers");
  CHECK_FAIL_RETURN_UNEXPECTED(json_obj.find("input_columns") != json_obj.end(), "Failed to find input_columns");
  CHECK_FAIL_RETURN_UNEXPECTED(json_obj.find("output_columns") != json_obj.end(), "Failed to find output_columns");
  CHECK_FAIL_RETURN_UNEXPECTED(json_obj.find("project_columns") != json_obj.end(), "Failed to find project_columns");
  CHECK_FAIL_RETURN_UNEXPECTED(json_obj.find("operations") != json_obj.end(), "Failed to find operations");
  std::vector<std::string> input_columns = json_obj["input_columns"];
  std::vector<std::string> output_columns = json_obj["output_columns"];
  std::vector<std::string> project_columns = json_obj["project_columns"];
  std::vector<std::shared_ptr<TensorOperation>> operations;
  RETURN_IF_NOT_OK(Serdes::ConstructTensorOps(json_obj["operations"], &operations));
  *result = std::make_shared<MapNode>(ds, operations, input_columns, output_columns, project_columns);
  (*result)->SetNumWorkers(json_obj["num_parallel_workers"]);
  return Status::OK();
}
#endif

// Gets the dataset size
Status MapNode::GetDatasetSize(const std::shared_ptr<DatasetSizeGetter> &size_getter, bool estimate,
                               int64_t *dataset_size) {
  if (dataset_size_ > 0) {
    *dataset_size = dataset_size_;
    return Status::OK();
  }
  // If cache is injected after a MapNode, it is possible that the pipeline needs to handle different numbers of rows
  // compared to a non-cached pipeline. This is mostly true for TFRecord dataset, since it uses row-based sharding
  // with cache but file-based sharding without cache. However, MapNode couldn't tell whether the leaf below is
  // TFRecord or not, therefore it doesn't rely on its child here but simply run through the tree.
  if (!IsSizeDefined() || IsCached()) {
    RETURN_IF_NOT_OK(size_getter->DryRun(shared_from_this(), dataset_size));
    dataset_size_ = *dataset_size;
    return Status::OK();
  }
  if (children_.size() == 1) {
    return children_.front()->GetDatasetSize(size_getter, estimate, dataset_size);
  } else {
    RETURN_STATUS_UNEXPECTED("Trying to get dataset size from leaf node, missing override");
  }
}
}  // namespace dataset
}  // namespace mindspore
