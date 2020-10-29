# Copyright 2019 Huawei Technologies Co., Ltd
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

import mindspore as ms
import mindspore.nn as nn
from mindspore.common.api import _executor
from mindspore.ops import operations as P
from mindspore.ops import composite as C
from mindspore import Tensor, context
from mindspore.nn import TrainOneStepCell, Adam
from tests.ut.python.ops.test_math_ops import VirtualLoss


grad_all = C.GradOperation(get_all=True)


class GradWrap(nn.Cell):
    def __init__(self, network):
        super(GradWrap, self).__init__()
        self.network = network

    def construct(self, x, y, z):
        return grad_all(self.network)(x, y, z)


class NetWithLoss(nn.Cell):
    def __init__(self, network):
        super(NetWithLoss, self).__init__()
        self.loss = VirtualLoss()
        self.network = network

    def construct(self, x, y, z):
        predict = self.network(x, y, z)
        return self.loss(predict)


class Net(nn.Cell):
    def __init__(self, shape, slice_mode=nn.EmbeddingLookup.BATCH_SLICE, target="Device", operator='SUM'):
        super().__init__()
        self.embedding = nn.MultiFieldEmbeddingLookup(vocab_size=32, embedding_size=64, target=target,
                                                      field_size=shape[1], slice_mode=slice_mode, operator=operator)
        self.reshape = P.Reshape().shard(((8, 1, 1),))
        self.batch_size = shape[0]

    def construct(self, x, y, z):
        out = self.embedding(x, y, z)
        out = self.reshape(out, (self.batch_size, -1))
        return out


def compile_net(net, shape):
    context.set_context(enable_sparse=True)
    x = Tensor(np.ones(shape), dtype=ms.int32)
    y = Tensor(np.ones(shape), dtype=ms.float32)
    z = Tensor(np.ones(shape), dtype=ms.int32)
    optimizer = Adam(net.trainable_params(), learning_rate=0.1)
    train_net = TrainOneStepCell(net, optimizer)
    train_net.set_auto_parallel()
    train_net.set_train()
    _executor.compile(train_net, x, y, z)
    context.reset_auto_parallel_context()


def test_embeddinglookup_batch_parallel_sum():
    context.set_auto_parallel_context(device_num=8, global_rank=0, parallel_mode="semi_auto_parallel")
    shape = [64, 64]
    net = NetWithLoss(Net(shape, target='DEVICE'))
    compile_net(net, shape)


def test_embeddinglookup_row_parallel_sum():
    context.set_auto_parallel_context(device_num=8, global_rank=0, parallel_mode="semi_auto_parallel")
    shape = [64, 64]
    net = NetWithLoss(Net(shape, slice_mode=nn.EmbeddingLookup.TABLE_ROW_SLICE, target='DEVICE'))
    compile_net(net, shape)


def test_embeddinglookup_column_parallel_sum():
    context.set_auto_parallel_context(device_num=8, global_rank=0, parallel_mode="semi_auto_parallel")
    shape = [64, 64]
    net = NetWithLoss(Net(shape, slice_mode=nn.EmbeddingLookup.TABLE_COLUMN_SLICE, target='DEVICE'))
    compile_net(net, shape)


def test_embeddinglookup_batch_parallel_mean():
    context.set_auto_parallel_context(device_num=8, global_rank=0, parallel_mode="semi_auto_parallel")
    shape = [64, 64]
    net = NetWithLoss(Net(shape, target='DEVICE', operator='MEAN'))
    compile_net(net, shape)


def test_embeddinglookup_column_parallel_mean():
    context.set_auto_parallel_context(device_num=8, global_rank=0, parallel_mode="semi_auto_parallel")
    shape = [64, 64]
    net = NetWithLoss(Net(shape, target='DEVICE', slice_mode=nn.EmbeddingLookup.TABLE_COLUMN_SLICE, operator='MEAN'))
    compile_net(net, shape)


def test_embeddinglookup_row_parallel_mean():
    context.set_auto_parallel_context(device_num=8, global_rank=0, parallel_mode="semi_auto_parallel")
    shape = [64, 64]
    net = NetWithLoss(Net(shape, target='DEVICE', slice_mode=nn.EmbeddingLookup.TABLE_ROW_SLICE, operator='MEAN'))
    compile_net(net, shape)


def test_embeddinglookup_batch_parallel_max():
    context.set_auto_parallel_context(device_num=8, global_rank=0, parallel_mode="semi_auto_parallel")
    shape = [64, 64]
    net = NetWithLoss(Net(shape, target='DEVICE', operator='MAX'))
    compile_net(net, shape)


def test_embeddinglookup_column_parallel_max():
    context.set_auto_parallel_context(device_num=8, global_rank=0, parallel_mode="semi_auto_parallel")
    shape = [64, 64]
    net = NetWithLoss(Net(shape, target='DEVICE', slice_mode=nn.EmbeddingLookup.TABLE_COLUMN_SLICE, operator='MAX'))
    compile_net(net, shape)


def test_embeddinglookup_row_parallel_max():
    context.set_auto_parallel_context(device_num=8, global_rank=0, parallel_mode="semi_auto_parallel")
    shape = [64, 64]
    net = NetWithLoss(Net(shape, target='DEVICE', slice_mode=nn.EmbeddingLookup.TABLE_ROW_SLICE, operator='MAX'))
    compile_net(net, shape)
