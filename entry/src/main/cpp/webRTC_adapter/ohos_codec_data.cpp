#include "hilog/log.h"
#include "ohos_codec_data.h"

namespace webrtc {
namespace ohos {

void CodecCallback::OnCodecError(OH_AVCodec *videoEnc, int32_t errorCode, void *userData) {
    (void)videoEnc;
    (void)errorCode;
    (void)userData;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "CodecCallback", "AVCodec errorCode %{public}d", errorCode);
}

void CodecCallback::OnCodecFormatChange(OH_AVCodec *videoEnc, OH_AVFormat *format, void *userData) {
    (void)videoEnc;
    (void)format;
    (void)userData;
    if (userData == nullptr) {
        return;
    }
    (void)userData;
    CodecData *codecUserData = static_cast<CodecData *>(userData);
    std::unique_lock<std::mutex> lock(codecUserData->outputMutex_);
    //TODO
}

void CodecCallback::OnNeedInputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVMemory  *buffer, void *userData) {
    if (userData == nullptr) {
        return;
    }
    (void)codec;
    CodecData *codecUserData = static_cast<CodecData *>(userData);
    std::unique_lock<std::mutex> lock(codecUserData->inputMutex_);
    codecUserData->inputBufferInfoQueue_.emplace(index, buffer);
    codecUserData->inputCond_.notify_all();
}

void CodecCallback::OnNewOutputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVMemory  *buffer, OH_AVCodecBufferAttr *attr, void *userData) {
    if (userData == nullptr) {
        return;
    }
    (void)codec;
    CodecData *codecUserData = static_cast<CodecData *>(userData);
    std::unique_lock<std::mutex> lock(codecUserData->outputMutex_);
    codecUserData->outputBufferInfoQueue_.emplace(index, buffer);
    codecUserData->outputCond_.notify_all();
}

}
}