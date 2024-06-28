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
#include "Broadcaster.h"
#include "json.hpp"
// #include "httplib.h"

Broadcaster broadcaster;
// broadcaster.Start(baseUrl, enableAudio, useSimulcast, response, verifySsl);

using json = nlohmann::json;

static napi_value InitMediasoup(napi_env env,napi_callback_info info) {
    auto logLevel = mediasoupclient::Logger::LogLevel::LOG_DEBUG;
    mediasoupclient::Logger::SetLogLevel(logLevel);
    mediasoupclient::Logger::SetDefaultHandler();

    // Initilize mediasoupclient.
    mediasoupclient::Initialize();
    napi_value result;
    napi_create_int64(env, 1, &result);
    return result;
}

static napi_value GetMediasoupDevice(napi_env env,napi_callback_info info) {
  OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "GetMediasoupDevice\n");
    
  // 获取参数
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  size_t result1;
  napi_get_value_string_utf8(env, args[0], nullptr, 0, &result1);
  if (result1 == 0) {
    return nullptr;
    }
    char * test = new char[result1 + 1];
    napi_get_value_string_utf8(env, args[0], test, result1 + 1, &result1);

    auto routerRtpCapabilities = nlohmann::json::parse(test);
    
    const nlohmann::json roomRtpCapabilities = broadcaster.Start(true,  false, routerRtpCapabilities);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest",  "GetMediasoupDevice %{public}s \n",roomRtpCapabilities.dump().c_str());
    
    napi_value result;
    napi_create_string_utf8(env, roomRtpCapabilities.dump().c_str(), roomRtpCapabilities.dump().length(), &result);
    return result;
}

static napi_value ConnectMediastream(napi_env env, napi_callback_info info) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "GetMediasoupDevice\n");

    // 获取参数
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    size_t result1;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &result1);
    if (result1 == 0) {
    return nullptr;
    }
    char *test = new char[result1 + 1];
    napi_get_value_string_utf8(env, args[0], test, result1 + 1, &result1);

    auto routerRtpCapabilities = nlohmann::json::parse(test);
    int res = broadcaster.CreateTransport(routerRtpCapabilities);

    napi_value result;
    napi_create_int64(env,res, &result);
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
  napi_property_descriptor desc[] = {
    {"getMediasoupDevice", nullptr, GetMediasoupDevice, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"initMediasoup", nullptr, InitMediasoup, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"connectMediastream", nullptr, ConnectMediastream, nullptr, nullptr, nullptr, napi_default, nullptr},
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
