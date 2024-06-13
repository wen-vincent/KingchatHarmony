import image from '@ohos.multimedia.image'

export const createFromReceiver: (a: image.ImageReceiver) => image.Image;
export const initCamera: (a: string, b: String) => void;
export const stopCamera: () => void;
export const changeCamera: () => void;

export const pcClientInit: (server: string, port: number) => number;
export const pcClientConnectServer: () => number;
export const pcClientConnectPeer: (a: string, b: String) => number;
export const pcClientGetPeerName: (peerid: number) => string;
export const pcClientGetPeers: () => number;
export const pcClientDisConnectServer: () => number;
export const pcClientDisConnectPeer: () =>number;
export const pcClientServerIsConnect: () => boolean;
export const pcClientPeerIsConnect: () => boolean;
