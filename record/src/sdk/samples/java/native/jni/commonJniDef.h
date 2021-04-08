//define signatures
#define LONG_SIGNATURE "J"
#define INT_SIGNATURE "I"
#define BOOL_SIGNATURE "Z"
#define INT_ARRAY_SIGNATURE "[I"
#define DOUBLE_SIGNATURE "D"
#define LONG_PARA_VOID_RETURN "(J)V" 
#define VOID_PARA_VOID_RETURN "()V"
#define EMPTY_PARA_INT_RETURN "()I"
#define SG_INT "I"
#define STRING_SIGNATURE "Ljava/lang/String;"
#define BYTEARRAY "[B"
#define CHANNEL_PROFILE_SIGNATURE "Lio/agora/recording/common/Common$CHANNEL_PROFILE_TYPE;"
#define REMOTE_VIDEO_STREAM_SIGNATURE "Lio/agora/recording/common/Common$REMOTE_VIDEO_STREAM_TYPE;"
#define MIXED_AV_CODEC_TYPE_SIGNATURE "Lio/agora/recording/common/Common$MIXED_AV_CODEC_TYPE;"
//-----------------Video Begin-------------------
//#define VIDEOFRAME_SIGNATURE "io/agora/recording/common/Common$VideoFrame"
#define VIDEOFRAME_H264_SIGNATURE "Lio/agora/recording/common/Common$VideoH264Frame;"
#define VIDEOFRAME_H265_SIGNATURE "Lio/agora/recording/common/Common$VideoH265Frame;"
#define VIDEOFRAME_YUV_SIGNATURE "Lio/agora/recording/common/Common$VideoYuvFrame;"
#define VIDEOFRAME_JPG_SIGNATURE "Lio/agora/recording/common/Common$VideoJpgFrame;" 
#define VIDEO_FRAME_TYPE_SIGNATURE "Lio/agora/recording/common/Common$VIDEO_FRAME_TYPE;"
#define AUDIO_FORMAT_TYPE_SIGNATURE "Lio/agora/recording/common/Common$AUDIO_FORMAT_TYPE;"
#define VIDEO_FORMAT_TYPE_SIGNATURE "Lio/agora/recording/common/Common$VIDEO_FORMAT_TYPE;"
#define VIDEOMIXLAYOUT_SIGNATURE "[Lio/agora/recording/common/Common$VideoMixingLayout$Region;"
#define VIDEOMIXLAYOUT_LITERALWM_SIGNATURE "[Lio/agora/recording/common/Common$LiteraWatermarkConfig;"
#define VIDEOMIXLAYOUT_TIMESTAMPWM_SIGNATURE "[Lio/agora/recording/common/Common$TimestampWatermarkConfig;"
#define VIDEOMIXLAYOUT_IMAGEWM_SIGNATURE "[Lio/agora/recording/common/Common$ImageWatermarkConfig;"
//jmethodID signature
#define SN_MTD_COMMON_INIT "(Lio/agora/recording/common/Common;)V"
//video  init jmethodID 
#define SN_MTD_VIDEO_YUV_FRAME_INIT "(Lio/agora/recording/common/Common;JIIIII)V"
#define SN_MTD_VIDEO_H264_FRAME_INIT "(Lio/agora/recording/common/Common;)V"
#define SN_MTD_VIDEO_H265_FRAME_INIT "(Lio/agora/recording/common/Common;)V"
#define SN_MTD_VIDEO_JPG_FRAME_INIT "(Lio/agora/recording/common/Common;)V"
//class name
//VIDEO
#define CN_REP "io/agora/recording/common/RecordingEngineProperties"
#define CN_VIDEO_FRAME "io/agora/recording/common/Common$VideoFrame"
#define CN_VIDEO_YUV_FRAME "io/agora/recording/common/Common$VideoYuvFrame"
#define CN_VIDEO_JPG_FRAME "io/agora/recording/common/Common$VideoJpgFrame"
#define CN_VIDEO_H264_FRAME "io/agora/recording/common/Common$VideoH264Frame"
#define CN_VIDEO_H265_FRAME "io/agora/recording/common/Common$VideoH265Frame"

#define CN_AUDIO_STATS "io/agora/recording/common/Common$RemoteAudioStats"
#define CN_VIDEO_STATS "io/agora/recording/common/Common$RemoteVideoStats"
#define CN_RECORDING_STATS "io/agora/recording/common/Common$RecordingStats"

//video frame type clas
#define CN_VIDEO_FRAME_TYPE "io/agora/recording/common/Common$VIDEO_FRAME_TYPE"

//init jmethodID
#define SG_MTD_INIT "<init>"
#define SG_COMMON_INIT "<init>"
#define MTD_TYPE "type"

#define VIDEO_FRAME_SIGNATURE "(Lio/agora/recording/common/Common;)V"
#define MTD_INIT_VIDEO_FRAME "(Lio/agora/recording/common/Common;)V"

//#define SN_CB_FUNC_RECEIVE_VIDEOFRAME "(JLio/agora/recording/common/Common$VideoFrame;)V"
#define SN_CB_FUNC_RECEIVE_VIDEOFRAME "(JILio/agora/recording/common/Common$VideoFrame;I)V"
//declare java callback function
#define CB_FUNC_RECEIVE_VIDEOFRAME "videoFrameReceived"

#define CB_FUNC_RECEIVE_AUDIOFRAME "audioFrameReceived"

//declare filedIDs
#define FID_VIDEO_YUV_FRAME_BUF "buf"
#define FID_VIDEO_YUV_FRAME_BUFSIZE "bufSize"
#define FID_VIDEO_FRAME_YUV "yuv"
#define FID_VIDEO_FRAME_JPG "jpg"
#define FID_VIDEO_FRAME_H264 "h264"
#define FID_VIDEO_FRAME_H265 "h265"
#define FID_FRAME_MS "frame_ms"

//static java class init jmethodIDs
//static jmethodID mJavaVideoFrameInitMtd;
//static jmethodID mJavaVideoYuvFrameInitMtd;

//-----------------Video End-------------------

//-----------------Audio Begin-------------------
//class name
//Audio
#define CN_AUDIO_VOLUME_INFO "io/agora/recording/common/Common$AudioVolumeInfo"
#define CN_AUDIO_FRAME "io/agora/recording/common/Common$AudioFrame"
#define CN_AUDIO_FRAME_TYPE "io/agora/recording/common/Common$AUDIO_FRAME_TYPE"
#define CN_AUDIO_AAC_FRAME "io/agora/recording/common/Common$AudioAacFrame"
#define CN_AUDIO_PCM_FRAME "io/agora/recording/common/Common$AudioPcmFrame"
//#define SN_CB_FUNC_RECEIVE_AUDIOFRAME "(JLio/agora/recording/common/Common$AudioFrame;)V"
#define SN_CB_FUNC_RECEIVE_AUDIOFRAME "(JLio/agora/recording/common/Common$AudioFrame;)V"

#define SN_AUDIO_FRAME_TYPE "Lio/agora/recording/common/Common$AUDIO_FRAME_TYPE;"
#define SN_INIT_MTD_AUDIO_FRAME "(Lio/agora/recording/common/Common;JJJ)V"
#define SN_INIT_MTD_AUDIO_AAC_FRAME "(Lio/agora/recording/common/Common;J)V"

#define SN_AUDIO_PCM_FRAME "Lio/agora/recording/common/Common$AudioPcmFrame;"
#define SN_AUDIO_AAC_FRAME "Lio/agora/recording/common/Common$AudioAacFrame;"

//-----------------Audio End-------------------

//common
#define FID_USERID "uid"
//audio pcm 
#define FID_CHANNELS "channels"
#define FID_SAMPLE_BITS "sample_bits"
#define FID_SAMPLE_RATES "sample_rates"
#define FID_SAMPLE "samples"
#define FID_PCMBUF "pcmBuf"
#define FID_AACBUF "aacBuf"
#define FID_PCMBUFFERSIZE "pcmBufSize"
#define FID_TYPE "type"
#define FID_PCM "pcm"
#define FID_AAC "aac"
#define FID_AACBUF "aacBuf"
#define FID_AACBUFFERSIZE "aacBufSize"
#define FID_CHANNELNUM "frame_num"
#define FID_CHANNELS "channels"
#define FID_BITRATE "bitrate"

//audio volume
#define FID_AUDIO_VOLUME "volume"

#define FID_QUALITYID "quality"
#define FID_NETWORK_DELAYID "networkTransportDelay"
#define FID_JITTER_DELAYID "jitterBufferDelay"
#define FID_LOSS_RATEID "audioLossRate"

#define FID_DELAYID "delay"
#define FID_WIDTHID "width"
#define FID_HEIGHTID "height"
#define FID_RECEIVEDBITRATEID "receivedBitrate"
#define FID_DECODEROUTPUTFRAMERATEID "decoderOutputFrameRate"
#define FID_RXSTRAMTYPEID "rxStreamType"

#define FID_DURATIONID "duration"
#define FID_RXBYTESID "rxBytes"
#define FID_RXKBITRATEID "rxKBitRate"
#define FID_RXAUDIOKBITRATEID "rxAudioKBitRate"
#define FID_RXVIDEOKBITRATEID "rxVideoKBitRate"
#define FID_LASTMILEDELAYID "lastmileDelay"
#define FID_USERCOUNTID "userCount"
#define FID_CPUAPPUSAGEID "cpuAppUsage"
#define FID_CPUTOTALUSAGEID "cpuTotalUsage"

//-----------------Callback Java method----------------

enum GETID_TYPE{
  MTDID,
  FIDID
};
struct JavaObjectMethod {
  int id;
  const char* name;
  const char* signature;
};
enum CBObjectMethodId {
  MID_ON_ERROR = 0,
  MID_ON_WARNING = 1,
  MID_ON_USEROFFLINE = 2,
  MID_ON_USERJOINED = 3,
  MID_ON_AUDIOFRAME_RECEIVED = 4,
  MID_ON_VIDEOFRAME_RECEIVED = 5,
  MID_ON_LEAVECHANNEL = 6,
  MID_ON_ACTIVE_SPEAKER = 7,
  MID_ON_JOINCHANNEL_SUCCESS = 8,
  MID_ON_AUDIO_VOLUME_INDICATION = 9,
  MID_ON_FIRST_REMOTE_VIDEO_DECODED = 10,
  MID_ON_FIRST_REMOTE_AUDIO_FRAME = 11,
  MID_ON_RECEIVING_STREAM_STATUS_CHANGED = 12,
  MID_ON_CONNECTION_LOST = 13,
  MID_ON_CONNECTION_INTERRUPTED = 14,
  MID_ON_USER_VIDEO_STATE_CHANGED = 15,
  MID_ON_USER_AUDIO_STATE_CHANGED = 16,
  MID_ON_REJOINCHANNEL_SUCCESS = 17,
  MID_ON_CONNECTION_CHANGED = 18,
  MID_ON_REMOTE_VIDEO_STATS = 19,
  MID_ON_REMOTE_AUDIO_STATS = 20,
  MID_ON_RECORDING_STATS = 21,
  MID_ON_LOCAL_REGISTERED = 22,
  MID_ON_USER_INFO_UPDATE = 23,
  MID_CBOBJECT_NUM = 24
};

enum ClassMethodId {
  MID_GET_RANDOM_UUID = 0,
  MID_GET_LOCAL_HOST = 1,
  MID_GET_LOCAL_HOST_LIST = 2,
  MID_CLASS_NUM = 3,
};
static JavaObjectMethod jCBObjectMethods[] = {
  { MID_ON_ERROR, "onError", "(II)V" },
  { MID_ON_WARNING, "onWarning", "(I)V" },
  { MID_ON_USEROFFLINE, "onUserOffline", "(JI)V" },
  { MID_ON_USERJOINED, "onUserJoined", "(JLjava/lang/String;)V"},
  { MID_ON_AUDIOFRAME_RECEIVED, "audioFrameReceived", "(JLio/agora/recording/common/Common$AudioFrame;)V"},
  { MID_ON_VIDEOFRAME_RECEIVED, "videoFrameReceived","(JILio/agora/recording/common/Common$VideoFrame;I)V"},
  { MID_ON_LEAVECHANNEL, "onLeaveChannel", "(I)V"},
  { MID_ON_ACTIVE_SPEAKER, "onActiveSpeaker", "(J)V"},
  { MID_ON_REJOINCHANNEL_SUCCESS, "onRejoinChannelSuccess", "(Ljava/lang/String;J)V"},
  { MID_ON_AUDIO_VOLUME_INDICATION, "onAudioVolumeIndication", "([Lio/agora/recording/common/Common$AudioVolumeInfo;)V"},
  { MID_ON_FIRST_REMOTE_VIDEO_DECODED, "onFirstRemoteVideoDecoded", "(JIII)V"},
  { MID_ON_FIRST_REMOTE_AUDIO_FRAME, "onFirstRemoteAudioFrame", "(JI)V"},
  { MID_ON_RECEIVING_STREAM_STATUS_CHANGED, "onReceivingStreamStatusChanged", "(ZZ)V"},
  { MID_ON_CONNECTION_LOST, "onConnectionLost", "()V"},
  { MID_ON_CONNECTION_INTERRUPTED, "onConnectionInterrupted", "()V"},
  { MID_ON_USER_VIDEO_STATE_CHANGED, "onRemoteVideoStreamStateChanged", "(JII)V" },
  { MID_ON_USER_AUDIO_STATE_CHANGED, "onRemoteAudioStreamStateChanged", "(JII)V" },
  { MID_ON_JOINCHANNEL_SUCCESS, "onJoinChannelSuccess", "(Ljava/lang/String;J)V"},
  { MID_ON_CONNECTION_CHANGED, "onConnectionStateChanged", "(II)V"},
  { MID_ON_REMOTE_AUDIO_STATS, "onRemoteAudioStats", "(JLio/agora/recording/common/Common$RemoteAudioStats;)V"},
  { MID_ON_REMOTE_VIDEO_STATS, "onRemoteVideoStats", "(JLio/agora/recording/common/Common$RemoteVideoStats;)V"},
  { MID_ON_RECORDING_STATS, "onRecordingStats", "(Lio/agora/recording/common/Common$RecordingStats;)V"},
  { MID_ON_LOCAL_REGISTERED, "onLocalUserRegistered", "(JLjava/lang/String;)V"},
  { MID_ON_USER_INFO_UPDATE, "onUserInfoUpdated", "(JLjava/lang/String;)V"}
};
//video
enum VIDEOFRAME{
  MID_INIT = 0,
  MID_YUV = 1,
  MID_H264 = 2,
  MID_JPG = 3,
  MID_H265 = 4
};
static JavaObjectMethod jVideoFrameMethods[] = {
  {MID_INIT, "<init>", "(Lio/agora/recording/common/Common;)V"},
  {MID_YUV, "yuv", "Lio/agora/recording/common/Common$VideoYuvFrame;"},
  {MID_H264,"h264","Lio/agora/recording/common/Common$VideoH264Frame;"},
  {MID_JPG, "jpg", "Lio/agora/recording/common/Common$VideoJpgFrame;"},
  {MID_H265, "h265", "Lio/agora/recording/common/Common$VideoH265Frame;"}
};
enum FID_VIDEOYUVFRAME{
  FID_FRAMEMS = 0,
  FID_YUVBUF = 1,
  FID_YUVBUFSIZE =2,
  FID_YUVWIDTH = 3,
  FID_YUVHEIGHT = 4,
  FID_YUVYSTRIDE = 5,
  FID_YUVUSTRIDE = 6,
  FID_YUVVSTRIDE = 7,
  FID_YUVNUM = 8,
};

static JavaObjectMethod jVideoYuvFrameFields[] = {
  {FID_FRAMEMS, FID_FRAME_MS, LONG_SIGNATURE},
  {FID_YUVBUF, FID_VIDEO_YUV_FRAME_BUF, BYTEARRAY}, 
  {FID_YUVBUFSIZE,FID_VIDEO_YUV_FRAME_BUFSIZE, LONG_SIGNATURE},
  {FID_YUVWIDTH,"width", "I"},
  {FID_YUVHEIGHT, "height", "I"},
  {FID_YUVYSTRIDE, "ystride", "I"},
  {FID_YUVUSTRIDE, "ustride", "I"},
  {FID_YUVVSTRIDE, "vstride", "I"},
};
enum FID_VIDEOH264FRAME{
  FID_H264_FRAMEMS = 0 ,
  FID_H264_FRAMENUM = 1,
  FID_H264_BUF = 2,
  FID_H264_BUF_SIZE = 3,
  FID_H264NUM = 4
};
static JavaObjectMethod jVideoH264FrameFields[] = {
  {FID_H264_FRAMEMS, FID_FRAME_MS, LONG_SIGNATURE},
  {FID_H264_FRAMENUM, FID_CHANNELNUM, LONG_SIGNATURE},
  {FID_H264_BUF, FID_VIDEO_YUV_FRAME_BUF, BYTEARRAY},
  {FID_H264_BUF_SIZE, FID_VIDEO_YUV_FRAME_BUFSIZE, LONG_SIGNATURE}
};
enum FID_VIDEOH265FRAME{
  FID_H265_FRAMEMS = 0 ,
  FID_H265_FRAMENUM = 1,
  FID_H265_BUF = 2,
  FID_H265_BUF_SIZE = 3,
  FID_H265NUM = 4
};
static JavaObjectMethod jVideoH265FrameFields[] = {
  {FID_H265_FRAMEMS, FID_FRAME_MS, LONG_SIGNATURE},
  {FID_H265_FRAMENUM, FID_CHANNELNUM, LONG_SIGNATURE},
  {FID_H265_BUF, FID_VIDEO_YUV_FRAME_BUF, BYTEARRAY},
  {FID_H265_BUF_SIZE, FID_VIDEO_YUV_FRAME_BUFSIZE, LONG_SIGNATURE}
};
enum FID_JPGFRAME{
  FID_JPG_FRAMEMS = 0,
  FID_JPG_BUF = 1,
  FID_JPG_BUFSIZE = 2,
  FID_JPGNUM = 3
};
static JavaObjectMethod jVideoJpgFrameFields[] = {
  {FID_JPG_FRAMEMS, FID_FRAME_MS, LONG_SIGNATURE},
  {FID_JPG_BUF, FID_VIDEO_YUV_FRAME_BUF, BYTEARRAY},
  {FID_JPG_BUFSIZE, FID_VIDEO_YUV_FRAME_BUFSIZE, LONG_SIGNATURE},
};
//audio
enum FID_AUDIOFRAME{
  FID_AF_TYPE = 0,
  FID_AF_PCM = 1,
  FID_AF_AAC = 2,
  FID_AF_NUM= 3 
};
static JavaObjectMethod jAudioFrameFields[] = {
  {FID_AF_TYPE, FID_TYPE, SN_AUDIO_FRAME_TYPE},
  {FID_AF_PCM, FID_PCM, SN_AUDIO_PCM_FRAME},
  {FID_AF_AAC, FID_AAC, SN_AUDIO_AAC_FRAME}
};
enum FID_PCMFRAME{
  FID_PCM_FRAMEMS = 0,
  FID_PCM_CHANNELS = 1,
  FID_PCM_SAMPLEBITS = 2,
  FID_PCM_SAMPLERATES = 3,
  FID_PCM_SAMPLES = 4,
  FID_PCM_BUF = 5,
  FID_PCM_BUFSIZE = 6,
  FID_PCMNUM = 7
};
static JavaObjectMethod jAudioPcmFrameFields[] = {
  {FID_PCM_FRAMEMS, FID_FRAME_MS,LONG_SIGNATURE},
  {FID_PCM_CHANNELS, FID_CHANNELS, LONG_SIGNATURE},
  {FID_PCM_SAMPLEBITS, FID_SAMPLE_BITS, LONG_SIGNATURE},
  {FID_PCM_SAMPLERATES, FID_SAMPLE_RATES, LONG_SIGNATURE},
  {FID_PCM_SAMPLES, FID_SAMPLE, LONG_SIGNATURE},
  {FID_PCM_BUF, FID_PCMBUF, BYTEARRAY},
  {FID_PCM_BUFSIZE, FID_PCMBUFFERSIZE, LONG_SIGNATURE},
};
enum FID_AACFRAME{
  FID_AAC_FRAMEMS = 0,
  FID_AAC_BUF = 1,
  FID_AAC_BUFSIZE = 2,
  FID_AAC_CHANNELS = 3,
  FID_AAC_BITRATE = 4,
  FID_AACNUM = 5 
};
static JavaObjectMethod jAudioAacFrameFields[] = {
  {FID_AAC_FRAMEMS, FID_FRAME_MS,LONG_SIGNATURE},
  {FID_AAC_BUF, FID_AACBUF, BYTEARRAY},
  {FID_AAC_BUFSIZE, FID_AACBUFFERSIZE, LONG_SIGNATURE},
  {FID_AAC_CHANNELS, FID_CHANNELS, INT_SIGNATURE},
  {FID_AAC_BITRATE, FID_BITRATE, INT_SIGNATURE},
};

enum FID_AUDIO_VOLUME_INFO{
  FID_UID = 0,
  FID_VOLUME = 1,
  FID_AUDIO_VOLUME_INFO_NUM = 2
};

static JavaObjectMethod jAudioVolumeInfoFields[] = {
  {FID_AUDIO_VOLUME_INFO::FID_UID, FID_USERID, LONG_SIGNATURE},
  {FID_AUDIO_VOLUME_INFO::FID_VOLUME, FID_AUDIO_VOLUME, SG_INT},
};

enum FID_AUDIO_STATS{
  FID_QUALITY = 0,
  FID_NETWORK_DELAY = 1,
  FID_JITTER_DELAY = 2,
  FID_LOSS_RATE = 3,
  FID_AUDIO_STATS_NUM = 4
};

static JavaObjectMethod jAudioStatsFields[] = {
  {FID_AUDIO_STATS::FID_QUALITY, FID_QUALITYID, INT_SIGNATURE},
  {FID_AUDIO_STATS::FID_NETWORK_DELAY, FID_NETWORK_DELAYID, INT_SIGNATURE},
  {FID_AUDIO_STATS::FID_JITTER_DELAY, FID_JITTER_DELAYID, INT_SIGNATURE},
  {FID_AUDIO_STATS::FID_LOSS_RATE, FID_LOSS_RATEID, INT_SIGNATURE}
};

enum FID_VIDEO_STATS {
  FID_DELAY = 0,
  FID_WIDTH = 1,
  FID_HEIGHT = 2,
  FID_RECEIVEDBITRATE = 3,
  FID_DECODEROUTPUTFRAMERATE = 4,
  FID_RXSTRAMTYPE = 5,
  FID_VIDEO_STATS_NUM = 6
};

static JavaObjectMethod jVideoStatsFields[] = {
  {FID_VIDEO_STATS::FID_DELAY, FID_DELAYID, INT_SIGNATURE},
  {FID_VIDEO_STATS::FID_WIDTH, FID_WIDTHID, INT_SIGNATURE},
  {FID_VIDEO_STATS::FID_HEIGHT, FID_HEIGHTID, INT_SIGNATURE},
  {FID_VIDEO_STATS::FID_RECEIVEDBITRATE, FID_RECEIVEDBITRATEID, INT_SIGNATURE},
  {FID_VIDEO_STATS::FID_DECODEROUTPUTFRAMERATE,FID_DECODEROUTPUTFRAMERATEID, INT_SIGNATURE},
  {FID_VIDEO_STATS::FID_RXSTRAMTYPE, FID_RXSTRAMTYPEID, INT_SIGNATURE}
};

enum FID_RECORDING_STATS {
  FID_DURATION = 0,
  FID_RXBYTES = 1,
  FID_RXKBITRATE = 2,
  FID_RXAUDIOKBITRATE = 3,
  FID_RXVIDEOKBITRATE = 4,
  FID_LASTMILEDELAY = 5,
  FID_USERCOUNT = 6,
  FID_CPUAPPUSAGE = 7,
  FID_CPUTOTALUSAGE = 8,
  FID_RECORDING_STATS_NUM = 9
};

static JavaObjectMethod jRecordingStatsFields[] = {
  {FID_RECORDING_STATS::FID_DURATION, FID_DURATIONID, INT_SIGNATURE},
  {FID_RECORDING_STATS::FID_RXBYTES, FID_RXBYTESID, INT_SIGNATURE},
  {FID_RECORDING_STATS::FID_RXKBITRATE, FID_RXKBITRATEID, INT_SIGNATURE},
  {FID_RECORDING_STATS::FID_RXAUDIOKBITRATE, FID_RXAUDIOKBITRATEID, INT_SIGNATURE},
  {FID_RECORDING_STATS::FID_RXVIDEOKBITRATE, FID_RXVIDEOKBITRATEID, INT_SIGNATURE},
  {FID_RECORDING_STATS::FID_LASTMILEDELAY, FID_LASTMILEDELAYID, INT_SIGNATURE},
  {FID_RECORDING_STATS::FID_USERCOUNT, FID_USERCOUNTID, INT_SIGNATURE},
  {FID_RECORDING_STATS::FID_CPUAPPUSAGE, FID_CPUAPPUSAGEID, INT_SIGNATURE},
  {FID_RECORDING_STATS::FID_CPUTOTALUSAGE, FID_CPUTOTALUSAGEID, INT_SIGNATURE}
};
