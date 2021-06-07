# Copyright 2021 Huawei Technologies Co., Ltd
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
"""hub config."""
import mindspore.common.dtype as mstype
from src.retinahead import retinanetWithLossCell, retinahead
from src.backbone import resnet101
from src.config import config


def create_network(name, *args, **kwargs):
    if name == "retinanet_resnet101":
        backbone = resnet101()
        retinaNet = retinahead(backbone, config)
        net = retinanetWithLossCell(retinaNet, config)
        net.to_float(mstype.float16)
        return net
    raise NotImplementedError(f"{name} is not implemented in the repo")
