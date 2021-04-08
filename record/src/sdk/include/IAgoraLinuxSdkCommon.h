#ifndef _IAGORA_LINUXSDKCOMMON_H_
#define _IAGORA_LINUXSDKCOMMON_H_

#include <cstdint>
#include <string>
#include <string.h>

namespace agora {
namespace linuxsdk {

class IEngine{
    virtual ~IEngine() {};

};

class IEngineConfig {
    virtual ~IEngineConfig() {};
};

typedef unsigned char uchar_t;
typedef unsigned int uint_t;
typedef unsigned int uid_t;
typedef uint64_t u64_t;

/** Error codes.
 * 
 * @note When using the Agora Recording SDK, you may also receive error codes 
 * from the Agora Native SDK. See more [error codes](https://docs.agora.io/en/Interactive%20Broadcast/API%20Reference/cpp/namespaceagora.html#a8affb9bb02864d82c4333529dc3d75a1).
 */
enum ERROR_CODE_TYPE {
    
    /** 0: No error. */
    ERR_OK = 0,
    //1~1000
    /** 1: General error with no classified reason. */
    ERR_FAILED = 1,
    /** 2: Invalid parameter. For example, the specific channel name contains illegal characters. */
    ERR_INVALID_ARGUMENT = 2,
    /** 3: The SDK module is not ready. Agora recommends the following methods to solve this error:
     - Check the audio device.
     - Check the completeness of the app.
     - Re-initialize the SDK.
     */
    ERR_INTERNAL_FAILED = 3,
};

/** State codes. */
enum STAT_CODE_TYPE {
    /** 0: Everything is normal. */
    STAT_OK = 0,
    /** 1: Errors from the Agora Native SDK. See more [Error Codes](https://docs.agora.io/en/Interactive%20Broadcast/API%20Reference/cpp/namespaceagora.html#a8affb9bb02864d82c4333529dc3d75a1). */
    STAT_ERR_FROM_ENGINE = 1,
    /** 2: Failure to join the channel. */
    STAT_ERR_ARS_JOIN_CHANNEL = 2,
    /** 3: Failure to create a process. */
    STAT_ERR_CREATE_PROCESS = 3,
    /** 4: Invalid parameters of the video profile of the mixed video. See [Video Profile Table](https://docs.agora.io/en/faq/recording_video_profile?_ga=2.4402419.1965239098.1566358977-1833718170.1540974595) to set the `mixResolution` parameter. */
    STAT_ERR_MIXED_INVALID_VIDEO_PARAM = 4,
    /** 5: Null pointer. */
    STAT_ERR_NULL_POINTER = 5,
    /** 6: Invalid parameters of the proxy server. */
    STAT_ERR_PROXY_SERVER_INVALID_PARAM = 6,

    /** 0x8: Error in polling. */
    STAT_POLL_ERR = 0x8,
    /** 0x10: Polling hangs up. */
    STAT_POLL_HANG_UP = 0x10,
    /** 0x20: Invalid polling request. */
    STAT_POLL_NVAL = 0x20,
};

/** The reasons why the recording server leaves the channel. */
enum LEAVE_PATH_CODE {
    /** 0: The initialization fails. */
    LEAVE_CODE_INIT = 0,
    /** 1: The AgoraCoreService process receives the SIGINT signal. */
    LEAVE_CODE_SIG = 1<<1,
    /** 2: The recording server automatically leaves the channel and 
     * stops recording as there is no user in the channel. 
     */
    LEAVE_CODE_NO_USERS = 1<<2,
    /** 3: The AgoraCoreService process receives the SIGTERM signal. */
    LEAVE_CODE_TIMER_CATCH = 1<<3,
    /** 4: The recording server calls the 
     * \ref agora::recording::IRecordingEngine::leaveChannel "leaveChannel" method 
     * to leave the channel. 
     */
    LEAVE_CODE_CLIENT_LEAVE = 1 << 4,
};

/** Warning codes.
 @note When using the Agora Recording SDK, you may also receive warning codes from the Native SDK. See [Interactive Broadcast Warning Codes](https://docs.agora.io/en/Interactive%20Broadcast/API%20Reference/cpp/namespaceagora.html#a32d042123993336be6646469da251b21).
 */
enum WARN_CODE_TYPE {
    /** 103: No channel resources are available. Maybe because the server cannot allocate any channel resource. */
    WARN_NO_AVAILABLE_CHANNEL = 103,
    /** 104: A timeout when looking up the channel. When a user joins a channel, the SDK looks up the specified channel. This warning usually occurs when the network conditions are too poor to connect to the server.
     */
    WARN_LOOKUP_CHANNEL_TIMEOUT = 104,
    /** 105: The server rejected the request to look up the channel. The server cannot process this request or the request is illegal. */
    WARN_LOOKUP_CHANNEL_REJECTED = 105,
    /** 106: A timeout occurred when opening the channel. Once the specific channel is found, the SDK opens the channel. This warning usually occurs when the network conditions are too poor to connect to the server. */
    WARN_OPEN_CHANNEL_TIMEOUT = 106,
    /** 107: The server rejected the request to open the channel. The server cannot process this request or the request is illegal. */
    WARN_OPEN_CHANNEL_REJECTED = 107,
    /** 108: An abnormal error occurs. The SDK would resume the recording. */
    WARN_RECOVERY_CORE_SERVICE_FAILURE = 108,
};

/** Channel types.
 @note The Recording SDK must use the same channel profile as the Agora Native/Web SDK, otherwise issues may occur.
 */
enum CHANNEL_PROFILE_TYPE
{
    /** 0: (Default) Communication mode. This is used in one-on-one or group calls, where all users in the channel can talk freely. */
    CHANNEL_PROFILE_COMMUNICATION = 0,
    /** 1: Live broadcast mode. The host sends and receives voice/video, while the audience only receives voice/video. Host and audience roles can be set by calling setClientRole. */
    CHANNEL_PROFILE_LIVE_BROADCASTING = 1,
};

/** The reasons why the user leaves the channel or goes offline. */
enum USER_OFFLINE_REASON_TYPE
{
    /** 0: The user has quit the call. */
    USER_OFFLINE_QUIT = 0,
    /** 1: The SDK timed out and the user dropped offline because it has not received any data packet for a period of time. If a user quits the call and the message is not passed to the SDK (due to an unreliable channel), the SDK assumes the user has dropped offline.
     */
    USER_OFFLINE_DROPPED = 1,
    /** 2: The client role has changed from the host to the audience. The option is only valid when you set the channel profile as live broadcast when calling \ref agora::recording::IRecordingEngine::joinChannel "joinChannel".
     */
    USER_OFFLINE_BECOME_AUDIENCE = 2,
};

/** Takes effect only when the Agora Native SDK has enabled dual-stream mode (high stream by default). */
enum REMOTE_VIDEO_STREAM_TYPE
{
    /** 0: (Default) High stream. */
    REMOTE_VIDEO_STREAM_HIGH = 0,
    /** 1: Low stream. */
    REMOTE_VIDEO_STREAM_LOW = 1,
};

/** Video decoding format. */
enum VIDEO_FORMAT_TYPE {
    /** 0: Default video format. */
    VIDEO_FORMAT_DEFAULT_TYPE = 0,
    /** **DEPRECATED** 1: Video frame in H.264 format. */
    VIDEO_FORMAT_H264_FRAME_TYPE = 1,
    /** 1: Video frame in H.264 or H.265 format. */
    VIDEO_FORMAT_ENCODED_FRAME_TYPE = VIDEO_FORMAT_H264_FRAME_TYPE,
    /** 2: Video frame in YUV format. */
    VIDEO_FORMAT_YUV_FRAME_TYPE = 2,
    /** 3: Video frame in JPEG format. */
    VIDEO_FORMAT_JPG_FRAME_TYPE = 3,
    /** 4: JPEG file format. */
    VIDEO_FORMAT_JPG_FILE_TYPE = 4,
    /** 5: JPEG file format + MP4 video.
     - Individual Mode (\ref agora::recording::RecordingConfig#isMixingEnabled "isMixingEnabled" is set as false): MP4 video and JPEG files.
     - Composite Mode (\ref agora::recording::RecordingConfig#isMixingEnabled "isMixingEnabled" is set as true): MP4 video file for mixed streams and JPEG files for individual streams.
     */
    VIDEO_FORMAT_JPG_VIDEO_FILE_TYPE = 5,
};

/** Audio decoding format. */
enum AUDIO_FORMAT_TYPE {
    /** 0: Default audio format. */
    AUDIO_FORMAT_DEFAULT_TYPE = 0,
    /** 1: Audio frame in AAC format. */
    AUDIO_FORMAT_AAC_FRAME_TYPE = 1,
    /** 2: Audio frame in PCM format. */
    AUDIO_FORMAT_PCM_FRAME_TYPE = 2,
    /** 3: Audio-mixing frame in PCM format. */
    AUDIO_FORMAT_MIXED_PCM_FRAME_TYPE = 3,
};

/** Audio frame type. */
enum AUDIO_FRAME_TYPE {
    /** 0: PCM format. */
    AUDIO_FRAME_RAW_PCM = 0,
    /** 1: AAC format. */
    AUDIO_FRAME_AAC = 1
};

/** Memory type. */
enum MEMORY_TYPE {
    /** 0: Stack. */
    STACK_MEM_TYPE = 0,
    /** 1: Heap. */
    HEAP_MEM_TYPE = 1
};

/** Video frame type. */
enum VIDEO_FRAME_TYPE {
    /** 0: YUV format. */
    VIDEO_FRAME_RAW_YUV = 0,
    /** 1: H.264 format. */
    VIDEO_FRAME_H264 = 1,
    /** 2: JPG format. */
    VIDEO_FRAME_JPG = 2,
    /** 3: H.265 format. */
    VIDEO_FRAME_H265 = 3,
};

enum SERVICE_MODE {
    RECORDING_MODE = 0,//down stream
    SERVER_MODE = 1,//up-down stream
    IOT_MODE = 2,//up-down stream
};

/** Whether to record automatically or manually. */
enum TRIGGER_MODE_TYPE {
    /** 0: (Default) Automatically. */
    AUTOMATICALLY_MODE = 0,
    /** 1: Manually. To start and stop recording, call \ref agora::recording::IRecordingEngine::startService "startService" 
     * and \ref agora::recording::IRecordingEngine::stopService "stopService" respectively.
     */
    MANUALLY_MODE = 1
};
    
/** The programming language. */
enum LANGUAGE_TYPE {
    /** 0: (Default) C++. */
    CPP_LANG = 0,
    /** 1: Java. */
    JAVA_LANG = 1
};
    
/** Audio profile. Sets the sampling rate, bitrate, encode mode, and the number of channels. */
enum AUDIO_PROFILE_TYPE {
    /** 0: (Default) Sampling rate of 48 kHz, communication encoding, mono, and a bitrate of up to 48 Kbps. */
    AUDIO_PROFILE_DEFAULT = 0, //use default settings.
    /** 1: Sampling rate of 48 kHz, music encoding, mono, and a bitrate of up to 128 Kbps. */
    AUDIO_PROFILE_HIGH_QUALITY = 1, //48khz, 128kbps, mono, music
    /** 2: Sampling rate of 48 kHz, music encoding, stereo, and a bitrate of up to 192 Kbps. */
    AUDIO_PROFILE_HIGH_QUALITY_STEREO = 2, //48khz, 192kbps, stereo, music
};

/** Log level. */
enum agora_log_level {
    /** 1: Fatal. */
    AGORA_LOG_LEVEL_FATAL = 1,
    /** 2: Error. */
    AGORA_LOG_LEVEL_ERROR = 2,
    /** 3: Warning. */
    AGORA_LOG_LEVEL_WARN = 3,
    /** 4: Notice. */
    AGORA_LOG_LEVEL_NOTICE = 4,
    /** 5: Info. */
    AGORA_LOG_LEVEL_INFO = 5,
    /** 6: Debug. */
    AGORA_LOG_LEVEL_DEBUG = 6,
};

/** The reasons causing the change of the remote stream state. */
enum RemoteStreamStateChangedReason {
  /** Starts pulling the stream of the remote user. */
  REASON_REMOTE_STREAM_STARTED,
  /** Stops pulling the stream of the remote user. */
  REASON_REMOTE_STREAM_STOPPED
};

/** The states of the remote stream. */
enum RemoteStreamState {
  /** The remote stream is received normally. */
  REMOTE_STREAM_STATE_RUNNING,
  /** The remote stream is stopped. */
  REMOTE_STREAM_STATE_STOPPED
};

/** Mix audio and video in real time. Takes effect only when the \ref agora::recording::RecordingConfig#isMixingEnabled "isMixingEnabled" is set as true.
 
 See [Supported Players](https://docs.agora.io/en/faq/recording_player).
 
 */
enum MIXED_AV_CODEC_TYPE {
    /** 0: (Default) Mixes the audio and video respectively.*/
    MIXED_AV_DEFAULT = 0,  
    /** 1: Mixes the audio and video in real time into an MP4 file. Supports limited players.*/
    MIXED_AV_CODEC_V1 = 1,
    /** 2: Mixes the audio and video in real time into an MP4 file. Supports more players.*/
    MIXED_AV_CODEC_V2 = 2
};

/** User roles in a live broadcast. */
enum ClientRoleType {
  /** The  host in a live broadcast. */
  CLIENT_ROLE_BROADCASTER = 1,
  /** The audience in a live broadcast. */
  CLIENT_ROLE_AUDIENCE =2,
};

/** Connection states. */
enum ConnectionStateType 
{
  /** 1: The SDK is disconnected from Agora's edge server.

   - This is the initial state before calling the \ref agora::recording::IRecordingEngine::joinChannel "joinChannel" method.
   - The SDK also enters this state when the application calls the \ref agora::recording::IRecordingEngine::leaveChannel "leaveChannel" method.
   */
  CONNECTION_STATE_DISCONNECTED = 1,
  /** 2: The SDK is connecting to Agora's edge server.

   - When the application calls the \ref agora::recording::IRecordingEngine::joinChannel "joinChannel" method, the SDK starts to establish a connection to the specified channel, triggers the \ref agora::recording::IRecordingEngineEventHandler::onConnectionStateChanged "onConnectionStateChanged" callback, and switches to the #CONNECTION_STATE_CONNECTING state.
   - When the SDK successfully joins the channel, it triggers the \ref agora::recording::IRecordingEngineEventHandler::onConnectionStateChanged "onConnectionStateChanged" callback and switches to the #CONNECTION_STATE_CONNECTED state.
   - After the SDK joins the channel and when it finishes initializing the recording engine, the SDK triggers the \ref agora::recording::IRecordingEngineEventHandler::onJoinChannelSuccess "onJoinChannelSuccess" callback.
   */
  CONNECTION_STATE_CONNECTING = 2,
  /** 3: The SDK is connected to Agora's edge server and has joined a channel. You can now publish or subscribe to a media stream in the channel.

   If the connection to the channel is lost because, for example, if the network is down or switched, the SDK automatically tries to reconnect and triggers:
   - The \ref agora::recording::IRecordingEngineEventHandler::onConnectionInterrupted "onConnectionInterrupted" callback.
   - The \ref agora::recording::IRecordingEngineEventHandler::onConnectionStateChanged "onConnectionStateChanged" callback and switches to the #CONNECTION_STATE_RECONNECTING state.
   */
  CONNECTION_STATE_CONNECTED = 3,
  /** 4: The SDK keeps rejoining the channel after being disconnected from a joined channel because of network issues.

   - If the SDK cannot rejoin the channel within 10 seconds after being disconnected from Agora's edge server, the SDK triggers the \ref agora::recording::IRecordingEngineEventHandler::onConnectionLost "onConnectionLost" callback, stays in the #CONNECTION_STATE_RECONNECTING state, and keeps rejoining the channel.
   - If the SDK fails to rejoin the channel 20 minutes after being disconnected from Agora's edge server, the SDK triggers the \ref agora::recording::IRecordingEngineEventHandler::onConnectionStateChanged "onConnectionStateChanged" callback, switches to the #CONNECTION_STATE_FAILED state, and stops rejoining the channel.
   */
  CONNECTION_STATE_RECONNECTING = 4,
  /** 5: The SDK fails to connect to Agora's edge server or join the channel.

   If the SDK is banned from joining the channel by Agora's edge server (through the RESTful API), the SDK triggers the \ref agora::recording::IRecordingEngineEventHandler::onConnectionStateChanged "onConnectionStateChanged" callback and switches to the #CONNECTION_STATE_FAILED state.
   
   You must call the \ref agora::recording::IRecordingEngine::leaveChannel "leaveChannel" method to leave this state, and call the \ref agora::recording::IRecordingEngine::joinChannel "joinChannel" method again to rejoin the channel.
   */
  CONNECTION_STATE_FAILED = 5,
};


/** Reasons for a connection state change. */
enum ConnectionChangedReasonType 
{
  /** 0: The SDK is connecting to Agora's edge server. */
  CONNECTION_CHANGED_CONNECTING = 0,
  /** 1: The SDK has joined the channel successfully. */
  CONNECTION_CHANGED_JOIN_SUCCESS = 1,
  /** 2: The connection between the SDK and Agora's edge server is interrupted. */
  CONNECTION_CHANGED_INTERRUPTED = 2,
  /** 3: The connection between the SDK and Agora's edge server is banned by Agora's edge server. */
  CONNECTION_CHANGED_BANNED_BY_SERVER = 3,
  /** 4: The SDK fails to join the channel for more than 20 minutes and stops reconnecting to the channel. */
  CONNECTION_CHANGED_JOIN_FAILED = 4,
  /** 5: The SDK has left the channel. */
  CONNECTION_CHANGED_LEAVE_CHANNEL = 5,
};

enum MAX_USER_ACCOUNT_LENGTH_TYPE
{
  MAX_USER_ACCOUNT_LENGTH = 256
};

/** Identifiers of a user. */
struct UserInfo {
  /** The user ID. */
  uid_t uid;
  /** The user account. */
  char userAccount[MAX_USER_ACCOUNT_LENGTH];
  UserInfo()
    : uid(0) {
      userAccount[0] = '\0';
  }
};

/* Properties of the audio volume information. An array containing the user ID and volume information for each speaker. */
struct AudioVolumeInfo {
    /* The user ID of the speaker. The uid of the local user is 0. */
    uid_t uid;
    /*The volume of the speaker. The value ranges between 0 (lowest volume) and 255 (highest volume).*/
    uint32_t volume;

    AudioVolumeInfo()
    : uid(0), volume(0) { }
};

/** The statistics of the remote video stream. 
 * For details, see [Statistics of remote video streams](https://docs.agora.io/en/Interactive%20Broadcast/in-call_quality_windows?platform=Windows#statistics-of-remote-video-streams). 
 */
struct RemoteVideoStats {
  /** Delay (ms). */
  int delay;
  /** The width (pixel) of the remote video. */
  int width;
  /** The height (pixel) of the remote video. */
  int height;
  /** The receiving bitrate (Kbps). */
  int receivedBitrate;
  /** The decoder output frame rate (fps) of the remote video. 
   * 
   * @note This parameter is only valid in Composite Recording mode or when the video decoding format is set as YUV.
   */
  int decoderOutputFrameRate;
  /** The video stream type:
   * - 0: High-stream video.
   * - 1: Low-stream video.
   */
  REMOTE_VIDEO_STREAM_TYPE rxStreamType;

  RemoteVideoStats()
    : delay(0),
      width(0),
      height(0),
      receivedBitrate(0),
      decoderOutputFrameRate(0),
      rxStreamType(REMOTE_VIDEO_STREAM_HIGH) {}
};

/** The statistics of the remote audio stream. 
 * For details, see [Statistics of remote audio streams](https://docs.agora.io/en/Interactive%20Broadcast/in-call_quality_windows?platform=Windows#statistics-of-remote-audio-streams).
 */
struct RemoteAudioStats {
  /** The audio quality.
   * - 0: The quality is unknown.
   * - 1: The quality is excellent.
   * - 2: The quality is quite good, but the bitrate may be slightly lower than 1.
   * - 3: Users can feel the communication slightly impaired.
   * - 4: Users cannot communicate smoothly.
   * - 5: The quality is so bad that users can barely communicate.
   * - 6: The quality is down and users cannot communicate at all.
   * - 7: Users cannot detect the quality (Not in use).
   * - 8: Detecting the quality. 
   */
  int quality;
  /** The network delay (ms) from the sender to the receiver. */
  int networkTransportDelay;
  /** The jitter buffer delay (ms) at the receiver. */
  int jitterBufferDelay;
  /** The packet loss rate (%) in the reported interval. */
  int audioLossRate;

  RemoteAudioStats()
    : quality(0),
      networkTransportDelay(0),
      jitterBufferDelay(0),
      audioLossRate(0) {}
};

/** The statistics of IRecordingEngine. */
struct RecordingStats
{
    /** The length of time (s) when the recording server is in the channel, 
     * represented by an aggregate value.
     */
    uint32_t duration;
    /** The total number of received bytes, represented by an aggregate value.
     */
    uint32_t rxBytes;
    /** The receiving bitrate (Kbps), represented by an instantaneous value.
     */
    uint32_t rxKBitRate;
    /** The receiving bitrate (Kbps) of audio, represented by an instantaneous value.
     */
    uint32_t rxAudioKBitRate;
    /** The receiving bitrate (Kbps) of video, represented by an instantaneous value.
     */
    uint32_t rxVideoKBitRate;
    /** The latency (ms) between the recording client and the Agora server.
     */
    uint32_t lastmileDelay;
    /** The number of users in the channel.
     * 
     * - Communication profile: userCount = The number of users in the channel (including the recording server).
     * - Live broadcast profile: userCount = The number of hosts in the channel + 1.
     */
    uint32_t userCount;
    /** Application CPU usage (%).
     */
    double cpuAppUsage;
    /** System CPU usage (%).
     */
    double cpuTotalUsage;
  RecordingStats()
      : duration(0)
      , rxBytes(0)
      , rxKBitRate(0)
      , rxAudioKBitRate(0)
      , rxVideoKBitRate(0)
      , lastmileDelay(0)
      , userCount(0)
      , cpuAppUsage(0)
      , cpuTotalUsage(0) {}
};

/** The parameters of the audio frame in PCM format. */
class AudioPcmFrame {
    public:
    AudioPcmFrame(u64_t frame_ms, uint_t sample_rates, uint_t samples);
    ~AudioPcmFrame();
    public:
    /** Timestamp of the frame.*/
    u64_t frame_ms_;
    /** Number of audio channels.*/
    uint_t channels_; // 1
    /** Bitrate of the sampling data.*/
    uint_t sample_bits_; // 16
    /** Sampling rate.*/
    uint_t sample_rates_; // 8k, 16k, 32k
    /** Number of samples of the frame.*/
    uint_t samples_;

    /** Audio frame buffer.*/
    const uchar_t *pcmBuf_;
    /** Size of the audio frame buffer.*/
    uint_t pcmBufSize_;
};

/** The parameters of the audio frame in AAC format. */
class AudioAacFrame {
    public:
    explicit AudioAacFrame(u64_t frame_ms);
    ~AudioAacFrame();

    /** Audio frame buffer.*/
    const uchar_t *aacBuf_;
    /** Timestamp of the frame.*/
    u64_t frame_ms_;
    /** Size of the audio frame buffer.*/
    uint_t aacBufSize_;
    /** Number of audio channels.*/
    uint_t channels_;
    /** Bitrate of the audio.*/
    uint_t bitrate_;
};

/** The audio frame format.*/
struct AudioFrame {
    /** The audio frame format, PCM or ACC. */
    AUDIO_FRAME_TYPE type;
    union {
        /** Audio data in PCM format. See the structure of \ref AudioPcmFrame "AudioPcmFrame".*/
        AudioPcmFrame *pcm;
        /** Audio data in AAC format. See the structure of \ref AudioAacFrame "AudioAacFrame".*/
        AudioAacFrame *aac;
    } frame;

    AudioFrame();
    ~AudioFrame();
    /** See \ref agora::linuxsdk::MEMORY_TYPE "MEMORY_TYPE". */
    MEMORY_TYPE mType;
};

/** The parameters of the video frame in YUV format. */
class VideoYuvFrame {
    public:
    VideoYuvFrame(u64_t frame_ms, uint_t width, uint_t height, uint_t ystride,
            uint_t ustride, uint_t vstride);
    ~VideoYuvFrame();

    /** Timestamp of the frame.*/
    u64_t frame_ms_;

    /** Y buffer pointer.*/
    const uchar_t *ybuf_;
    /** U buffer pointer.*/
    const uchar_t *ubuf_;
    /** V buffer pointer.*/
    const uchar_t *vbuf_;

    /** Width of the video in the number of pixels.*/
    uint_t width_;
    /** Height of the video in the number of pixels.*/
    uint_t height_;

    /** Line span of the Y buffer.*/
    uint_t ystride_;
    /** Line span of the U buffer.*/
    uint_t ustride_;
    /** Line span of the V buffer.*/
    uint_t vstride_;

    //all
    /** Video frame buffer.*/
    const uchar_t *buf_;
    /** Size of the video frame buffer.*/
    uint_t bufSize_;
};

/** The parameters of the video frame in H.264 format. */
struct VideoH264Frame {
    public:
    VideoH264Frame():
        frame_ms_(0),
        frame_num_(0),
        buf_(NULL),
        bufSize_(0)
    {}

    ~VideoH264Frame(){}
    /** Timestamp of the frame.*/
    u64_t frame_ms_;
    /** Index of the frame.*/
    uint_t frame_num_;

    //all
    /** Video frame buffer.*/
    const uchar_t *buf_;
    /** Size of the video frame buffer.*/
    uint_t bufSize_;
};

/** The parameters of the video frame in H.265 format. */
struct VideoH265Frame {
    public:
    VideoH265Frame():
        frame_ms_(0),
        frame_num_(0),
        buf_(NULL),
        bufSize_(0)
    {}

    ~VideoH265Frame(){}
    /** Timestamp (ms) of the frame.*/
    u64_t frame_ms_;
    /** Index of the frame.*/
    uint_t frame_num_;

    //all
    /** Video frame buffer.*/
    const uchar_t *buf_;
    /** Size of the video frame buffer.*/
    uint_t bufSize_;
};

/** The parameters of the video frame in JPG format. */
struct VideoJpgFrame {
    public:
    VideoJpgFrame():
        frame_ms_(0),
        buf_(NULL),
        bufSize_(0){}

   ~VideoJpgFrame() {}
    /** Timestamp of the frame.*/
    u64_t frame_ms_;

    //all
    /** Video frame buffer.*/
    const uchar_t *buf_;
    /** Size of the video frame buffer.*/
    uint_t bufSize_;
};

/** The video frame format.*/
struct VideoFrame {
    /** The video frame format, Yuv, H264 or Jpg. */
    VIDEO_FRAME_TYPE type;
    union {
        /** Video data in Yuv format. See the structure of \ref VideoYuvFrame "VideoYuvFrame".*/
        VideoYuvFrame *yuv;
        /** Video data in H264 format. See the structure of \ref VideoH264Frame "VideoH264Frame".*/
        VideoH264Frame *h264;
        /** Video data in Jpg format. See the structure of \ref VideoJpgFrame "VideoJpgFrame".*/
        VideoJpgFrame *jpg;
        /** Video data in H265 format. See the structure of \ref VideoH265Frame "VideoH265Frame".*/
        VideoH265Frame *h265;
    } frame;

    /** The rotation of the video frame, 0, 90, 180 or 270. */
    int rotation_; // 0, 90, 180, 270
    VideoFrame();
    ~VideoFrame();

    /** See \ref agora::linuxsdk::MEMORY_TYPE "MEMORY_TYPE". */
    MEMORY_TYPE mType;
};

/** Supported watermark types. */
enum WatermarkType {
  /** The text watermark. */
  WATERMARK_TYPE_LITERA,

  /** The timestamp watermark. */
  WATERMARK_TYPE_TIMESTAMP,

  /** The image watermark.*/
  WATERMARK_TYPE_IMAGE
};

/** The configuration of the text watermark. */
struct LiteraWatermarkConfig {
  /** The watermark text in the string format.
   * 
   * @note
   * - The supported characters depend on the font. The default font is NotoSansMonoCJKsc-Regular. See the [font introduction](https://www.google.com/get/noto/help/cjk/). To change the font, you can also set the `font_file_path` parameter to specify the path of the font file.
   * - Supports word wrap. The text will wrap to next line when it exceeds the watermark rectangle. 
   * - Supports line breaks.
   * - There is no limit on the string length. The display of the text on the watermark rectangle is influenced by the font size and the size of the watermark rectangle. The part that exceeds the rectangle will not be displayed.
   */
  const char* wm_litera;
  /** The path of the font file. If not specified, use the default font NotoSansMonoCJKsc-Regular.
   * 
   * @note Supports font files in the formats such as ttf and otf.
   */
  const char* font_file_path;
  /** The font size. The default value is 10, which equals to 10 x 15 points at 144 dpi.
   */
  uint32_t font_size;
  /** The horizontal shift (pixel) of the rectangle 
   * containing the watermark from the left border of the canvas.
   * The default value is 0.
   */
  uint32_t offset_x;
  /** The vertical shift (pixel) of the rectangle 
   * containing the watermark from the top border of the canvas.
   * The default value is 0.
   */
  uint32_t offset_y;
  /** The width (pixel) of the rectangle containing the watermark.
   * The default value is 0.
   */
  uint32_t wm_width;
  /** The height (pixel) of the rectangle containing the watermark. 
   * The default value is 0.
   */
  uint32_t wm_height;
};

/** The configuration of the timestamp watermark.
 * 
 * The dynamic timestamp shows the current time of the recording server, 
 * such as "2019:06:18 14:30:35". 
 */
struct TimestampWatermarkConfig {
  /** The font size. The default value is 10, which equals to 10 x 15 points at 144 dpi.
   */
  uint32_t font_size;
  /** The horizontal shift (pixel) of the rectangle 
   * containing the watermark from the left border of the canvas.
   * The default value is 0.
   */
  uint32_t offset_x;
  /** The vertical shift (pixel) of the rectangle 
   * containing the watermark from the top border of the canvas.
   * The default value is 0.
   */
  uint32_t offset_y;
  /** The width (pixel) of the rectangle containing the watermark.
   * The default value is 0.
   */
  uint32_t wm_width;
  /** The height (pixel) of the rectangle containing the watermark. 
   * The default value is 0.
   */
  uint32_t wm_height;
};

/** The configuration of the image watermark. */
struct ImageWatermarkConfig {
  /** The path of the image file.
   * 
   * @note
   * - Only supports local PNG images.
   * - The resolution of the image should not exceed 480p.
   * - If the image is smaller than the watermark rectangle, the SDK centers the image and does not stretch the image; if the image is larger than the watermark rectangle, the SDK scales down the image and then centers the image in the watermark rectangle.
   */
  const char* image_path;
  /** The horizontal shift (pixel) of the rectangle 
   * containing the watermark from the left border of the canvas.
   * The default value is 0.
   */
  uint32_t offset_x;
  /** The vertical shift (pixel) of the rectangle 
   * containing the watermark from the top border of the canvas.
   * The default value is 0.
   */
  uint32_t offset_y;
  /** The width (pixel) of the rectangle containing the watermark.
   * The default value is 0.
   */
  uint32_t wm_width;
  /** The height (pixel) of the rectangle containing the watermark. 
   * The default value is 0.
   */
  uint32_t wm_height;
};


/** The configuration of a watermark. */
struct WatermarkConfig {
  /** The union of the watermark configurations. */
  union WatermarkConfigUnion{
    /** The configuration of the text watermark. See LiteraWatermarkConfig. */
    LiteraWatermarkConfig litera;
    /** The configuration of the timestamp watermark. See TimestampWatermarkConfig. */
    TimestampWatermarkConfig timestamp;
    /** The configuration of the image watermark. See ImageWatermarkConfig. */
    ImageWatermarkConfig image;

    WatermarkConfigUnion() {
      memset(this, 0, sizeof(*this));
    }
  };

  /** Choose one of the three watermark types. See #WatermarkType. */
  WatermarkType type;
  /** Define the parameters of the watermark type that you 
   * choose in the `type` parameter. For details, see WatermarkConfigUnion. 
   */
  WatermarkConfigUnion config;
};

/** The layout setting of the videos in composite recording mode. */
typedef struct VideoMixingLayout
{
    /** The setting of the region. */
    struct Region {
        /** The UID of the user (communication mode)/host (live broadcast mode) displaying the video in the region. */
        uid_t uid;
        /** The relative horizontal position of the top-left corner of the region. The value is between 0.0 and 1.0. */
        double x;//[0,1]
        /** The relative vertical position of the top-left corner of the region. The value is between 0.0 and 1.0. */
        double y;//[0,1]
        /** The relative width of the region. The value is between 0.0 and 1.0. */
        double width;//[0,1]
        /** The relative height of the region. The value is between 0.0 and 1.0. */
        double height;//[0,1]
        //  Optional
        //  [0, 1.0] where 0 denotes throughly transparent, 1.0 opaque
        /** The transparency of the image. The value is between 0.0 (transparent) and 1.0 (opaque).
         */
        double alpha;

        /** Render mode.
         
         - RENDER_MODE_HIDDEN(0): (Default) Cropped mode. Uniformly scale the video until it fills the visible boundaries (cropped). One dimension of the video may have clipped contents.
         - RENDER_MODE_FIT(1): Fit mode. Uniformly scale the video until one of its dimension fits the boundary (zoomed to fit). Areas that are not filled due to the disparity in the aspect ratio will be filled with black.
         */
        int renderMode;//RENDER_MODE_HIDDEN: Crop, RENDER_MODE_FIT: Zoom to fit
        Region()
            :uid(0)
             , x(0)
             , y(0)
             , width(0)
             , height(0)
             , alpha(1.0)
             , renderMode(1)
        {}

    };
    /** The width of the canvas (the display window or screen). */
    int canvasWidth;
    /** The height of the canvas (the display window or screen). */
    int canvasHeight;
    /** The background color of the canvas (the display window or screen) in RGB hex value.
     
     @note If you set the \ref agora::recording::RecordingConfig::defaultVideoBg "defaultVideoBg" parameter in \ref agora::recording::RecordingConfig "Recordingconfig" when calling the \ref agora::recording::IRecordingEngine::joinChannel "joinChannel" method, the `backgroundColor` parameter does not take effect.
     */
    const char* backgroundColor;//e.g. "#C0C0C0" in RGB
    /** The number of the users (communication mode)/hosts (live broadcast mode) in the channel. */
    uint32_t regionCount;
    /** The user (communication mode)/host (live broadcast mode) list of #VideoMixingLayout. 
     * Each user (communication mode)/host (live broadcast mode) in the channel 
     * has a region to display the video on the screen with the following parameters 
     * to be set. See \ref agora::linuxsdk::VideoMixingLayout::Region "Region" to set parameters. 
     */
    const Region* regions;
    /** User-defined data.*/
    const char* appData;
    /** The length of the user-defined data. */
    int appDataLength;
    /** Sets whether or not to show the last frame of a user in the region 
     * after the user leaves the channel:
     * 
     * - true: The user's last frame shows in the region.
     * - false: (Default) The user's last frame does not show in the region.
     */
    bool keepLastFrame;
    /** Sets the number of watermarks that you want to add to the video.
     * 
     * You can add up to 15 watermarks, including one timestamp watermark, 
     * four image watermarks, and ten text watermarks.
     */
    uint32_t wm_num;
    /** The configuration of the watermarks. Pointer to an array of WatermarkConfig.
     */
    WatermarkConfig * wm_configs;
    VideoMixingLayout()
        :canvasWidth(0)
         , canvasHeight(0)
         , backgroundColor(NULL)
         , regionCount(0)
         , regions(NULL)
         , appData(NULL)
         , appDataLength(0)
         , keepLastFrame(false)
         , wm_num(0)
         , wm_configs(NULL)
    {}
} VideoMixingLayout;

/** User information. */
typedef struct UserJoinInfos {
    /** The relative path of the recorded files and recording log. */
    const char* storageDir;
    //new attached info add below

    UserJoinInfos():
        storageDir(NULL)
    {}
}UserJoinInfos;


}
}

#endif
