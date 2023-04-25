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
#include "ops/shape_calc.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include "ops/op_utils.h"
#include "utils/check_convert_utils.h"
#include "utils/hash_set.h"
#include "abstract/ops/primitive_infer_map.h"
#include "mindapi/src/helper.h"

namespace mindspore {
namespace ops {
ShapeCalcFunctorPtr ShapeCalc::get_functor() const {
  auto attr = api::ToRef<mindspore::Primitive>(impl_).GetAttr(kAttrFunctor);
  MS_EXCEPTION_IF_NULL(attr);
  return attr->cast<ShapeCalcFunctorPtr>();
}

std::vector<int64_t> ShapeCalc::get_value_depend() const {
  return GetValue<std::vector<int64_t>>(GetAttr(kAttrValueDepend));
}

class MIND_API ShapeCalcInfer : public abstract::OpInferBase {
 public:
  AbstractBasePtr InferShapeAndType(const abstract::AnalysisEnginePtr &, const PrimitivePtr &primitive,
                                    const std::vector<AbstractBasePtr> &input_args) const override {
    MS_EXCEPTION_IF_NULL(primitive);
    auto prim_name = primitive->name();
    auto value_depend = GetValue<std::vector<int64_t>>(primitive->GetAttr(kAttrValueDepend));
    ShapeArray args(input_args.size());
    HashSet<size_t> unknown_inputs;
    for (size_t i = 0; i < input_args.size(); ++i) {
      MS_EXCEPTION_IF_NULL(input_args[i]);
      auto value_ptr = input_args[i]->BuildValue();
      MS_EXCEPTION_IF_NULL(value_ptr);
      if (input_args[i]->isa<abstract::AbstractSequence>() || input_args[i]->isa<abstract::AbstractScalar>()) {
        if (IsValueKnown(value_ptr)) {
          args[i] = CheckAndConvertUtils::CheckIntOrTupleInt(std::to_string(i), value_ptr, prim_name);
        } else {
          (void)unknown_inputs.insert(i);
        }
      } else if (input_args[i]->isa<abstract::AbstractTensor>()) {
        if (std::find(value_depend.begin(), value_depend.end(), SizeToLong(i)) != value_depend.end()) {
          // value tensor
          if (value_ptr->isa<tensor::Tensor>()) {
            args[i] = CheckAndConvertUtils::CheckTensorIntValue(std::to_string(i), value_ptr, prim_name);
          } else {
            (void)unknown_inputs.insert(i);
          }
          continue;
        }
        // shape tensor
        auto input = input_args[i]->cast<abstract::AbstractTensorPtr>();
        MS_EXCEPTION_IF_NULL(input);
        auto shape_ptr = input->shape();
        MS_EXCEPTION_IF_NULL(shape_ptr);
        const auto &shape = shape_ptr->shape();
        // input is a tensor that saves the shape, and tensor itself should be 0D or 1D
        MS_EXCEPTION_IF_CHECK_FAIL(shape.size() <= 1, "Input tensor's rank must be <= 1");
        if (!shape.empty()) {
          args[i] = shape[0] < 0 ? ShapeVector{abstract::Shape::kShapeRankAny}
                                 : ShapeVector(shape[0], abstract::Shape::kShapeDimAny);
        }
      } else {
        MS_EXCEPTION(TypeError) << "For '" << prim_name << "', input[" << i
                                << "] must be a Tensor, scalar integer, list integer or tuple integer, but got "
                                << input_args[i]->ToString();
      }
    }
    auto functor_attr = primitive->GetAttr(kAttrFunctor);
    MS_EXCEPTION_IF_NULL(functor_attr);
    auto functor = functor_attr->cast<ShapeCalcFunctorPtr>();
    MS_EXCEPTION_IF_NULL(functor);
    auto out = functor->Infer(args, unknown_inputs);
    if (out.size() == 1) {
      // single output does not use AbstractTuple to avoid TupleGetItem
      return std::make_shared<abstract::AbstractTensor>(kInt64, out);
    }
    // multiple outputs
    AbstractBasePtrList abs_list;
    abs_list.reserve(out.size());
    (void)std::transform(out.begin(), out.end(), std::back_inserter(abs_list),
                         [](int64_t s) { return std::make_shared<abstract::AbstractTensor>(kInt64, ShapeVector{s}); });
    return std::make_shared<abstract::AbstractTuple>(abs_list);
  }

  BaseShapePtr InferShape(const PrimitivePtr &primitive,
                          const std::vector<AbstractBasePtr> &input_args) const override {
    return InferShapeAndType(nullptr, primitive, input_args)->BuildShape();
  }

  TypePtr InferType(const PrimitivePtr &primitive, const std::vector<AbstractBasePtr> &input_args) const override {
    return InferShapeAndType(nullptr, primitive, input_args)->BuildType();
  }
};
MIND_API_OPERATOR_IMPL(ShapeCalc, BaseOperator);
REGISTER_PRIMITIVE_OP_INFER_IMPL(ShapeCalc, prim::kPrimShapeCalc, ShapeCalcInfer, false);
}  // namespace ops
}  // namespace mindspore
