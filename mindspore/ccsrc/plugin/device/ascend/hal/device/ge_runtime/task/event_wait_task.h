/**
 * Copyright 2019-2020 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_CCSRC_RUNTIME_DEVICE_ASCEND_GE_RUNTIME_EVENT_WAIT_TASK_H_
#define MINDSPORE_CCSRC_RUNTIME_DEVICE_ASCEND_GE_RUNTIME_EVENT_WAIT_TASK_H_

#include <memory>
#include <string>
#include "plugin/device/ascend/hal/device/ge_runtime/task/task.h"

namespace mindspore::ge::model_runner {
class EventWaitTask : public TaskRepeater<EventWaitTaskInfo> {
 public:
  EventWaitTask(const ModelContext &model_context, const std::shared_ptr<EventWaitTaskInfo> &task_info);

  ~EventWaitTask() override;

  void Distribute() override;

  std::string task_name() const override { return task_info_->op_name(); }

 private:
  std::shared_ptr<EventWaitTaskInfo> task_info_;
  rtStream_t stream_;
  rtEvent_t event_;
};
}  // namespace mindspore::ge::model_runner
#endif  // MINDSPORE_CCSRC_RUNTIME_DEVICE_ASCEND_GE_RUNTIME_EVENT_WAIT_TASK_H_
