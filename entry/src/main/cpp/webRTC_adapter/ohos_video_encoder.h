#ifndef OH_WEB_RTC_OHOS_VIDEO_ENCODER_H
#define OH_WEB_RTC_OHOS_VIDEO_ENCODER_H

#include "api/video_codecs/video_encoder.h"
#include "ohos_video_common.h"
#include "ohos_codec_data.h"

#include <multimedia/player_framework/native_avcodec_videoencoder.h>
#include <multimedia/player_framework/native_avcapability.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <multimedia/player_framework/native_avformat.h>

namespace webrtc {
namespace ohos {

class OhosVideoEncoder {
public:
    OhosVideoEncoder() = default;
    ~OhosVideoEncoder();
    int32_t Create();
    int32_t Config(CodecData *codecData);
    int32_t Start();
    int32_t PushInputData(CodecBufferInfo &info);
    int32_t FreeOutPutData(uint32_t bufferIndex);
    int32_t Stop();
    void Release();
private : 
    int32_t SetCallback(CodecData *codecData);
    int32_t Configure(FormatInfo *formatInfo);
    OH_AVCodec *encoder_ {nullptr};
};

}
}

#endif //OH_WEB_RTC_OHOS_VIDEO_ENCODER_H