# Copyright 2020 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# less required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================

"""Evaluation for SSD"""
import os
import argparse
import time
from mindspore import context, Tensor
from mindspore.train.serialization import load_checkpoint, load_param_into_net
from mindspore.model_zoo.ssd import SSD300, ssd_mobilenet_v2
from dataset import create_ssd_dataset, data_to_mindrecord_byte_image
from config import ConfigSSD
from util import metrics

def ssd_eval(dataset_path, ckpt_path):
    """SSD evaluation."""

    ds = create_ssd_dataset(dataset_path, batch_size=1, repeat_num=1, is_training=False)
    net = SSD300(ssd_mobilenet_v2(), ConfigSSD(), is_training=False)
    print("Load Checkpoint!")
    param_dict = load_checkpoint(ckpt_path)
    load_param_into_net(net, param_dict)

    net.set_train(False)
    i = 1.
    total = ds.get_dataset_size()
    start = time.time()
    pred_data = []
    print("\n========================================\n")
    print("total images num: ", total)
    print("Processing, please wait a moment.")
    for data in ds.create_dict_iterator():
        img_np = data['image']
        image_shape = data['image_shape']
        annotation = data['annotation']

        output = net(Tensor(img_np))
        for batch_idx in range(img_np.shape[0]):
            pred_data.append({"boxes": output[0].asnumpy()[batch_idx],
                              "box_scores": output[1].asnumpy()[batch_idx],
                              "annotation": annotation,
                              "image_shape": image_shape})
        percent = round(i / total * 100, 2)

        print(f'    {str(percent)} [{i}/{total}]', end='\r')
        i += 1
    cost_time = int((time.time() - start) * 1000)
    print(f'    100% [{total}/{total}] cost {cost_time} ms')
    mAP = metrics(pred_data)
    print("\n========================================\n")
    print(f"mAP: {mAP}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='SSD evaluation')
    parser.add_argument("--device_id", type=int, default=0, help="Device id, default is 0.")
    parser.add_argument("--dataset", type=str, default="coco", help="Dataset, default is coco.")
    parser.add_argument("--checkpoint_path", type=str, required=True, help="Checkpoint file path.")
    args_opt = parser.parse_args()

    context.set_context(mode=context.GRAPH_MODE, device_target="Ascend", device_id=args_opt.device_id)
    context.set_context(enable_task_sink=True, enable_loop_sink=True, enable_mem_reuse=True)

    config = ConfigSSD()
    prefix = "ssd_eval.mindrecord"
    mindrecord_dir = config.MINDRECORD_DIR
    mindrecord_file = os.path.join(mindrecord_dir, prefix + "0")
    if not os.path.exists(mindrecord_file):
        if not os.path.isdir(mindrecord_dir):
            os.makedirs(mindrecord_dir)
        if args_opt.dataset == "coco":
            if os.path.isdir(config.COCO_ROOT):
                print("Create Mindrecord.")
                data_to_mindrecord_byte_image("coco", False, prefix)
                print("Create Mindrecord Done, at {}".format(mindrecord_dir))
            else:
                print("COCO_ROOT not exits.")
        else:
            if os.path.isdir(config.IMAGE_DIR) and os.path.exists(config.ANNO_PATH):
                print("Create Mindrecord.")
                data_to_mindrecord_byte_image("other", False, prefix)
                print("Create Mindrecord Done, at {}".format(mindrecord_dir))
            else:
                print("IMAGE_DIR or ANNO_PATH not exits.")

    print("Start Eval!")
    ssd_eval(mindrecord_file, args_opt.checkpoint_path)
