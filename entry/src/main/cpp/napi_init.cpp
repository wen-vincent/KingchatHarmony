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
#include "api/scoped_refptr.h"
#include "pc/video_track_source.h"
#include "ohos_capturer_track_source.h"
#include "ohos_camera_capture.h"
#include "ohos_camera.h"
#include "mediasoupclient.hpp"
#include <arm-linux-ohos/bits/alltypes.h>
#include <multimedia/image_framework/image_mdk.h>
#include <multimedia/image_framework/image_receiver_mdk.h>
#include <malloc.h>
#include "client/ohos/peer_sample.h"
#include "json.hpp"
#include "my_sum.h"
using json = nlohmann::json;



static napi_value CreateFromReceiver(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  napi_valuetype value_type;
  napi_typeof(env, args[0], &value_type);
  napi_ref reference;
  napi_create_reference(env, args[0], 1, &reference);
  napi_value img_receiver_js;
  napi_get_reference_value(env, reference, &img_receiver_js);

  ImageReceiverNative *img_receiver_c = OH_Image_Receiver_InitImageReceiverNative(env, img_receiver_js);
  napi_value next_image = webrtc::ohos::OhosCamera::GetInstance().GetImageData(env, img_receiver_c);
  
  int32_t ret = OH_Image_Receiver_Release(img_receiver_c);
  if (ret != 0) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_Image_Receiver_Release failed");
    return nullptr;
  }
  return next_image;
}

static napi_value GetMediasoupDevice(napi_env env,napi_callback_info info) {
  OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "GetMediasoupDevice %{public}s \n");

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
    mediasoupclient::Device device; 
    try {
        int num = device.Load(routerRtpCapabilities);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "load %{public}d \n",num);
    } catch (...) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "catch \n");
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "Load! \n");

    napi_value result2;
    auto rtpCapabilities = device.GetRtpCapabilities().dump();
    napi_create_string_utf8(env, rtpCapabilities.c_str(), rtpCapabilities.length(), &result2);
    return result2;
    
}

static napi_value InitCamera(napi_env env, napi_callback_info info){
  webrtc::ohos::OhosCamera::GetInstance().Init(env, info);
  rtc::scoped_refptr<webrtc::ohos::CapturerTrackSource> ohos_cts = webrtc::ohos::CapturerTrackSource::Create();
  
  OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "had been try create");
  return nullptr;
}

static napi_value StopCamera(napi_env env, napi_callback_info info) {
  webrtc::ohos::OhosCamera::GetInstance().StopCamera();
  return nullptr;
}
static napi_value PeerClientInit(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    napi_valuetype string_type;
    napi_typeof(env, args[0], &string_type);
    napi_valuetype number_type;
    napi_typeof(env, args[1], &number_type);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerClientInit");
    char server[256] = {0};
    int port = 0;
    size_t buf_size = 0;

    if (napi_get_value_string_utf8(env, args[0], server, sizeof(server), &buf_size) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "napi_get_value_string_utf8 server failed! \n");
        return nullptr;
    }

    if (napi_get_value_int32(env, args[1], &port) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "napi_get_value_int32 port failed! \n");
        return nullptr;
    }

    if (PeerSampleClientStart(server, port) < 0) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerSampleClientStart failed! \n");
        return nullptr;
    }

    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}

static napi_value PeerClientConnectServer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerClientConnectServer");
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "mediasoup version %{public}s", mediasoupclient::Version().c_str());
    
    if (!GetServerConnect()) {
        PeerSamplePostEvent(PEER_EVENT_CONNECT_SERVER);
    }
    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}

static napi_value PeerClientDisConnectServer(napi_env env, napi_callback_info info)
{
    if (GetPeerConnect()) {
        PeerSamplePostEvent(PEER_EVENT_DISCONNECT_PEER);
        webrtc::ohos::OhosCamera::GetInstance().StopCamera();
        sleep(1);
    }
    if (GetServerConnect()) {
        PeerSamplePostEvent(PEER_EVENT_DISCONNECT_SERVER);
    }
    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}

static napi_value PeerClientConnectPeer(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerClientConnectPeer");
    webrtc::ohos::OhosCamera::GetInstance().Init(env, info);
    if (!GetPeerConnect()) {
        PeerSamplePostEvent(PEER_EVENT_CONNECT_PEER);
    }
    
    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}

static napi_value PeerClientDisConnectPeer(napi_env env, napi_callback_info info)
{
    if (GetPeerConnect()) {
        PeerSamplePostEvent(PEER_EVENT_DISCONNECT_PEER);
        webrtc::ohos::OhosCamera::GetInstance().StopCamera();
        sleep(1);
    }
    napi_value result;
    napi_create_int32(env, 0, &result);
    return result;
}
static napi_value PeerClientGetPeerName(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerClientGetPeerName");
    if (PeerSampleGetPeers() > 0) {
        std::string name = PeerSampleGetPeerName(0);
        napi_value result;
        napi_create_string_utf8(env, name.c_str(), name.length(), &result);
        return result;
    }
    
    return nullptr;
}

static napi_value OpenCamera(napi_env env, napi_callback_info info) {

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "openCamera");
    webrtc::ohos::OhosCamera::GetInstance().Init(env, info);

    napi_value result;
    napi_create_int32(env, 0, &result);
    uint32_t camera_index = webrtc::ohos::OhosCamera::GetInstance().GetCameraIndex();
    camera_index = 1;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "camera_index = %{public}d", camera_index);
    webrtc::ohos::OhosCamera::GetInstance().InitCamera();
    webrtc::ohos::OhosCamera::GetInstance().SetCameraIndex(camera_index);
    webrtc::ohos::OhosCamera::GetInstance().InitCamera();
    webrtc::ohos::OhosCamera::GetInstance().StartCamera();
    
    return nullptr;
}

static napi_value PeerClientGetPeers(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerClientGetPeers111");
    int peers = PeerSampleGetPeers();
    napi_value result;
    napi_create_int64(env, peers, &result);
    return result;
    
}

static napi_value PeerClientServerIsConnect(napi_env env, napi_callback_info info) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerClientServerIsConnect222");
    bool connected = GetServerConnect();
    napi_value result;
    napi_create_int32(env, connected ? 1 : 0, &result);
    return result;
}

static napi_value PeerClientPeerIsConnect(napi_env env, napi_callback_info info) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "PeerClientPeerIsConnect333");
    bool connected = GetPeerConnect();
    napi_value result;
    napi_create_int32(env, connected ? 1 : 0, &result);
    return result;
}

static napi_value ChangeCamera(napi_env env, napi_callback_info info) {
  webrtc::ohos::OhosCamera::GetInstance().StopCamera();
  webrtc::ohos::OhosCamera::GetInstance().CameraRelease();
  uint32_t camera_index = webrtc::ohos::OhosCamera::GetInstance().GetCameraIndex();
  OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "camera_index = %{public}d", camera_index);
  camera_index = camera_index <= 1 ? 1 - camera_index : 0;
  webrtc::ohos::OhosCamera::GetInstance().SetCameraIndex(camera_index);
  webrtc::ohos::OhosCamera::GetInstance().InitCamera();
  webrtc::ohos::OhosCamera::GetInstance().StartCamera();

  return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
  napi_property_descriptor desc[] = {
    {"initCamera", nullptr, InitCamera, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"createFromReceiver", nullptr, CreateFromReceiver, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"stopCamera", nullptr, StopCamera, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"changeCamera", nullptr, ChangeCamera, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"pcClientInit", nullptr, PeerClientInit, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"pcClientConnectServer", nullptr, PeerClientConnectServer, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"pcClientConnectPeer", nullptr, PeerClientConnectPeer, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"openCamera", nullptr, OpenCamera, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"pcClientGetPeerName", nullptr, PeerClientGetPeerName, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"pcClientGetPeers", nullptr, PeerClientGetPeers, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"pcClientDisConnectServer", nullptr, PeerClientDisConnectServer, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"pcClientDisConnectPeer", nullptr, PeerClientDisConnectPeer, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"pcClientServerIsConnect", nullptr, PeerClientServerIsConnect, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"pcClientPeerIsConnect", nullptr, PeerClientPeerIsConnect, nullptr, nullptr, nullptr, napi_default, nullptr},
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
