// Created on 2024/3/5.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "ohos_camera.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_rotation.h"
#include "libyuv/convert.h"
#include "rtc_base/time_utils.h"
#include "hilog/log.h"
#include <arm-linux-ohos/bits/alltypes.h>
#include <locale.h>
#include <multimedia/image_framework/image_mdk.h>
#include <multimedia/image_framework/image_pixel_map_mdk.h>
#include <multimedia/image_framework/image_pixel_map_napi.h>
#include <multimedia/image_framework/image_receiver_mdk.h>
#include <malloc.h>


namespace webrtc {
namespace ohos {

OhosCamera& OhosCamera::GetInstance(){
  static OhosCamera ohos_camera;
  return ohos_camera;
}

bool OhosCamera::Init(napi_env env, napi_callback_info info){
  if(is_init_) return true;
  size_t argc = 2;
  // 声明参数数组
  napi_value args[2] = {nullptr};
  
  // 获取传入的参数并依次放入参数数组中
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  size_t result;
  napi_get_value_string_utf8(env, args[0], nullptr, 0, &result);
  if (result == 0) {
    return false;
  }
  img_receive_surfaceId_ = new char[result + 1];
  napi_get_value_string_utf8(env, args[0], img_receive_surfaceId_, result + 1, &result);
  OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "img_receive_surfaceId_ = %{public}s", img_receive_surfaceId_);

  napi_get_value_string_utf8(env, args[1], nullptr, 0, &result);
  if (result > 0) {
    x_component_surfaceId_ = new char[result + 1];
    napi_get_value_string_utf8(env, args[1], x_component_surfaceId_, result + 1, &result);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "x_component_surfaceId_ = %{public}s",
                 x_component_surfaceId_);
  }

  is_init_ = true;
  return true;
}

int32_t OhosCamera::InitCamera() {
  int32_t return_code = -1;

  Camera_ErrorCode ret = OH_Camera_GetCameraManager(&camera_manager_);
  if (camera_manager_ == nullptr || ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_Camera_GetCameraManager failed, ret = %{public}d", ret);
    return return_code;
  }

  ret = OH_CameraManager_GetSupportedCameras(camera_manager_, &cameras_, &cameras_size_);
  if(cameras_ == nullptr || ret != CAMERA_OK || cameras_size_ <= 0) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CameraManager_GetSupportedCameras failed, ret = %{public}d", ret);
    return return_code;
  }

  ret = OH_CameraManager_GetSupportedCameraOutputCapability(camera_manager_, &cameras_[camera_dev_index_], &camera_output_capability_);
  if(camera_output_capability_ == nullptr || ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CameraManager_GetSupportedCameraOutputCapability failed, ret = %{public}d", ret);
    return return_code;
  }
  
  if(camera_output_capability_->metadataProfilesSize <= 0) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "metadataProfilesSize <= 0");
    return return_code;
  }
 
  return_code = 0;
  return return_code;
}

int32_t OhosCamera::CameraInputCreateAndOpen() {
  Camera_ErrorCode ret;
  ret = OH_CameraManager_CreateCameraInput(camera_manager_, &cameras_[camera_dev_index_], &camera_input_);
  if (camera_input_ == nullptr || ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest",
                 "OH_CameraManager_CreateCaptureSession failed, ret = %{public}d", ret);
    return -1;
  }

  ret = OH_CameraInput_Open(camera_input_);
  if (ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CameraInput_Open failed, ret = %{public}d", ret);
    return -1;
  }

  return 0;
}

int32_t OhosCamera::CameraInputRelease() {
  Camera_ErrorCode ret;
  if (camera_input_ != nullptr) {
    ret = OH_CameraInput_Close(camera_input_);
    if (ret != CAMERA_OK) {
      OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CameraInput_Close failed, ret = %{public}d", ret);
    }
    ret = OH_CameraInput_Release(camera_input_);
    if (ret != CAMERA_OK) {
      OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CameraInput_Release failed, ret = %{public}d", ret);
    }
  }

  camera_input_ = nullptr;
  return 0;
}

int32_t OhosCamera::PreviewOutputCreate() {
  Camera_ErrorCode ret;
  preview_profile_ = camera_output_capability_->previewProfiles[profile_index_];
  ret =
    OH_CameraManager_CreatePreviewOutput(camera_manager_, preview_profile_, img_receive_surfaceId_, &preview_output_);
  if (preview_output_ == nullptr || ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest",
                 "OH_CameraManager_CreatePreviewOutput failed, ret = %{public}d", ret);
    return -1;
  }
  
  if (x_component_surfaceId_ != nullptr) {
    ret = OH_CameraManager_CreatePreviewOutput(camera_manager_, preview_profile_, x_component_surfaceId_,
                                               &x_component_preview_);
    if (preview_output_ == nullptr || ret != CAMERA_OK) {
      OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest",
                   "OH_CameraManager_CreatePreviewOutput failed, ret = %{public}d", ret);
      return -1;
    }
  }
  
  return 0;
}

int32_t OhosCamera::PreviewOutputRelease() {
  Camera_ErrorCode ret;
  if (preview_output_ != nullptr) {
    ret = OH_PreviewOutput_Release(preview_output_);
    if (ret != CAMERA_OK) {
      OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_PreviewOutput_Release failed, ret = %{public}d", ret);
    }
  }

  if (x_component_preview_ != nullptr) {
    ret = OH_PreviewOutput_Release(x_component_preview_);
    if (ret != CAMERA_OK) {
      OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_PreviewOutput_Release failed, ret = %{public}d", ret);
    }
  }

  preview_output_ = nullptr;
  x_component_preview_ = nullptr;
  return 0;
}

int32_t OhosCamera::CaptureSessionSetting() {
  Camera_ErrorCode ret;
  ret = OH_CameraManager_CreateCaptureSession(camera_manager_, &capture_session_);
  if (capture_session_ == nullptr || ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest",
                 "OH_CameraManager_CreateCaptureSession failed, ret = %{public}d", ret);
    return -1;
  }
  
  ret = OH_CaptureSession_BeginConfig(capture_session_);
  if (ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CaptureSession_BeginConfig failed, ret = %{public}d",
                 ret);
    return -1;
  }

  if (camera_input_) {
    ret = OH_CaptureSession_AddInput(capture_session_, camera_input_);
    if (ret != CAMERA_OK) {
      OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CaptureSession_AddInput failed, ret = %{public}d", ret);
      return -1;
    }
  }

  if (preview_output_) {
    ret = OH_CaptureSession_AddPreviewOutput(capture_session_, preview_output_);
    if (ret != CAMERA_OK) {
      OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest",
                   "OH_CaptureSession_AddPreviewOutput failed, ret = %{public}d", ret);
      return -1;
    }
  }
  
  if (x_component_preview_) {
    ret = OH_CaptureSession_AddPreviewOutput(capture_session_, x_component_preview_);
    if (ret != CAMERA_OK) {
      OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest",
                   "OH_CaptureSession_AddPreviewOutput failed, ret = %{public}d", ret);
      return -1;
    }
  }

  ret = OH_CaptureSession_CommitConfig(capture_session_);
  if (ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CaptureSession_CommitConfig failed, ret = %{public}d",
                 ret);
    return -1;
  }
  return 0;
}

int32_t OhosCamera::CaptureSessionUnsetting() {
  Camera_ErrorCode ret;
  if(capture_session_ == nullptr) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "CaptureSessionUnsetting failed, capture_session_ is nullptr");
    return -1;
  }
  
  ret = OH_CaptureSession_Release(capture_session_);
  capture_session_ = nullptr;
  if (ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CaptureSession_Release failed, ret = %{public}d", ret);
    return -1;
  }

  return 0;
}

int32_t OhosCamera::StartCamera() {
  if (is_camera_started_) {
    // 先关闭相机、会话、预览流，然后再重新打开
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StartCamera: Camera has been started");
    StopCamera();
    CaptureSessionUnsetting();
    CameraInputRelease();
    PreviewOutputRelease();
  }
  Camera_ErrorCode ret = CAMERA_OK;

  if (camera_input_ == nullptr) {
    CameraInputCreateAndOpen();
  }
  if (preview_output_ == nullptr) {
    PreviewOutputCreate();
  }
  if (capture_session_ == nullptr) {
    CaptureSessionSetting();
  }

  ret = OH_CaptureSession_Start(capture_session_);
  if (ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CaptureSession_Start failed, ret = %{public}d", ret);
    return -1;
  }

  is_camera_started_ = true;
  return 0;
}

int32_t OhosCamera::StopCamera() {
  Camera_ErrorCode ret = CAMERA_OK;

  if (is_camera_started_ == false) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StopCamera: Camera not started");
    return 0;
  }

  if (capture_session_ == nullptr) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "StopCamera: capture_session_ is nullptr");
    return -1;
  }

  ret = OH_CaptureSession_Stop(capture_session_);
  if (ret != CAMERA_OK) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CaptureSession_Stop failed, ret = %{public}d", ret);
    return -1;
  }

  is_camera_started_ = false;
  return 0;
}

int32_t OhosCamera::DeleteCameraOutputCapability() {
  if (camera_output_capability_ != nullptr) {
    // napi暂不支持该操作
    // Camera_ErrorCode ret = OH_CameraManager_DeleteSupportedCameraOutputCapability(camera_manager_,
    // camera_output_capability_); if (ret != CAMERA_OK) {
    //   OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_CameraManager_DeleteSupportedCameraOutputCapability\
    //   failed, ret = %{public}d", ret);
    // }
    preview_profile_ = nullptr;
    camera_output_capability_ = nullptr;
  }

  return 0;
}

int32_t OhosCamera::DeleteCameras() {
  if (cameras_ != nullptr) {
    // napi暂不支持该操作
    // Camera_ErrorCode ret = OH_CameraManager_DeleteSupportedCameras(camera_manager_, cameras_, cameras_size_);
    // if (ret != CAMERA_OK) {
    //   OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest",
    //                "OH_CameraManager_DeleteSupportedCameras failed, ret = %{public}d", ret);
    // }
    cameras_ = nullptr;
  }

  return 0;
}

int32_t OhosCamera::DeleteCameraManage() {
  if (camera_manager_ != nullptr) {
    // napi暂不支持该操作
    // Camera_ErrorCode ret = OH_Camera_DeleteCameraManager(camera_manager_);
    // if(ret != CAMERA_OK) {
    //   OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_Camera_DeleteCameraManager failed, ret = %{public}d",
    //   ret);
    // }
    camera_manager_ = nullptr;
  }

  return 0;
}

int32_t OhosCamera::CameraRelease() {
  if (is_camera_started_) {
    StopCamera();
  }
  CaptureSessionUnsetting();
  CameraInputRelease();
  PreviewOutputRelease();

  DeleteCameraOutputCapability();
  DeleteCameras();
  DeleteCameraManage();
  return 0;
}

uint32_t OhosCamera::GetCameraIndex() { return camera_dev_index_; }

int32_t OhosCamera::SetCameraIndex(uint32_t camera_index) {
  if (camera_index >= 0 && camera_index <= cameras_size_) {
    camera_dev_index_ = camera_index;
  }
  OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "camera_dev_index_ = %{public}d", camera_dev_index_);
  return 0;
}

bool OhosCamera::ImageReceiverOn(uint8_t *buffer, int32_t width, int32_t height) {
//   OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "imageReceiverOn started");
  height = (height > 0) ? height : -height; // abs

  rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer = webrtc::I420Buffer::Create(width, height);
  libyuv::NV21ToI420(buffer, width, buffer + width * height, width, i420_buffer.get()->MutableDataY(),
                     i420_buffer.get()->StrideY(), i420_buffer.get()->MutableDataU(), i420_buffer.get()->StrideU(),
                     i420_buffer.get()->MutableDataV(), i420_buffer.get()->StrideV(), width, height);

  webrtc::VideoFrame video_frame = webrtc::VideoFrame::Builder()
                                     .set_video_frame_buffer(i420_buffer)
                                     .set_timestamp_rtp(0)
                                     .set_timestamp_ms(rtc::TimeMillis())
                                     .set_rotation(webrtc::kVideoRotation_90)
                                     .build();
  if (data_callback_) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "data_callback_");
    data_callback_->OnFrame(video_frame);
  }
  return true;
}

napi_value OhosCamera::GetImageData(napi_env env, ImageReceiverNative* image_receiver_c) {
  int32_t ret;
  napi_value next_image;
  // 或调用 OH_Image_Receiver_ReadNextImage(imgReceiver_c, &nextImage);
  ret = OH_Image_Receiver_ReadLatestImage(image_receiver_c, &next_image);
  if (ret != IMAGE_RESULT_SUCCESS) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_Image_Receiver_ReadLatestImage failed, ret = %{public}d", ret);
    return nullptr;
  }

  ImageNative *next_image_native = OH_Image_InitImageNative(env, next_image);

  OhosImageSize img_size;
  ret = OH_Image_Size(next_image_native, &img_size);
//   OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_Image_Size  width: %{public}d, height:%{public}d",
//                img_size.width, img_size.height);
  if (ret != IMAGE_RESULT_SUCCESS) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_Image_Size failed, ret = %{public}d", ret);
    return nullptr;
  }
  
  OhosImageComponent imgComponent;
  ret = OH_Image_GetComponent(next_image_native, 4, &imgComponent); // 4=jpeg
  if (ret != IMAGE_RESULT_SUCCESS) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_Image_GetComponent failed, ret = %{public}d", ret);
    return nullptr;
  }

  uint8_t *img_buffer = imgComponent.byteBuffer;
  if (ImageReceiverOn(img_buffer, img_size.width, img_size.height) == false) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "ImageReceiverOn failed");
    return nullptr;
  }

  ret = OH_Image_Release(next_image_native);
  if (ret != IMAGE_RESULT_SUCCESS) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OH_Image_Release failed, ret = %{public}d", ret);
    return nullptr;
  }
  return next_image;
}

void OhosCamera::RegisterCaptureDataCallback(rtc::VideoSinkInterface<webrtc::VideoFrame> *data_callback) {
  data_callback_ = data_callback;
}

void OhosCamera::UnregisterCaptureDataCallback() {
  data_callback_ = nullptr;
}

OhosCamera::~OhosCamera() { 
  OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "mytest", "OhosCamera::~OhosCamera start");

  CameraRelease();
  if (img_receive_surfaceId_ != nullptr) {
    delete[] img_receive_surfaceId_;
    img_receive_surfaceId_ = nullptr;
  }
  
  if (x_component_preview_ != nullptr) {
    delete[] x_component_surfaceId_;
    x_component_surfaceId_ = nullptr;
  }

  camera_dev_index_ = 0;
  profile_index_ = 0;
  is_init_ = false;
}
} // namespace ohos
} // namespace webrtc
