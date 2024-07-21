#ifndef OH_WEB_RTC_OHOS_VIDEO_COMMON_H
#define OH_WEB_RTC_OHOS_VIDEO_COMMON_H
#include <multimedia/player_framework/native_avcodec_videoencoder.h>
#include <multimedia/player_framework/native_avbuffer_info.h>
#include <multimedia/player_framework/native_avformat.h>

#include <string>

namespace webrtc {
namespace ohos {

struct FormatInfo {
  std::string codecMime;
  int32_t videoWidth {0};
  int32_t videoHeight {0};
  double frameRate {30.0};
  int64_t bitrate {3000000};
  OH_AVPixelFormat pixelFormat {AV_PIXEL_FORMAT_NV21};
  bool rangeFlag {false};
  int32_t profile {OH_AVCProfile::AVC_PROFILE_BASELINE};
  int32_t rateMode {OH_VideoEncodeBitrateMode::CBR};
};

struct CodecBufferInfo {
  int32_t bufferIndex = -1;
  uint8_t *buff_ = nullptr;
  OH_AVCodecBufferAttr attr_ {0, 0, 0, 0};
  CodecBufferInfo(uint32_t argBufferIndex, OH_AVMemory *argBuffer, OH_AVCodecBufferAttr argAttr)
      : bufferIndex(argBufferIndex), buff_(reinterpret_cast<uint8_t *>(argBuffer)), attr_(argAttr){};
  CodecBufferInfo(uint32_t argBufferIndex, OH_AVMemory *argBuffer)
      : bufferIndex(argBufferIndex), buff_(reinterpret_cast<uint8_t *>(argBuffer)){};
  CodecBufferInfo(uint32_t argBufferIndex, OH_AVBuffer *argBuffer)
      : bufferIndex(argBufferIndex), buff_(reinterpret_cast<uint8_t *>(argBuffer)) 
  {
      OH_AVBuffer_GetBufferAttr(argBuffer, &attr_);
  };
};
}
}

#endif //OH_WEB_RTC_OHOS_VIDEO_COMMON_H
