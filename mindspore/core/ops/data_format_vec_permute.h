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

#ifndef MINDSPORE_CORE_OPS_DATA_FORMAT_VEC_PERMUTE_H_
#define MINDSPORE_CORE_OPS_DATA_FORMAT_VEC_PERMUTE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ops/base_operator.h"
#include "mindapi/base/types.h"

namespace mindspore {
namespace ops {
constexpr auto kNameDataFormatVecPermute = "DataFormatVecPermute";
/// \brief Permute input tensor from src_format to dst_format.
/// Refer to Python API @ref mindspore.ops.DataFormatVecPermute for more details.
class MIND_API DataFormatVecPermute : public BaseOperator {
 public:
  MIND_API_BASE_MEMBER(DataFormatVecPermute);
  /// \brief Constructor.
  DataFormatVecPermute() : BaseOperator(kNameDataFormatVecPermute) { InitIOName({"x"}, {"y"}); }
};

abstract::AbstractBasePtr DataFormatVecPermuteInfer(const abstract::AnalysisEnginePtr &, const PrimitivePtr &primitive,
                                                    const std::vector<abstract::AbstractBasePtr> &input_args);

using PrimDataFormatVecPermutePtr = std::shared_ptr<DataFormatVecPermute>;
}  // namespace ops
}  // namespace mindspore
#endif  // MINDSPORE_CORE_OPS_DATA_FORMAT_VEC_PERMUTE_H_
