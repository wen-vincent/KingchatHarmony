import image from '@ohos.multimedia.image'

export const createFromReceiver: (a: image.ImageReceiver) => image.Image;
export const initCamera: () => void;
export const stopCamera: () => void;
export const changeCamera: () => void;

export const pcClientInit: (server: String, port: Number) => Number;
export const pcClientConnectServer: () => Number;
export const pcClientConnectPeer: (a: String, b: String) => Number;
export const openCamera: (a: String, b: String) => Number;
export const pcClientGetPeerName: (peerid: Number) => string;
export const pcClientGetPeers: () => Number;
export const pcClientDisConnectServer: () => Number;
export const pcClientDisConnectPeer: () =>Number;
export const pcClientServerIsConnect: () => Boolean;
export const pcClientPeerIsConnect: () => Boolean;

export const getMediasoupDevice: (a:String) => string;
export const initMediasoup: () => Number;
export const connectMediastream: (String) => Number;
