/**
 * Copyright 2021 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_CORE_OPS_HSIGMOID_H_
#define MINDSPORE_CORE_OPS_HSIGMOID_H_
#include <vector>
#include <memory>

#include "ops/primitive_c.h"
#include "ops/op_utils.h"
#include "utils/check_convert_utils.h"

namespace mindspore {
namespace ops {
constexpr auto kNameHSigmoid = "HSigmoid";
/// \brief Hard sigmoid activation function. Refer to Python API @ref mindspore.ops.HSigmoid for more details.
class MS_CORE_API HSigmoid : public PrimitiveC {
 public:
  /// \brief Constructor.
  HSigmoid() : PrimitiveC(kNameHSigmoid) { InitIOName({"input_x"}, {"output"}); }
  /// \brief Destructor.
  ~HSigmoid() = default;
  MS_DECLARE_PARENT(HSigmoid, PrimitiveC);  // come from ops/primitive_c.h
};

AbstractBasePtr HSigmoidInfer(const abstract::AnalysisEnginePtr &, const PrimitivePtr &primitive,
                              const std::vector<AbstractBasePtr> &input_args);

using PrimHSigmoidPtr = std::shared_ptr<HSigmoid>;
}  // namespace ops
}  // namespace mindspore

#endif  // MINDSPORE_CORE_OPS_HSIGMOID_H_
