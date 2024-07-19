//
// Created on 2024/3/6.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".


#ifndef OH_WEB_RTC_OHOS_CAMERA_H
#define OH_WEB_RTC_OHOS_CAMERA_H

#include "hilog/log.h"
#include "napi/native_api.h"
#include <arm-linux-ohos/bits/alltypes.h>
#include <multimedia/image_framework/image_receiver_mdk.h>
#include <ohcamera/camera.h>
#include <ohcamera/camera_manager.h>

#include "rtc_base/ref_counter.h"
#include "api/video/video_source_interface.h"
#include "api/video/video_frame.h"
#include "modules/video_capture/video_capture.h"


namespace webrtc{
namespace ohos{
class OhosCamera {
public:
  static OhosCamera& GetInstance();
  bool Init(napi_env env, napi_callback_info info);
  int32_t InitCamera();
  int32_t StartCamera();
  int32_t StopCamera();
  int32_t CameraRelease();
  
  uint32_t GetCameraIndex();
  int32_t SetCameraIndex(uint32_t camera_index);
  napi_value GetImageData(napi_env env, ImageReceiverNative *image_receiver_c);
  void RegisterCaptureDataCallback(rtc::VideoSinkInterface<webrtc::VideoFrame> *dataCallback);
  void UnregisterCaptureDataCallback();
  virtual ~OhosCamera();

private:
  OhosCamera() {};
  bool ImageReceiverOn(uint8_t *buffer, int32_t w, int32_t h);
  int32_t CameraInputCreateAndOpen();
  int32_t CameraInputRelease();
  
  int32_t PreviewOutputCreate();
  int32_t PreviewOutputRelease();
  
  int32_t CaptureSessionSetting();
  int32_t CaptureSessionUnsetting();

  int32_t DeleteCameraOutputCapability();
  int32_t DeleteCameras();
  int32_t DeleteCameraManage();
  
  rtc::VideoSinkInterface<webrtc::VideoFrame> *data_callback_ {nullptr};
  webrtc::VideoCaptureCapability configured_capability_;
  bool is_init_ {false};
  bool is_camera_started_ {false};
  char* img_receive_surfaceId_ {nullptr};
  char* x_component_surfaceId_ {nullptr};
  Camera_Manager* camera_manager_ {nullptr};
  Camera_Device* cameras_ {nullptr};
  uint32_t cameras_size_ {0};
  Camera_CaptureSession* capture_session_ {nullptr};
  Camera_OutputCapability* camera_output_capability_ {nullptr};
  const Camera_Profile* preview_profile_ {nullptr};
  Camera_PreviewOutput* preview_output_ {nullptr};
  Camera_PreviewOutput* x_component_preview_ {nullptr};
  Camera_Input* camera_input_ {nullptr};
  uint32_t camera_dev_index_ {0};
  uint32_t profile_index_ {0};
};
} // namespace ohos
} // namespace webrtc
#endif //OH_WEB_RTC_OHOS_CAMERA_H
