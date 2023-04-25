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
#ifndef MINDSPORE_CCSRC_PIPELINE_PYNATIVE_GRAD_BPROP_EXPANDER_GRAD_OPS_COMMON_UTILS_H_
#define MINDSPORE_CCSRC_PIPELINE_PYNATIVE_GRAD_BPROP_EXPANDER_GRAD_OPS_COMMON_UTILS_H_

#include <cmath>
#include <vector>
#include <utility>
#include <set>
#include <string>
#include <memory>
#include "include/common/expander/core/node.h"
#include "pipeline/pynative/grad/bprop_expander/bprop_irbuilder.h"

namespace mindspore::expander::bprop {
constexpr size_t i0 = 0;
constexpr size_t i1 = 1;
constexpr size_t i2 = 2;
constexpr size_t i3 = 3;
constexpr size_t i4 = 4;
constexpr size_t i5 = 5;
constexpr size_t i6 = 6;
constexpr size_t i7 = 7;
constexpr size_t i8 = 8;
constexpr size_t i9 = 9;
constexpr size_t i10 = 10;
inline const auto pi = std::acos(-1.0);
inline const auto log_2 = std::log(2.0);
inline const auto log_pi = std::log(pi);

class PureShapeCalc : public ShapeCalcFunctor {
 public:
  using CalcFunc = ShapeArray (*)(const ShapeArray &);
  using InferFunc = std::vector<int64_t> (*)(const ShapeArray &, const HashSet<size_t> &);
  explicit PureShapeCalc(const std::string &name) : ShapeCalcFunctor(name) {
    FunctorRegistry::Instance().Register(name, [this]() { return shared_from_base<Functor>(); });
  }
  PureShapeCalc(const PureShapeCalc &) = delete;
  PureShapeCalc(PureShapeCalc &&) = delete;
  PureShapeCalc &operator=(const PureShapeCalc &) = delete;
  ~PureShapeCalc() override = default;
  MS_DECLARE_PARENT(PureShapeCalc, ShapeCalcFunctor)

  ValuePtr ToValue() const override { return nullptr; }
  void FromValue(const ValuePtr &) override {}
  ShapeArray Calc(const ShapeArray &inputs) const override {
    MS_EXCEPTION_IF_CHECK_FAIL(calc_func_ != nullptr, "The calc_func of " + name() + " is nullptr");
    return calc_func_(inputs);
  }
  std::vector<int64_t> Infer(const ShapeArray &inputs, const HashSet<size_t> &unknown_inputs) const override {
    MS_EXCEPTION_IF_CHECK_FAIL(infer_func_ != nullptr, "The infer_func of " + name() + " is nullptr");
    return infer_func_(inputs, unknown_inputs);
  }

  PureShapeCalc &SetCalc(const CalcFunc &calc_func) {
    calc_func_ = calc_func;
    return *this;
  }
  std::shared_ptr<PureShapeCalc> SetInfer(const InferFunc &infer_func) {
    infer_func_ = infer_func;
    return shared_from_base<PureShapeCalc>();
  }

  CalcFunc calc_func_{nullptr};
  InferFunc infer_func_{nullptr};
};

#define DEF_PURE_SHAPE_CALC(name) \
  static std::shared_ptr<PureShapeCalc> name = (*(std::make_shared<PureShapeCalc>("ShapeCalc_" #name)))

// normalize the axis to [0, rank)
int64_t NormalizeAxis(int64_t axis, size_t rank);

std::vector<std::vector<int64_t>> BroadcastGradientArgs(const std::vector<int64_t> &x_shape,
                                                        const std::vector<int64_t> &y_shape);

std::vector<int64_t> GetTransposeAxis(const std::vector<int64_t> &x_shape, int64_t axis);

std::vector<int64_t> TupleDiv(const std::vector<int64_t> &x, const std::vector<int64_t> &y);

std::vector<int64_t> ReduceShape(const std::vector<int64_t> &x, const std::vector<int64_t> &axis);

int64_t CheckRange(int64_t idx, int64_t dim_size);

NodePtrList BinopGradCommon(const BpropIRBuilder *ib, const NodePtr &x, const NodePtr &y, const NodePtr &dx,
                            const NodePtr &dy);

NodePtrList BinopGradCommonWithShift(const BpropIRBuilder *ib, const NodePtr &x, const NodePtr &y, const NodePtr &dx,
                                     const NodePtr &dy, size_t shift);
NodePtr SumGradReduceAxisWithCast(const BpropIRBuilder *ib, const NodePtr &dx, const NodePtr &axis);

std::vector<int64_t> Range(int64_t start, int64_t stop, int64_t step = 1);
std::vector<int64_t> Range(int64_t stop);

template <typename T>
std::vector<T> operator+(std::vector<T> const &m, std::vector<T> const &n) {
  std::vector<T> v;                             // initialized vector v
  v.reserve(m.size() + n.size());               // reverse function used in v
  (void)v.insert(v.end(), m.begin(), m.end());  // insert func used in vec m.
  (void)v.insert(v.end(), n.begin(), n.end());  // insert func used in vec n.
  return v;                                     // return the vector v
}

int64_t GetIntValue(const NodePtr &node);
std::vector<int64_t> GetIntList(const ValuePtr &value);
std::vector<int64_t> GetIntList(const NodePtr &node);

NodePtr GetEps(const BpropIRBuilder *ib, const TypePtr &type);
std::vector<int64_t> GenerateInverseIndex(const std::vector<int64_t> &x_shp, int64_t axis_v, int64_t batch_dims = 0);
std::vector<int64_t> GenerateShapeIndex(const std::vector<int64_t> &out_shp, const std::vector<int64_t> &ind_shp,
                                        int64_t axis_v, int64_t batch_dims = 0);
std::vector<int64_t> RegenerateOutputShape(const std::vector<int64_t> &x_shp, const std::vector<int64_t> &ind_shp,
                                           int64_t axis_v, int64_t batch_dims = 0);
std::vector<int64_t> TileShape(const std::vector<int64_t> &multiples, const std::vector<int64_t> &shapex);
std::vector<int64_t> InvertPermutation(const std::vector<int64_t> &perm);
std::vector<int64_t> GetTransposition(int64_t axis, int64_t rank);

NodePtr SumGrad(const BpropIRBuilder *ib, const NodePtr &x, const NodePtr &axis, const NodePtr &dout);
NodePtr MinOrMaxGrad(const BpropIRBuilder *ib, const NodePtr &x, const NodePtr &axis, const NodePtr &out,
                     const NodePtr &dout);
std::pair<ShapeVector, ShapeVector> SplitShapeIndex(const ShapeVector &input_shape, const ShapeVector &axis);
NodePtr ArgminOrArgmaxGrad(const BpropIRBuilder *ib, const NodePtr &x, const int64_t &axis, const bool &keep_dims,
                           const NodePtr &out, const NodePtr &dout, const bool is_max);
TypeId PromoteBinaryDtype(TypeId t1, TypeId t2);
NodePtr LGamma(const BpropIRBuilder *ib, const NodePtr &x);
bool CheckType(const TypePtr &check_type, const std::set<TypePtr> &template_types);
ShapeVector PoolToNHWC(const ShapeVector &v);
ShapeVector ConvToNHWC(const ShapeVector &v);
}  // namespace mindspore::expander::bprop
#endif  // MINDSPORE_CCSRC_PIPELINE_PYNATIVE_GRAD_BPROP_EXPANDER_GRAD_OPS_COMMON_UTILS_H_
