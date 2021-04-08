#include "../../agorasdk/AgoraSdk.h"
#include "string.h"
#include "iostream"
#include "jni.h"

#include "jni/commonJniDef.h"

using namespace std;
namespace jniproxy
{

enum AgoraRecordingMode {
  kRecordingModeDefault,
  kRecordingModeRawData
};

class AgoraJniProxySdk : public agora::AgoraSdk {
public:
  typedef void (*callback_t ) (void);
  AgoraJniProxySdk();
  ~AgoraJniProxySdk(); 
  void initialize();
  virtual void onError(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code);
  virtual void onWarning(int warn);

  virtual void onJoinChannelSuccess(const char * channelId, agora::linuxsdk::uid_t uid);

  virtual void onLeaveChannel(agora::linuxsdk::LEAVE_PATH_CODE code);
  virtual void onUserJoined(agora::linuxsdk::uid_t uid, agora::linuxsdk::UserJoinInfos &infos);
  virtual void onRemoteVideoStreamStateChanged(agora::linuxsdk::uid_t uid, agora::linuxsdk::RemoteStreamState state, agora::linuxsdk::RemoteStreamStateChangedReason reason);
  virtual void onRemoteAudioStreamStateChanged(agora::linuxsdk::uid_t uid, agora::linuxsdk::RemoteStreamState state, agora::linuxsdk::RemoteStreamStateChangedReason reason);
  virtual void onUserOffline(agora::linuxsdk::uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason);
  virtual void audioFrameReceived(unsigned int uid, const agora::linuxsdk::AudioFrame *frame) const;
  virtual void videoFrameReceived(unsigned int uid, const agora::linuxsdk::VideoFrame *frame) const;
  virtual void onActiveSpeaker(unsigned int uid);
  virtual void onAudioVolumeIndication(const agora::linuxsdk::AudioVolumeInfo* speakers, unsigned int speakerNum); 
  virtual void onFirstRemoteVideoDecoded(uid_t uid, int width, int height, int elapsed);
  virtual void onFirstRemoteAudioFrame(uid_t uid, int elapsed);
  virtual void onReceivingStreamStatusChanged(bool receivingAudio, bool receivingVideo);
  virtual void onConnectionLost();
  virtual void onConnectionInterrupted();

  virtual void onRejoinChannelSuccess(const char* channelId, uid_t uid);
  virtual void onConnectionStateChanged(agora::linuxsdk::ConnectionStateType stats, agora::linuxsdk::ConnectionChangedReasonType reason);
  virtual void onRemoteAudioStats(uid_t uid, const agora::linuxsdk::RemoteAudioStats& stats);
  virtual void onRemoteVideoStats(uid_t uid, const agora::linuxsdk::RemoteVideoStats& stats);
  virtual void onRecordingStats(const agora::linuxsdk::RecordingStats& stats);
  virtual void onLocalUserRegistered(uid_t uid, const char* userAccount);
  virtual void onUserInfoUpdated(uid_t uid, const agora::linuxsdk::UserInfo& info);


  void setJobAgoraJavaRecording(jobject job){
    mJavaAgoraJavaRecordingObject = job;
  }
  void setJcAgoraJavaRecording(jclass jc){
    mJavaAgoraJavaRecordingClass = jc;
  }
  void stopJavaProc(JNIEnv* env);
  void setJavaRecordingPath(JNIEnv* env, std::string& storeDir);
  void setRecordingMode(AgoraRecordingMode mode) {
    mRecordingMode = mode;
  }

  jobject newJObject(JNIEnv* env, jclass jcls, jmethodID jmtd) const;

  jclass newGlobalJClass(JNIEnv* env, const char* className);
  jobject newGlobalJObject(JNIEnv* env, jclass jc, const char* signature);
  //cache field ids & method ids
  void cacheJavaCBFuncMethodIDs4Video(JNIEnv* env, const char* className);
  //yuv,h264,h265,jpg init method
  void cacheJavaVideoFrameInitMethodIDs(JNIEnv* env, const char* className);
  void cacheAudioPcmFrame(JNIEnv* env);
  void cacheJavaObject(JNIEnv* env);
  //void cacheJavaCBFuncMethodIDs4YUV(JNIEnv* env, const char* className);
  //void cacheJavaCBFuncMethodIDs4YUV(JNIEnv* env, const char* className);
  //void cacheJavaCBFuncMethodIDs4YUV(JNIEnv* env, const char* className);
  //void cacheJavaCBFuncMethodIDs4YUV(JNIEnv* env, const char* className);
  jmethodID safeGetMethodID(JNIEnv* env, jclass clazz, const char* name, const char* sig) const;
  jfieldID safeGetFieldID(JNIEnv* env, jclass clazz, const char* name, const char* sig) const;
  jfieldID safeGetFieldID2(JNIEnv* env, jclass clazz, const char* name, const char* sig) const;
  jobject newGlobalJObject2(JNIEnv* env, jclass jc, jmethodID initMid) const;

public:
   
private:
  void initJavaObjects(JNIEnv* env, bool init);
  int staticInitCBFuncMid(JNIEnv* env, jclass jclazz);
  int staticInitVideoYuvFrameFid(JNIEnv* env, jclass jclazz);
  
  template<typename T1, typename T2 >
  int staticInitCommonFrameFid(JNIEnv* env, jclass clazz, GETID_TYPE type, T1& src, T2& dest);

private:
  static jmethodID mJavaRecvVideoMtd;
  static jmethodID mJavaRecvAudioMtd;
  static jmethodID mJavaVideoFrameInitMtd;
  //yuv jpg h264 h265 init method
  static jmethodID mJavaVideoYuvFrameInitMtd;
  static jmethodID mJavaVideoJpgFrameInitMtd;
  static jmethodID mJavaVideoH264FrameInitMtd;
  static jmethodID mJavaVideoH265FrameInitMtd;

  static jmethodID m_CBObjectMethodIDs[MID_CBOBJECT_NUM];
  static jfieldID m_VideoYuvFrameFieldIDs[FID_YUVNUM];
  static jfieldID m_VideoH264FrameFieldIDs[FID_H264NUM];
  static jfieldID m_VideoH265FrameFieldIDs[FID_H265NUM];
  static jfieldID m_VideoJpgFrameFieldIDs[FID_JPGNUM];
  
  static jfieldID m_AudioFrameFieldIDs[FID_AF_NUM];
  static jfieldID m_AudioPcmFrameFieldIDs[FID_PCMNUM];
  static jfieldID m_AudioAacFrameFieldIDs[FID_AACNUM];
  static jfieldID m_FieldIDs[FID_AACNUM];
  static jfieldID m_AudioVolumeInfoFieldIDs[FID_AUDIO_VOLUME_INFO_NUM];

  static jfieldID m_AudioStatsFieldIDs[FID_AUDIO_STATS_NUM];
  static jfieldID m_VideoStatsFieldIDs[FID_VIDEO_STATS_NUM];
  static jfieldID m_RecordingStatsFieldIDs[FID_RECORDING_STATS_NUM];
  
private:
  //audio
  bool fillJAudioFrameByFields(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame, jclass jcAudioFrame, jobject& jobAudioFrame) const;
  //pcm
  bool fillAudioPcmFrame(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame,jclass& jcAudioFrame, jobject& jobAudioFrame) const;
  bool fillPcmAllFields(JNIEnv* env, jobject& job, jclass& jc, const agora::linuxsdk::AudioFrame*& frame) const;
  //aac
  bool fillAudioAacFrame(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame,jclass& jcAudioFrame, jobject& jobAudioFrame) const;
  bool fillAacAllFields(JNIEnv* env, jobject& job, jclass& jc, const agora::linuxsdk::AudioFrame*& frame) const;

  //video
  bool fillVideoFrameByFields(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass jcVideoFrame, jobject jobVideoFrame) const;
  bool fillVideoOfYUV(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const;
  bool fillVideoOfJPG(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const;
  bool fillVideoOfH264(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const;
  bool fillVideoOfH265(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const;
  void checkAudioArraySize(JNIEnv* env, long size)const ;
  void checkVideoArraySize(JNIEnv* env, long size)const ;
private:
  std::string m_logdir;
  AgoraRecordingMode mRecordingMode;
private:
  //define java object
  //video
  jclass mJavaVideoFrameClass;
  mutable jobject mJavaVideoFrameObject;
  jclass mJavaVideoYuvFrameClass;
  mutable jobject mJavaVideoYuvFrameObject;
  
  jclass mJavaVideoJpgFrameClass;
  mutable jobject mJavaVideoJpgFrameObject;
  
  jclass mJavaVideoH264FrameClass;
  mutable jobject mJavaVideoH264FrameObject;

  jclass mJavaVideoH265FrameClass;
  mutable jobject mJavaVideoH265FrameObject;
  
  //type
  jclass mJavaVideoFrameTypeClass;
  jobject mJavaVideoFrameTypeObject;
  jmethodID mJavaVideoFrameTypeInitMtd; 
  jfieldID mJavaVideoFrameTypeTypeFid;

  //audio
  jclass mJavaAudioFrameClass;
  mutable jobject mJavaAudioFrameObject;

  //pcm field
  jfieldID mJavaAudioAacFrameFid;

  //pcm
  jclass mJavaAudioPcmFrameClass;
  jmethodID mJavaAudioPcmFrameInitMtd;
  mutable jobject mJavaAudioPcmFrameObject;

  //aac
  jclass mJavaAudioAacFrameClass;
  jmethodID mJavaAudioAacFrameInitMtd;
  mutable jobject mJavaAudioAacFrameObject;

  //AudioFrameType
  //jclass mJavaAudioFrameTypeClass;
  jobject mJavaAudioAacType;
  jobject mJavaAudioPcmType;

  jfieldID mJavaVideoFrameYuvFid;
  jfieldID mJavaVideoFrameH264Fid;
  jfieldID mJavaVideoFrameH265Fid;
  jfieldID mJavaVideoFrameJpgFid;
  jmethodID mJavaAudioFrameInitMtd;
  jclass mJavaAudioVolumeInfoClass;

  jclass mJavaAudioStatsClass;
  jclass mJavaVideoStatsClass;
  jclass mJavaRecordingStatsClass;
  jobject mJavaAudioStatsObject;
  jobject mJavaVideoStatsObject;
  jobject mJavaRecordingStatsObject;

private:
  jclass mJavaAgoraJavaRecordingClass;
  jobject mJavaAgoraJavaRecordingObject;
  mutable jbyteArray audioArray_;
  mutable long audioArrayLen_;
  mutable jbyteArray videoArray_;
  mutable long videoArrayLen_;
};

}//endnamespace
