#ifndef OH_WEB_RTC_OHOS_CODEC_DATA_H
#define OH_WEB_RTC_OHOS_CODEC_DATA_H

#include <mutex>
#include <queue>

#include "ohos_video_common.h"

namespace webrtc {
namespace ohos {
class CodecData {
public:
  FormatInfo *formatInfo {nullptr};
  uint32_t inputFrameCount_ {0};
  std::mutex inputMutex_;
  std::condition_variable inputCond_;
  std::queue<CodecBufferInfo> inputBufferInfoQueue_;
  uint32_t outputFrameCount_ {0};
  std::mutex outputMutex_;
  std::condition_variable outputCond_;
  std::queue<CodecBufferInfo> outputBufferInfoQueue_;
};

class CodecCallback {
public:
  static void OnCodecError(OH_AVCodec *codec, int32_t errorCode, void *userData);
  static void OnCodecFormatChange(OH_AVCodec *codec, OH_AVFormat *format, void *userData);
  static void OnNeedInputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVMemory  *buffer, void *userData);
  static void OnNewOutputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVMemory  *buffer, OH_AVCodecBufferAttr *attr, void *userData);
};
}
}

#endif //OH_WEB_RTC_OHOS_CODEC_DATA_H
