# Copyright 2020 Huawei Technologies Co., Ltd
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
"""ut for model serialize(save/load)"""
import os
import stat
import numpy as np
import pytest

import mindspore.nn as nn
from mindspore import context
from mindspore.common.parameter import Parameter
from mindspore.common.tensor import Tensor
from mindspore.ops import operations as P
from mindspore.train.serialization import export

context.set_context(mode=context.GRAPH_MODE)


def is_enable_onnxruntime():
    val = os.getenv("ENABLE_ONNXRUNTIME", "False")
    if val in ('ON', 'on', 'TRUE', 'True', 'true'):
        return True
    return False


run_on_onnxruntime = pytest.mark.skipif(not is_enable_onnxruntime(), reason="Only support running on onnxruntime")


def setup_module():
    pass


def teardown_module():
    cur_dir = os.path.dirname(os.path.realpath(__file__))
    for filename in os.listdir(cur_dir):
        if filename.find('ms_output_') == 0 and filename.find('.pb') > 0:
            # delete temp files generated by run ut
            os.chmod(filename, stat.S_IWRITE)
            os.remove(filename)


class BatchNormTester(nn.Cell):
    "used to test exporting network in training mode in onnx format"

    def __init__(self, num_features):
        super(BatchNormTester, self).__init__()
        self.bn = nn.BatchNorm2d(num_features)

    def construct(self, x):
        return self.bn(x)


def test_batchnorm_train_onnx_export():
    "test onnx export interface does not modify trainable flag of a network"
    input = Tensor(np.ones([1, 3, 32, 32]).astype(np.float32) * 0.01)
    net = BatchNormTester(3)
    net.set_train()
    if not net.training:
        raise ValueError('netowrk is not in training mode')
    onnx_file = 'batch_norm.onnx'
    export(net, input, file_name=onnx_file, file_format='ONNX')

    if not net.training:
        raise ValueError('netowrk is not in training mode')
    # check existence of exported onnx file and delete it
    assert os.path.exists(onnx_file)
    os.chmod(onnx_file, stat.S_IWRITE)
    os.remove(onnx_file)


class LeNet5(nn.Cell):
    """LeNet5 definition"""

    def __init__(self):
        super(LeNet5, self).__init__()
        self.conv1 = nn.Conv2d(1, 6, 5, pad_mode='valid')
        self.conv2 = nn.Conv2d(6, 16, 5, pad_mode='valid')
        self.fc1 = nn.Dense(16 * 5 * 5, 120)
        self.fc2 = nn.Dense(120, 84)
        self.fc3 = nn.Dense(84, 10)
        self.relu = nn.ReLU()
        self.max_pool2d = nn.MaxPool2d(kernel_size=2, stride=2)
        self.flatten = P.Flatten()

    def construct(self, x):
        x = self.max_pool2d(self.relu(self.conv1(x)))
        x = self.max_pool2d(self.relu(self.conv2(x)))
        x = self.flatten(x)
        x = self.relu(self.fc1(x))
        x = self.relu(self.fc2(x))
        x = self.fc3(x)
        return x


class DefinedNet(nn.Cell):
    """simple Net definition with maxpoolwithargmax."""

    def __init__(self, num_classes=10):
        super(DefinedNet, self).__init__()
        self.conv1 = nn.Conv2d(3, 64, kernel_size=7, stride=2, padding=0, weight_init="zeros")
        self.bn1 = nn.BatchNorm2d(64)
        self.relu = nn.ReLU()
        self.maxpool = P.MaxPoolWithArgmax(padding="same", ksize=2, strides=2)
        self.flatten = nn.Flatten()
        self.fc = nn.Dense(int(56 * 56 * 64), num_classes)

    def construct(self, x):
        x = self.conv1(x)
        x = self.bn1(x)
        x = self.relu(x)
        x, argmax = self.maxpool(x)
        x = self.flatten(x)
        x = self.fc(x)
        return x


class DepthwiseConv2dAndReLU6(nn.Cell):
    "Net for testing DepthwiseConv2d and ReLU6"

    def __init__(self, input_channel, kernel_size):
        super(DepthwiseConv2dAndReLU6, self).__init__()
        weight_shape = [1, input_channel, kernel_size, kernel_size]
        from mindspore.common.initializer import initializer
        self.weight = Parameter(initializer('ones', weight_shape), name='weight')
        self.depthwise_conv = P.DepthwiseConv2dNative(channel_multiplier=1, kernel_size=(kernel_size, kernel_size))
        self.relu6 = nn.ReLU6()

    def construct(self, x):
        x = self.depthwise_conv(x, self.weight)
        x = self.relu6(x)
        return x


# generate mindspore Tensor by shape and numpy datatype
def gen_tensor(shape, dtype=np.float32):
    return Tensor(np.ones(shape).astype(dtype))


# ut configs in triple: (ut_name, network, network-input)
net_cfgs = [
    ('lenet', LeNet5(), gen_tensor([1, 1, 32, 32])),
    ('maxpoolwithargmax', DefinedNet(), gen_tensor([1, 3, 224, 224])),
    ('depthwiseconv_relu6', DepthwiseConv2dAndReLU6(3, kernel_size=3), gen_tensor([1, 3, 32, 32])),
]


def get_id(cfg):
    return list(map(lambda x: x[0], net_cfgs))


# use `pytest test_onnx.py::test_onnx_export[name]` or `pytest test_onnx.py::test_onnx_export -k name` to run single ut
@pytest.mark.parametrize('name, net, inp', net_cfgs, ids=get_id(net_cfgs))
def test_onnx_export(name, net, inp):
    onnx_file = name + ".onnx"
    export(net, inp, file_name=onnx_file, file_format='ONNX')

    # check existence of exported onnx file and delete it
    assert os.path.exists(onnx_file)
    os.chmod(onnx_file, stat.S_IWRITE)
    os.remove(onnx_file)


@run_on_onnxruntime
@pytest.mark.parametrize('name, net, inp', net_cfgs, ids=get_id(net_cfgs))
def test_onnx_export_load_run(name, net, inp):
    onnx_file = name + ".onnx"
    export(net, inp, file_name=onnx_file, file_format='ONNX')

    import onnx
    import onnxruntime as ort

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
    input_map = {'x': inp.asnumpy()}
    # provide only input x to run model
    outputs = ort_session.run(None, input_map)
    print(outputs[0])
    # overwrite default weight to run model
    for item in net.trainable_params():
        default_value = item.default_input.asnumpy()
        input_map[item.name] = np.ones(default_value.shape, dtype=default_value.dtype)
    outputs = ort_session.run(None, input_map)
    print(outputs[0])

    # check existence of exported onnx file and delete it
    assert os.path.exists(onnx_file)
    os.chmod(onnx_file, stat.S_IWRITE)
    os.remove(onnx_file)
