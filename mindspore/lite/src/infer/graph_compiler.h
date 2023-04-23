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
#ifndef MINDSPORE_LITE_INFER_GRAPH_COMPILER_H_
#define MINDSPORE_LITE_INFER_GRAPH_COMPILER_H_

#include <memory>
#include <string>
#include "infer/execution_plan.h"
#include "infer/execution_flow.h"

namespace mindspore::infer::abstract {
constexpr char kBackendCPU[] = "CPU";
constexpr char kBackendGPU[] = "GPU";
constexpr char kBackendNPU[] = "NPU";
constexpr char kBackendAscend[] = "Ascend";
struct CompileOption {
  Format format{Format::NHWC};
  std::string backend{kBackendCPU};
  TypeId datatype{kNumberTypeFloat32};
};

class GraphCompiler : public std::enable_shared_from_this<GraphCompiler> {
 public:
  virtual ~GraphCompiler() = default;

  /// \brief Compile FuncGraph Into ExecutionPlan.
  ///
  /// \param[in] graph FuncGraph need to compile.
  ///
  /// \return ExecutionPlan pointer.
  virtual ExecutionPlanPtr Compile(FuncGraphPtr graph) = 0;

  virtual ExecutionFlowPtr Compile(const GraphSegmentPtr &segment, const AnfNodePtrList &inputs,
                                   const AnfNodePtrList &outputs, const CompileOption &option) = 0;
};
}  // namespace mindspore::infer::abstract

#endif  // MINDSPORE_LITE_INFER_GRAPH_COMPILER_H_
