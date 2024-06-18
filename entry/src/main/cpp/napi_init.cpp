/*
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#include "napi/native_api.h"
#include "hilog/log.h"

#include "mediasoupclient.hpp"
#include <arm-linux-ohos/bits/alltypes.h>
#include <multimedia/image_framework/image_mdk.h>
#include <multimedia/image_framework/image_receiver_mdk.h>
#include <malloc.h>

#include "json.hpp"
#include "my_sum.h"
using json = nlohmann::json;


static napi_value GetMediasoupDevice(napi_env env,napi_callback_info info) {
  OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "GetMediasoupDevice\n");

  // 获取参数
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  size_t result;
  napi_get_value_string_utf8(env, args[0], nullptr, 0, &result);
  if (result == 0) {
    return nullptr;
    }
    char * test = new char[result + 1];
    napi_get_value_string_utf8(env, args[0], test, result + 1, &result);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "GetMediasoupDevice %{public}s \n",test);
    auto routerRtpCapabilities = nlohmann::json::parse(test);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "routerRtpCapabilities: %{public}s \n",routerRtpCapabilities.dump().c_str());

    auto logLevel = mediasoupclient::Logger::LogLevel::LOG_DEBUG;
    mediasoupclient::Logger::SetLogLevel(logLevel);
    mediasoupclient::Logger::SetDefaultHandler();

    // Initilize mediasoupclient.
    mediasoupclient::Initialize();
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "创建Device \n");
    mediasoupclient::Device* device = new mediasoupclient::Device(); 
    try {
        int num = device->Load(routerRtpCapabilities);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "load %{public}d \n",num);
    } catch (...) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "catch \n");
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "Load! \n");

    napi_value result2;
    std::string rtpCapabilities = device->GetRtpCapabilities().dump();
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "-----------------! %{public}s \n",rtpCapabilities.c_str());

    //     napi_create_string_utf8(env, rtpCapabilities, strlen(rtpCapabilities), &result2);
    //     OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "-----------------! %{public}d %{public}s \n",
    //     strlen(rtpCapabilities),result2);
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
  napi_property_descriptor desc[] = {
    {"getMediasoupDevice", nullptr, GetMediasoupDevice, nullptr, nullptr, nullptr, napi_default, nullptr},
  };
  napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
  return exports;
}
EXTERN_C_END

static napi_module demoModule = {
  .nm_version = 1,
  .nm_flags = 0,
  .nm_filename = nullptr,
  .nm_register_func = Init,
  .nm_modname = "entry",
  .nm_priv = ((void*)0),
  .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void)
{
  napi_module_register(&demoModule);
}
