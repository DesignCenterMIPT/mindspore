# Copyright 2022 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================
import numpy as np
import pytest
import mindspore as ms
from mindspore import ops
import mindspore.nn as nn
from tests.st.numpy_native.utils import check_all_results, to_tensor


class NetSwapDims(nn.Cell):
    def construct(self, x, dim0, dim1):
        return ops.swapdims(x, dim0=dim0, dim1=dim1)


@pytest.mark.level1
@pytest.mark.platform_x86_cpu
@pytest.mark.platform_arm_cpu
@pytest.mark.platform_x86_gpu_training
@pytest.mark.platform_x86_ascend_training
@pytest.mark.platform_arm_ascend_training
@pytest.mark.env_onecard
@pytest.mark.parametrize('mode', [ms.GRAPH_MODE, ms.PYNATIVE_MODE])
def test_swapdims(mode):
    """
    Feature: swapdims
    Description: Verify the result of swapdims
    Expectation: success.
    """
    ms.set_context(mode=mode)
    np_array = np.random.random((3, 4, 5)).astype('float32')

    np_swap_output = []
    np_swap_output.append(np.swapaxes(np_array, 0, 1))
    np_swap_output.append(np.swapaxes(np_array, 1, 0))
    np_swap_output.append(np.swapaxes(np_array, 1, 1))
    np_swap_output.append(np.swapaxes(np_array, 2, 1))
    np_swap_output.append(np.swapaxes(np_array, 1, 2))
    np_swap_output.append(np.swapaxes(np_array, 2, 2))

    swapdims_op = NetSwapDims()
    op_swap_output = []
    ms_array = to_tensor(np_array)
    op_swap_output.append(swapdims_op(ms_array, 0, 1))
    op_swap_output.append(swapdims_op(ms_array, 1, 0))
    op_swap_output.append(swapdims_op(ms_array, 1, 1))
    op_swap_output.append(swapdims_op(ms_array, 2, 1))
    op_swap_output.append(swapdims_op(ms_array, 1, 2))
    op_swap_output.append(swapdims_op(ms_array, 2, 2))

    check_all_results(np_swap_output, op_swap_output)
