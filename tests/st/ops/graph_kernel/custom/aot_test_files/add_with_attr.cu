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

#include <vector>
#include "custom_aot_extra.h"

constexpr int THREADS = 1024;
__global__ void CustomAddKernel(float *input1, float *input2, float *output, float *tmp, float scale, float pad_u,
                                float pad_d, size_t size) {
  auto idx = blockIdx.x * THREADS + threadIdx.x;
  // Add with attr
  if (idx < size) {
    tmp[idx] = input1[idx] + input2[idx] * scale + pad_u;
    output[idx] = tmp[idx] + input2[idx] * scale + pad_d;
  }
}

class add_kernel_attr : public AotKernelData {
 public:
  float scale;
  std::vector<float> paddings;
};

void foo_destroy(void *kernel_ptr) { delete static_cast<add_kernel_attr *>(kernel_ptr); }

extern "C" int CustomAddInit(int *ndims, int64_t **shapes, const char **dtypes, AotExtra *extra) {
  size_t workspace_size = 1;
  for (size_t i = 0; i < ndims[0]; i++) {
    workspace_size *= shapes[0][i];
  }

  extra->SetWorkSpace({workspace_size * sizeof(float)});

  add_kernel_attr *kernel_ptr = new add_kernel_attr;
  kernel_ptr->scale = extra->Attr<float>("scale");
  kernel_ptr->paddings = extra->Attr<std::vector<float>>("paddings");
  extra->SetKernelData(kernel_ptr);

  return 0;
}

extern "C" int CustomAdd(int nparam, void **params, int *ndims, int64_t **shapes, const char **dtypes, void *stream,
                         void *extra_void) {
  cudaStream_t custream = static_cast<cudaStream_t>(stream);
  constexpr int OUTPUT_INDEX = 2;

  float *input1 = static_cast<float *>(params[0]);
  float *input2 = static_cast<float *>(params[1]);
  float *output = static_cast<float *>(params[2]);
  float *tmp = static_cast<float *>(params[3]);
  int size = 1;

  AotExtra *extra = static_cast<AotExtra *>(extra_void);
  for (int i = 0; i < ndims[OUTPUT_INDEX]; i++) {
    size *= shapes[OUTPUT_INDEX][i];
  }
  int n = size / THREADS;

  // Cumprod of output's shape to compute elements' num
  for (int i = 0; i < ndims[OUTPUT_INDEX]; i++) {
    size *= shapes[OUTPUT_INDEX][i];
  }
  auto kernel_ptr = static_cast<add_kernel_attr *>(extra->KernelData());

  float scale = kernel_ptr->scale;
  float paddings_up = kernel_ptr->paddings[0];
  float paddings_down = kernel_ptr->paddings[1];

  // Do the computation
  CustomAddKernel<<<n + 1, THREADS, 0, custream>>>(input1, input2, output, tmp, scale, paddings_up, paddings_down,
                                                   size);
  return 0;
}
