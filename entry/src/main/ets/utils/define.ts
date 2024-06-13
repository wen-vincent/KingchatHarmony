// use dscp
// 参考 http://www.rtcbits.com/2017/01/using-dscp-for-webrtc-packet-marking.html
const PC_PROPRIETARY_CONSTRAINTS = Object.freeze({
  optional: [{ googDscp: true }]
});

// 创建视频时候的传输方向, SEND: 发送, RECV: 接收
const TRANSPORT_DIRECTION = Object.freeze({
  SEND: 1,
  RECV: 2
});

// 视频方式 SINGLE: 单向视频,只发不收, TWOWAY: 双向视频,发送且接收
const VIDEO_MODE = Object.freeze({
  SINGLE: 1,
  TWOWAY: 2
});

// 媒体发送类型,
const MEDIA_SEND_STATE = Object.freeze({
  SENDONLY: 1, // 只发送,不转发
  FORWARD: 2   // 转发
});

// 媒体类型,
const MEDIA_IDENTITY = Object.freeze({
  CAMERA: 1, // 正常的视频状态,比方双向视频时候自己的视频流
  DESKTOP_DISPLAY: 2, // 桌面
  MIXED_FLOW: 3,  // 两人或者多人视频时候,录制混合视频
  RECORD_ONLY: 4, // 仅用来录制
  SINGLE_RECORD: 5,  // 单向视频录制
  TWOWAY_RECORD: 6   // 双向视频录制
  //TODO: 现在默认是音视频一组,以后区分视频和音频活着任意组合
});

export { PC_PROPRIETARY_CONSTRAINTS, TRANSPORT_DIRECTION, VIDEO_MODE, MEDIA_SEND_STATE, MEDIA_IDENTITY }