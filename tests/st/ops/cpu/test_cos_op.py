# Copyright 2021-2022 Huawei Technologies Co., Ltd
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

import sys
import os
import stat
import numpy as np
import pytest

import mindspore as ms
import mindspore.nn as nn
from mindspore import Tensor
from mindspore import context
from mindspore.ops import operations as P
from mindspore.train.serialization import export

context.set_context(mode=context.GRAPH_MODE, device_target="CPU")


class NetCos(nn.Cell):
    def __init__(self):
        super(NetCos, self).__init__()
        self.cos = P.Cos()

    def construct(self, x):
        return self.cos(x)


@pytest.mark.level0
@pytest.mark.platform_x86_cpu
@pytest.mark.env_onecard
def test_cos_dshape():
    """
    Feature: Test Cos dynamic shape.
    Description: Test Cos dynamic shape.
    Expectation: Success.
    """
    net = NetCos()
    input_x_dyn = Tensor(shape=[3, None], dtype=ms.float32)
    net.set_inputs(input_x_dyn)
    input_x = Tensor(np.random.random(([3, 10])), dtype=ms.float32)
    output = net(input_x)
    expect_shape = (3, 10)
    assert output.asnumpy().shape == expect_shape


@pytest.mark.level0
@pytest.mark.platform_x86_cpu
@pytest.mark.env_onecard
def test_cos():
    np_array = np.array([-1, -0.5, 0, 0.5, 1]).astype('float32')
    input_x = Tensor(np_array)
    net = NetCos()
    output = net(input_x)
    print(output)
    expect = np.cos(np_array)
    assert np.allclose(output.asnumpy(), expect)

    np_array = np.array([-1, -0.5, 0, 0.5, 1]).astype('float64')
    input_x = Tensor(np_array)
    net = NetCos()
    output = net(input_x)
    print(output)
    expect = np.cos(np_array)
    assert np.allclose(output.asnumpy(), expect)


@pytest.mark.level0
@pytest.mark.platform_x86_cpu
@pytest.mark.env_onecard
@pytest.mark.parametrize('dtype', [np.float32, np.float64])
def test_cos_tensor(dtype):
    """
    Feature: cos tensor interface
    Description: test the rightness of cos tensor interface
    Expectation: Success.
    """
    np_array = np.array([-1, -0.5, 0, 0.5, 1]).astype(dtype)
    input_x = Tensor(np_array)
    output = input_x.cos()
    print(output)
    expect = np.cos(np_array)
    assert np.allclose(output.asnumpy(), expect)


@pytest.mark.level1
@pytest.mark.platform_x86_cpu
# MIPT: skip this test
@pytest.mark.skipif('PyPy' in sys.version, reason="onnx is not supported")
def test_onnx_export_load_run_ops_cos():
    """
    Feature: Export onnx cos
    Description: Export Onnx file and verify the result of onnx Cos
    Expectation: success
    """
    import onnx
    import onnxruntime as ort

    x = Tensor(np.array([-1, -0.5, 0, 0.5, 1]), ms.float32)
    net = NetCos()
    ms_output = net(x)
    onnx_file = "NetCos.onnx"
    export(net, x, file_name=onnx_file, file_format='ONNX')

    print('--------------------- onnx load ---------------------')
    # Load the ONNX model
    model = onnx.load(onnx_file)
    # Check that the IR is well formed
    onnx.checker.check_model(model)
    # Print a human readable representation of the graph
    g = onnx.helper.printable_graph(model.graph)
    print(g)

    print('------------------ onnxruntime run ------------------')
    ort_session = ort.InferenceSession(onnx_file)
    input_x = ort_session.get_inputs()[0].name
    input_map = {input_x: x.asnumpy()}
    onnx_outputs = ort_session.run(None, input_map)
    print(onnx_outputs[0])
    assert np.allclose(ms_output.asnumpy(), onnx_outputs[0])
    assert os.path.exists(onnx_file)
    os.chmod(onnx_file, stat.S_IWRITE)
    os.remove(onnx_file)
