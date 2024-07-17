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
#include "./utils/utilCallJs.h"
#include <future>

Broadcaster broadcaster;
// broadcaster.Start(baseUrl, enableAudio, useSimulcast, response, verifySsl);

using json = nlohmann::json;

napi_threadsafe_function tsfn; // 线程安全函数
static int g_cValue;           // 保存value最新的值,作为参数传给js回调函数
int g_threadNum = 3;           // 线程数

struct CallbackContext {
    napi_async_work asyncWork = nullptr;
    napi_env env = nullptr;
    napi_ref callbackRef = nullptr;
    int retData = 0;
};

// 安全函数回调
static void ThreadSafeCallJs(napi_env env, napi_value js_cb, void *context, void *data) {
    CallbackContext *argContent = (CallbackContext *)data;
    if (argContent != nullptr) {
        OH_LOG_INFO(LOG_APP, "ThreadSafeTest CallJs start, retData:[%{public}d]", argContent->retData);
        napi_get_reference_value(env, argContent->callbackRef, &js_cb);
    } else {
        OH_LOG_INFO(LOG_APP, "ThreadSafeTest CallJs argContent is null");
        return;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, js_cb, &valueType);
    if (valueType != napi_valuetype::napi_function) {
        OH_LOG_ERROR(LOG_APP, "ThreadSafeTest callback param is not function");
        if (argContent != nullptr) {
            napi_delete_reference(env, argContent->callbackRef);
            delete argContent;
            argContent = nullptr;
            OH_LOG_INFO(LOG_APP, "ThreadSafeTest delete argContent");
        }
        return;
    }
    // 将当前value值作为参数调用js函数
    napi_value argv;
    napi_create_int32(env, g_cValue, &argv);
    napi_value result = nullptr;
    napi_call_function(env, nullptr, js_cb, 1, &argv, &result);
    // g_cValue保存调用js后的返回结果
    napi_get_value_int32(env, result, &g_cValue);
    OH_LOG_INFO(LOG_APP, "ThreadSafeTest CallJs end, [%{public}d]", g_cValue);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "ThreadSafeTest CallJs end, [%{public}d]", g_cValue);

    if (argContent != nullptr) {
        napi_delete_reference(env, argContent->callbackRef);
        delete argContent;
        argContent = nullptr;
        OH_LOG_INFO(LOG_APP, "ThreadSafeTest delete argContent end");
    }
}

static napi_value InitMediasoup(napi_env env, napi_callback_info info) {
    auto logLevel = mediasoupclient::Logger::LogLevel::LOG_DEBUG;
    mediasoupclient::Logger::SetLogLevel(logLevel);
    mediasoupclient::Logger::SetDefaultHandler();

    // Initilize mediasoupclient.
    mediasoupclient::Initialize();
    napi_value result;
    
    utilCallJs* calljs = new utilCallJs;
    calljs->loadJs(env, info);
    broadcaster.getProduceId = calljs;

    napi_create_int64(env, 1, &result);
    return result;
}

// static napi_value GetMediasoupDevice(napi_env env, napi_callback_info info) {
//     OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "GetMediasoupDevice\n");
//
//     // 获取参数
//     size_t argc = 1;
//     napi_value args[1] = {nullptr};
//     napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
//
//     size_t result1;
//     napi_get_value_string_utf8(env, args[0], nullptr, 0, &result1);
//     if (result1 == 0) {
//         return nullptr;
//     }
//     char *test = new char[result1 + 1];
//     napi_get_value_string_utf8(env, args[0], test, result1 + 1, &result1);
//
//     auto routerRtpCapabilities = nlohmann::json::parse(test);
//
//     const nlohmann::json roomRtpCapabilities = broadcaster.Start(true, false, routerRtpCapabilities);
//
//     OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "GetMediasoupDevice %{public}s \n",
//                  roomRtpCapabilities.dump().c_str());
//
//     napi_value result;
//     napi_create_string_utf8(env, roomRtpCapabilities.dump().c_str(), roomRtpCapabilities.dump().length(), &result);
//     return result;
// }

static napi_value GetMediasoupDevice(napi_env env, napi_callback_info info) {
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

    broadcaster.Start(true, false, routerRtpCapabilities);
    
    napi_value result;
    napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &result);
    return result;
}

static napi_value ConnectMediastream(napi_env env, napi_callback_info info) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "GetMediasoupDevice %{public}u\n",std::this_thread::get_id());

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

    nlohmann::json routerRtpCapabilities = nlohmann::json::parse(test);
//     int res = broadcaster.CreateTransport(routerRtpCapabilities);

    
    std::thread t(&Broadcaster::CreateTransport,std::ref(broadcaster),routerRtpCapabilities);
    t.detach();
    napi_value result;
    napi_create_int64(env, 1, &result);
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    CallbackData *callbackData = new CallbackData();
    napi_property_descriptor desc[] = {
        {"getMediasoupDevice", nullptr, GetMediasoupDevice, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"initMediasoup", nullptr, InitMediasoup, nullptr, nullptr, nullptr, napi_default, callbackData},
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
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) { napi_module_register(&demoModule); }
