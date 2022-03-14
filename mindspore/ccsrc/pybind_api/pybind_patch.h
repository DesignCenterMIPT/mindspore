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
#ifndef PYBIND_API_PYBIND_PATCH_H_
#define PYBIND_API_PYBIND_PATCH_H_

namespace pybind11 {
#if (PYBIND11_VERSION_MAJOR < 2 ||   \
     (PYBIND11_VERSION_MAJOR == 2 && \
      (PYBIND11_VERSION_MINOR < 8 || (PYBIND11_VERSION_MINOR == 8 && PYBIND11_VERSION_PATCH < 1))))
PYBIND11_RUNTIME_EXCEPTION(attribute_error, PyExc_AttributeError)
#endif  // Pybind11 <= 2.8.1
PYBIND11_RUNTIME_EXCEPTION(name_error, PyExc_NameError)
}  // namespace pybind11

#endif  // PYBIND_API_PYBIND_PATCH_H_
