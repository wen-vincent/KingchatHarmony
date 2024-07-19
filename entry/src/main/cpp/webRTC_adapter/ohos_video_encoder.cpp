#include "ohos_camera.h"
#include "hilog/log.h"
#include "ohos_video_encoder.h"
#include <bits/alltypes.h>
#include <mutex>
#include <thread>
#include <functional>
#include <unistd.h>
#include "ohos_common.h"

namespace webrtc {
namespace ohos {

OhosVideoEncoder::~OhosVideoEncoder() {
    Release();
}

int32_t OhosVideoEncoder::Create() {
    OH_AVCapability *capability = OH_AVCodec_GetCapability(OH_AVCODEC_MIMETYPE_VIDEO_AVC, true);
    bool isHardware = OH_AVCapability_IsHardware(capability);
    if (!isHardware) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, "OhosVideoEncoder", "Hardware Not Support");
    }
    const char *codec_name = OH_AVCapability_GetName(capability);
    encoder_ = OH_VideoEncoder_CreateByName(codec_name);
    if(encoder_ == nullptr) {
        return AV_ERR_UNKNOWN;
    } else {
        return AV_ERR_OK;
    }
}

int32_t OhosVideoEncoder::Config(CodecData *codecData) {
    int32_t ret = AV_ERR_UNKNOWN;
    if (encoder_ == nullptr || codecData == nullptr) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "Config nullptr");
      return AV_ERR_UNKNOWN;
    }
    ret = Configure(codecData->formatInfo);
    if (ret != AV_ERR_OK) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "Configure failed");
        return ret;
    }
    ret = SetCallback(codecData);
    if (ret != AV_ERR_OK) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "SetCallback failed");
      return ret;
    }
    ret = OH_VideoEncoder_Prepare(encoder_);
    if (ret != AV_ERR_OK) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OH_VideoEncoder_Prepare Failed");
      return ret;
    }
    return ret;
}

int32_t OhosVideoEncoder::Start() {
    if (encoder_ == nullptr) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OhosVideoEncoder nullptr");
      return AV_ERR_INVALID_VAL;
    }
    // 启动编码器，开始编码
    int32_t ret = OH_VideoEncoder_Start(encoder_);
    if (ret != AV_ERR_OK) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OH_VideoEncoder_Start Failed");
      return ret;
    }
    return ret;
}

int32_t OhosVideoEncoder::PushInputData(CodecBufferInfo &info) {
    if (encoder_ == nullptr) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OhosVideoEncoder nullptr");
      return AV_ERR_INVALID_VAL;
    }
    int32_t ret = AV_ERR_INVALID_VAL;
    ret = OH_AVBuffer_SetBufferAttr(reinterpret_cast<OH_AVBuffer *>(info.buff_), &info.attr_);
    if (ret != AV_ERR_OK) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "Set avbuffer attr failed");
      return ret;
    }
    ret = OH_VideoEncoder_PushInputBuffer(encoder_, info.bufferIndex);
    if (ret != AV_ERR_OK) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "Push input data failed");
      return ret;
    }
    return ret;
}

int32_t OhosVideoEncoder::FreeOutPutData(uint32_t bufferIndex) {
    if (encoder_ == nullptr) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OhosVideoEncoder nullptr");
      return AV_ERR_INVALID_VAL;
    }
    int32_t ret = AV_ERR_OK;
    ret = OH_VideoEncoder_FreeOutputBuffer(encoder_, bufferIndex);
    if (ret != AV_ERR_OK) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "Free output data failed, ret: %{public}d", ret);
    }
    return ret;
}

int32_t OhosVideoEncoder::Stop() {
    if (encoder_ == nullptr) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OhosVideoEncoder nullptr");
      return AV_ERR_INVALID_VAL;
    }
    int32_t ret = AV_ERR_INVALID_VAL;
    ret = OH_VideoEncoder_Stop(encoder_);
    if (ret != AV_ERR_OK) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OH_VideoEncoder_Stop Failed");
      return ret;
    }
    // 刷新编码器
    ret = OH_VideoEncoder_Flush(encoder_);
    if (ret != AV_ERR_OK) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OH_VideoEncoder_Flush Failed");
      return ret;
    }
    return AV_ERR_OK;
}

void OhosVideoEncoder::Release() {
    int32_t ret = AV_ERR_OK;
    if (encoder_ != nullptr) {
      ret = OH_VideoEncoder_Destroy(encoder_);
      encoder_ = nullptr;
    }
    if (ret != AV_ERR_OK) {
      OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OH_VideoEncoder_Destroy Failed");
    }
}

int32_t OhosVideoEncoder::Configure(FormatInfo *formatInfo) {
    // 配置编码器
    if ( encoder_ == nullptr || formatInfo == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "encoder_ or formatInfo nullptr");
        return AV_ERR_INVALID_VAL;
    }
    if (formatInfo->videoHeight == 0 || formatInfo->videoWidth == 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "Configure Parameter Invalid");
        return AV_ERR_INVALID_VAL;
    }
    OH_AVFormat *format_ = OH_AVFormat_Create();
    if (format_ == nullptr) {
        return AV_ERR_UNKNOWN;
    }
    // 写入format
    OH_AVFormat_SetIntValue(format_, OH_MD_KEY_WIDTH, formatInfo->videoWidth);
    OH_AVFormat_SetIntValue(format_, OH_MD_KEY_HEIGHT, formatInfo->videoHeight);
    OH_AVFormat_SetDoubleValue(format_, OH_MD_KEY_FRAME_RATE, formatInfo->frameRate);
    OH_AVFormat_SetIntValue(format_, OH_MD_KEY_PIXEL_FORMAT, formatInfo->pixelFormat);
    OH_AVFormat_SetIntValue(format_, OH_MD_KEY_RANGE_FLAG, formatInfo->rangeFlag);
    OH_AVFormat_SetIntValue(format_, OH_MD_KEY_VIDEO_ENCODE_BITRATE_MODE, formatInfo->rateMode);
    OH_AVFormat_SetLongValue(format_, OH_MD_KEY_BITRATE, formatInfo->bitrate);
    int32_t ret = OH_VideoEncoder_Configure(encoder_, format_);
    if (ret != AV_ERR_OK) {
        // 异常处理
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "OH_VideoEncoder_Configure Failed");
        return ret;
    } else {
        OH_AVFormat_Destroy(format_);
        format_ = nullptr;
    }
    return ret;
}

int32_t OhosVideoEncoder::SetCallback(CodecData *codecData) {
    // 配置异步回调，调用 OH_VideoEncoder_SetCallback 接口
    OH_AVCodecAsyncCallback cb = {&CodecCallback::OnCodecError, &CodecCallback::OnCodecFormatChange, 
                                  &CodecCallback::OnNeedInputBuffer, &CodecCallback::OnNewOutputBuffer};
    int32_t ret = OH_VideoEncoder_SetCallback(encoder_, cb, codecData);
    if (ret != AV_ERR_OK) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, "OhosVideoEncoder", "SetCallback Failed");
        return ret;
    }
    return ret;
}

}
}