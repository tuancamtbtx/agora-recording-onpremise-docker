#include "IAgoraLinuxSdkCommon.h"
#include "./jni/io_agora_recording_RecordingSDK.h"
#include "RecordingJni.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "helper.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

using namespace jniproxy;

jmethodID AgoraJniProxySdk::mJavaRecvAudioMtd = NULL;
jmethodID AgoraJniProxySdk::mJavaVideoFrameInitMtd = NULL;
jmethodID AgoraJniProxySdk::mJavaVideoYuvFrameInitMtd = NULL;
jmethodID AgoraJniProxySdk::mJavaVideoH264FrameInitMtd = NULL;
jmethodID AgoraJniProxySdk::mJavaVideoH265FrameInitMtd = NULL;
jmethodID AgoraJniProxySdk::mJavaVideoJpgFrameInitMtd = NULL;

jmethodID AgoraJniProxySdk::m_CBObjectMethodIDs[MID_CBOBJECT_NUM];
jfieldID AgoraJniProxySdk::m_VideoYuvFrameFieldIDs[FID_YUVNUM];
jfieldID AgoraJniProxySdk::m_AudioFrameFieldIDs[FID_AF_NUM];

jfieldID AgoraJniProxySdk::m_VideoH264FrameFieldIDs[FID_H264NUM];
jfieldID AgoraJniProxySdk::m_VideoH265FrameFieldIDs[FID_H265NUM];
jfieldID AgoraJniProxySdk::m_VideoJpgFrameFieldIDs[FID_JPGNUM];  
jfieldID AgoraJniProxySdk::m_AudioPcmFrameFieldIDs[FID_PCMNUM];
jfieldID AgoraJniProxySdk::m_AudioAacFrameFieldIDs[FID_AACNUM];
jfieldID AgoraJniProxySdk::m_AudioVolumeInfoFieldIDs[FID_AUDIO_VOLUME_INFO_NUM];
jfieldID AgoraJniProxySdk::m_AudioStatsFieldIDs[FID_AUDIO_STATS_NUM];
jfieldID AgoraJniProxySdk::m_VideoStatsFieldIDs[FID_VIDEO_STATS_NUM];
jfieldID AgoraJniProxySdk::m_RecordingStatsFieldIDs[FID_RECORDING_STATS_NUM];

  
template<typename T1, typename T2>
int AgoraJniProxySdk::staticInitCommonFrameFid(JNIEnv* env, jclass clazz, GETID_TYPE type, T1& src, T2& dest){
  for (int i = 0; i < sizeof(src) / sizeof(src[0]); i++){
    const JavaObjectMethod& m = src[i];
    jfieldID id = safeGetFieldID(env, clazz, m.name, m.signature);
    if(!id){
      cout<<"staticInitCommonFrameFid failed,name:"<<m.name<<",m.signature:"<<m.signature<<endl;
      continue;
    }
    dest[m.id] = id;
  }
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

JavaVM* g_jvm = NULL;

class AttachThreadScoped
{
public:
  explicit AttachThreadScoped(JavaVM* jvm, bool detach)
    : attached_(false), jvm_(jvm), env_(NULL), detach_(detach) {
    jint ret_val = jvm->GetEnv(reinterpret_cast<void**>(&env_),JNI_VERSION_1_4);
    if (ret_val == JNI_EDETACHED) {
      // Attach the thread to the Java VM.
      ret_val = jvm_->AttachCurrentThread((void**)&env_, NULL);
      attached_ = ret_val >= 0;
    }
  }
  ~AttachThreadScoped() {
    if (detach_ && attached_ && (jvm_->DetachCurrentThread() < 0)) {
      //TBD
    }
  }
  void detach(){
    if (attached_ && jvm_->DetachCurrentThread() < 0) {
      //TBD
    }
  }

  void forceDetach() {
    jvm_->DetachCurrentThread();
  }
  JNIEnv* env() { return env_; }
private:
  bool attached_;
  JavaVM* jvm_;
  JNIEnv* env_;
  bool detach_;
};

#define INIT_VIDEO_ARRAY_LEN 512*1024
#define INIT_AUDIO_ARRAY_LEN 256 

AgoraJniProxySdk::AgoraJniProxySdk():AgoraSdk()
  , audioArray_(NULL)
  , videoArray_(NULL)
  , audioArrayLen_(0)
  , videoArrayLen_(0)
  , mRecordingMode(kRecordingModeDefault) {
  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk constructor");
  mJavaAgoraJavaRecordingClass = NULL;
  mJavaAgoraJavaRecordingObject = NULL;
  mJavaVideoFrameClass = NULL;
  mJavaVideoFrameObject = NULL;
  mJavaVideoYuvFrameClass = NULL;
  mJavaVideoYuvFrameObject = NULL;
  mJavaVideoJpgFrameClass = NULL;
  mJavaVideoJpgFrameObject = NULL;
  mJavaVideoH264FrameClass = NULL;
  mJavaVideoH265FrameObject = NULL;
  //audio
  mJavaAudioFrameClass = NULL;
  mJavaAudioFrameObject = NULL;
  mJavaAudioAacFrameClass = NULL;
  mJavaAudioAacFrameObject = NULL;
  mJavaAudioPcmFrameClass =NULL;
  mJavaAudioPcmFrameObject =NULL;

  mJavaAudioFrameInitMtd = NULL;
  mJavaAudioAacFrameInitMtd = NULL;
  mJavaAudioPcmFrameInitMtd = NULL;

  mJavaVideoFrameTypeClass = NULL;
  mJavaVideoFrameTypeObject = NULL;
  mJavaAudioFrameInitMtd = NULL;

  mJavaAudioVolumeInfoClass = NULL;

  mJavaAudioStatsClass = NULL;
  mJavaVideoStatsClass = NULL;
  mJavaRecordingStatsClass = NULL;
}
AgoraJniProxySdk::~AgoraJniProxySdk(){
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if(!env) return;
  cout<<"AgoraJniProxySdk destructor begin"<<endl;
  initJavaObjects(env, false);
  cout<<"AgoraJniProxySdk destructor end"<<endl;
  if(audioArray_ != NULL) {
      env->DeleteGlobalRef(audioArray_);
      audioArray_ = NULL;
  }
  if(videoArray_ != NULL) {
      env->DeleteGlobalRef(videoArray_);
      videoArray_ = NULL;
  }
}
extern "C++" {
void AgoraJniProxySdk::initialize(){
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if(!env) return;
  checkAudioArraySize(env, INIT_AUDIO_ARRAY_LEN);
  checkVideoArraySize(env, INIT_VIDEO_ARRAY_LEN);
  initJavaObjects(env, true);
  cacheJavaCBFuncMethodIDs4Video(env, CN_VIDEO_FRAME);
  cacheJavaVideoFrameInitMethodIDs(env, CN_VIDEO_YUV_FRAME);
  cacheJavaVideoFrameInitMethodIDs(env, CN_VIDEO_H264_FRAME);
  cacheJavaVideoFrameInitMethodIDs(env, CN_VIDEO_H265_FRAME);
  cacheJavaVideoFrameInitMethodIDs(env, CN_VIDEO_JPG_FRAME);
  //cacheAudioPcmFrame(env);
  cacheJavaObject(env);
  staticInitCBFuncMid(env, mJavaAgoraJavaRecordingClass);
  //staticInitVideoYuvFrameFid(env,mJavaVideoYuvFrameClass);
  //video
  staticInitCommonFrameFid(env, mJavaVideoYuvFrameClass, FIDID,jVideoYuvFrameFields, m_VideoYuvFrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaVideoH264FrameClass, FIDID,jVideoH264FrameFields, m_VideoH264FrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaVideoH265FrameClass, FIDID,jVideoH265FrameFields, m_VideoH265FrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaVideoJpgFrameClass, FIDID,jVideoJpgFrameFields, m_VideoJpgFrameFieldIDs);
  //audio
  staticInitCommonFrameFid(env, mJavaAudioFrameClass, FIDID,jAudioFrameFields, m_AudioFrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaAudioPcmFrameClass, FIDID,jAudioPcmFrameFields, m_AudioPcmFrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaAudioAacFrameClass, FIDID, jAudioAacFrameFields, m_AudioAacFrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaAudioVolumeInfoClass, FIDID, jAudioVolumeInfoFields, m_AudioVolumeInfoFieldIDs);
  env->SetObjectField(mJavaAudioFrameObject, m_AudioFrameFieldIDs[FID_AF_AAC], mJavaAudioAacFrameObject);
  env->SetObjectField(mJavaAudioFrameObject, m_AudioFrameFieldIDs[FID_AF_PCM], mJavaAudioPcmFrameObject);
  env->SetObjectField(mJavaVideoFrameObject, mJavaVideoFrameYuvFid, mJavaVideoYuvFrameObject);
  env->SetObjectField(mJavaVideoFrameObject, mJavaVideoFrameH264Fid, mJavaVideoH264FrameObject);
  env->SetObjectField(mJavaVideoFrameObject, mJavaVideoFrameH265Fid, mJavaVideoH265FrameObject);
  env->SetObjectField(mJavaVideoFrameObject, mJavaVideoFrameJpgFid, mJavaVideoJpgFrameObject);

  staticInitCommonFrameFid(env, mJavaAudioStatsClass, FIDID, jAudioStatsFields, m_AudioStatsFieldIDs);
  staticInitCommonFrameFid(env, mJavaVideoStatsClass, FIDID, jVideoStatsFields, m_VideoStatsFieldIDs);
  staticInitCommonFrameFid(env, mJavaRecordingStatsClass, FIDID, jRecordingStatsFields, m_RecordingStatsFieldIDs);
  }
}
int AgoraJniProxySdk::staticInitVideoYuvFrameFid(JNIEnv* env, jclass clazz){
  for (int i = 0; i < sizeof(jVideoYuvFrameFields) / sizeof(jVideoYuvFrameFields[0]); i++){
    const JavaObjectMethod& m = jVideoYuvFrameFields[i];
    jfieldID fid = safeGetFieldID(env, clazz, m.name, m.signature);
    if (!fid){
      cout<<"AgoraJniProxySdk::staticInitCBFuncMid failed get methid:"<<m.name<<endl;
      return -1;
    }
    m_VideoYuvFrameFieldIDs[m.id] = fid;
  }
  return 0;
}

int AgoraJniProxySdk::staticInitCBFuncMid(JNIEnv* env, jclass clazz){
  for (int i = 0; i < sizeof(jCBObjectMethods) / sizeof(jCBObjectMethods[0]); i++){
    const JavaObjectMethod& m = jCBObjectMethods[i];
    jmethodID mid = safeGetMethodID(env, clazz, m.name, m.signature);
    if (!mid){
      cout<<"AgoraJniProxySdk::staticInitCBFuncMid failed get methid:"<<m.name<<endl;
    }

    m_CBObjectMethodIDs[m.id] = mid;
  }
}
void AgoraJniProxySdk::cacheJavaObject(JNIEnv* env){
}
jobject AgoraJniProxySdk::newJObject(JNIEnv* env, jclass jcls, jmethodID jmtd) const{
  if(!jmtd || !jcls){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"newJObject but jcls or jmethodID not inited!");
    return NULL;
  }
  jobject job = env->NewObject(jcls, jmtd);
  if(!job) {
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"cannot get videoinit methodid");
    return NULL;
  }
  return job;
}

jclass AgoraJniProxySdk::newGlobalJClass(JNIEnv* env, const char* className){
  jclass localRef = env->FindClass(className);
  if(!localRef) {
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJClass cannot find class:%s",className);
    return NULL;
  }
  jclass globalJc = static_cast<jclass>(env->NewGlobalRef(localRef));
  if(!globalJc){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJClass cound not create global reference!",className);
    return NULL;
  }
  env->DeleteLocalRef(localRef);
  return globalJc;
}
jobject AgoraJniProxySdk::newGlobalJObject2(JNIEnv* env, jclass jc, jmethodID initMid) const{
  if(!jc || !initMid) {
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJObject but jc or initMid is NULL");
    return NULL;
  }
  jobject globalJob = env->NewGlobalRef(env->AllocObject(jc));
  if(!globalJob){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJObject new global reference failed ");
    return NULL;
  }
  return globalJob;
}

jobject AgoraJniProxySdk::newGlobalJObject(JNIEnv* env, jclass jc, const char* signature){
  jmethodID initMid = env->GetMethodID(jc, SG_MTD_INIT, signature);
  if(!initMid) {
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJObject cannot get init method for this signature:%s", signature);
    return NULL;
  }
  jobject globalJob = env->NewGlobalRef(env->AllocObject(jc));
  if(!globalJob){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJObject new global reference failed ");
    return NULL;
  }
  return globalJob;
}
jmethodID AgoraJniProxySdk::safeGetMethodID(JNIEnv* env, jclass clazz, const char* name, const char* sig) const {
  CPN(clazz);
  jmethodID mid = env->GetMethodID(clazz, name, sig);
  return mid;
}
jfieldID AgoraJniProxySdk::safeGetFieldID(JNIEnv* env, jclass clazz, const char* name, const char* sig) const {
  CPN(clazz);
  jfieldID fid = env->GetFieldID(clazz, name, sig);
  return fid;
}
jfieldID AgoraJniProxySdk::safeGetFieldID2(JNIEnv* env, jclass clazz, const char* name, const char* sig) const {
  jclass jc = env->FindClass("Lio/agora/recording/common/Common$AUDIO_FRAME_TYPE;");
  jfieldID fid = env->GetFieldID(jc, "type", "I");
  return fid;
}
void AgoraJniProxySdk::cacheAudioPcmFrame(JNIEnv* env){
}
void AgoraJniProxySdk::cacheJavaVideoFrameInitMethodIDs(JNIEnv* env, const char* className){
  if(className && !strcmp(className,CN_VIDEO_YUV_FRAME)){
    CP(mJavaVideoYuvFrameClass);
    mJavaVideoYuvFrameInitMtd = safeGetMethodID(env, mJavaVideoYuvFrameClass, SG_MTD_INIT, SN_MTD_VIDEO_YUV_FRAME_INIT);
    CP(mJavaVideoYuvFrameInitMtd);
    if(!mJavaVideoYuvFrameInitMtd) {
      //CM_LOG_DIR(m_logdir.c_str(), ERROR,"cannot get video yuv init methodid");
      return;
    }
  }
  if(className && !strcmp(className,CN_VIDEO_H264_FRAME)){
    CP(mJavaVideoH264FrameClass);
    mJavaVideoH264FrameInitMtd = safeGetMethodID(env, mJavaVideoH264FrameClass, SG_MTD_INIT, SN_MTD_VIDEO_H264_FRAME_INIT);
    CP(mJavaVideoH264FrameInitMtd);
    if(!mJavaVideoH264FrameInitMtd) {
      //CM_LOG_DIR(m_logdir.c_str(), ERROR,"cannot get video h264 init methodid");
      return;
    }
  }
  if(className && !strcmp(className,CN_VIDEO_H265_FRAME)){
    CP(mJavaVideoH265FrameClass);
    mJavaVideoH265FrameInitMtd = safeGetMethodID(env, mJavaVideoH265FrameClass, SG_MTD_INIT, SN_MTD_VIDEO_H265_FRAME_INIT);
    CP(mJavaVideoH265FrameInitMtd);
    if(!mJavaVideoH265FrameInitMtd) {
      //CM_LOG_DIR(m_logdir.c_str(), ERROR,"cannot get video h265 init methodid");
      return;
    }
  }
  if(className && !strcmp(className,CN_VIDEO_JPG_FRAME)){
    CP(mJavaVideoJpgFrameClass);
    mJavaVideoJpgFrameInitMtd = safeGetMethodID(env, mJavaVideoJpgFrameClass, SG_MTD_INIT, SN_MTD_VIDEO_JPG_FRAME_INIT);
    CP(mJavaVideoJpgFrameInitMtd);
    if(!mJavaVideoJpgFrameInitMtd) {
      //CM_LOG_DIR(m_logdir.c_str(), ERROR,"cannot get video Jpg init methodid");
      return;
    }
  }
}
void AgoraJniProxySdk::cacheJavaCBFuncMethodIDs4Video(JNIEnv* env, const char* className){
  if (!env) return;
  //AV class
  mJavaVideoFrameInitMtd = safeGetMethodID(env, mJavaVideoFrameClass, SG_MTD_INIT, VIDEO_FRAME_SIGNATURE);
  if(!mJavaVideoFrameInitMtd) {
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"cannot get videoinit methodid");
    return;
  }
  mJavaVideoFrameYuvFid = env->GetFieldID(mJavaVideoFrameClass, FID_VIDEO_FRAME_YUV, VIDEOFRAME_YUV_SIGNATURE);
  if(!mJavaVideoFrameYuvFid){
    cout<<"mJavaVideoFrameYuvFid is null"<<endl;
    return;
  }
  mJavaVideoFrameJpgFid = env->GetFieldID(mJavaVideoFrameClass, FID_VIDEO_FRAME_JPG, VIDEOFRAME_JPG_SIGNATURE);
  CP(mJavaVideoFrameJpgFid);
  mJavaVideoFrameH264Fid = env->GetFieldID(mJavaVideoFrameClass, FID_VIDEO_FRAME_H264, VIDEOFRAME_H264_SIGNATURE);
  CP(mJavaVideoFrameH264Fid);
  mJavaVideoFrameH265Fid = env->GetFieldID(mJavaVideoFrameClass, FID_VIDEO_FRAME_H265, VIDEOFRAME_H265_SIGNATURE);
  CP(mJavaVideoFrameH265Fid);

 }
void AgoraJniProxySdk::initJavaObjects(JNIEnv* env, bool init){
  if(!init) return;

  mJavaAudioStatsClass = newGlobalJClass(env, CN_AUDIO_STATS);
  CP(mJavaAudioStatsClass);
  mJavaAudioStatsObject = newGlobalJObject(env, mJavaAudioStatsClass, SN_MTD_COMMON_INIT);
  CP(mJavaAudioStatsObject);

  mJavaVideoStatsClass = newGlobalJClass(env, CN_VIDEO_STATS);
  CP(mJavaVideoStatsClass);
  mJavaVideoStatsObject = newGlobalJObject(env, mJavaVideoStatsClass, SN_MTD_COMMON_INIT);
  CP(mJavaVideoStatsObject);

  mJavaRecordingStatsClass = newGlobalJClass(env, CN_RECORDING_STATS);
  CP(mJavaRecordingStatsClass);
  mJavaRecordingStatsObject = newGlobalJObject(env, mJavaRecordingStatsClass, SN_MTD_COMMON_INIT);
  CP(mJavaRecordingStatsObject);

  mJavaVideoFrameClass = newGlobalJClass(env, CN_VIDEO_FRAME);
  CP(mJavaVideoFrameClass);
  mJavaVideoFrameObject = newGlobalJObject(env, mJavaVideoFrameClass, SN_MTD_COMMON_INIT);
  CP(mJavaVideoFrameObject);
  mJavaVideoYuvFrameClass = newGlobalJClass(env, CN_VIDEO_YUV_FRAME);
  CP(mJavaVideoYuvFrameClass);
  mJavaVideoYuvFrameObject = newGlobalJObject(env, mJavaVideoYuvFrameClass, SN_MTD_VIDEO_YUV_FRAME_INIT); 
  CP(mJavaVideoYuvFrameObject);
  mJavaVideoJpgFrameClass = newGlobalJClass(env, CN_VIDEO_JPG_FRAME);
  CP(mJavaVideoJpgFrameClass);
  mJavaVideoJpgFrameObject = newGlobalJObject(env, mJavaVideoJpgFrameClass, SN_MTD_COMMON_INIT);
  CP(mJavaVideoJpgFrameObject);
  mJavaVideoH264FrameClass = newGlobalJClass(env, CN_VIDEO_H264_FRAME);
  CP(mJavaVideoH264FrameClass);
  mJavaVideoH264FrameObject = newGlobalJObject(env, mJavaVideoH264FrameClass, SN_MTD_COMMON_INIT);
  CP(mJavaVideoH264FrameObject);
  mJavaVideoH265FrameClass = newGlobalJClass(env, CN_VIDEO_H265_FRAME);
  CP(mJavaVideoH265FrameClass);
  mJavaVideoH265FrameObject = newGlobalJObject(env, mJavaVideoH265FrameClass, SN_MTD_COMMON_INIT);
  CP(mJavaVideoH265FrameObject);
  mJavaVideoFrameTypeClass = newGlobalJClass(env, CN_VIDEO_FRAME_TYPE);
  CP(mJavaVideoFrameTypeClass);
  mJavaVideoFrameTypeInitMtd = safeGetMethodID(env, mJavaVideoFrameTypeClass, SG_MTD_INIT, SN_MTD_COMMON_INIT); 
  CP(mJavaVideoFrameTypeInitMtd);
  mJavaVideoFrameTypeObject = newGlobalJObject2(env, mJavaVideoFrameTypeClass, mJavaVideoFrameTypeInitMtd);
  CP(mJavaVideoFrameTypeObject);
  mJavaVideoFrameTypeTypeFid = safeGetFieldID(env, mJavaVideoFrameTypeClass, MTD_TYPE, SG_INT);
  CP(mJavaVideoFrameTypeTypeFid);
  //audio
  mJavaAudioFrameClass = newGlobalJClass(env, CN_AUDIO_FRAME);
  CP(mJavaAudioFrameClass);
  mJavaAudioFrameInitMtd = safeGetMethodID(env, mJavaAudioFrameClass, SG_MTD_INIT, SN_MTD_COMMON_INIT);
  CP(mJavaAudioFrameInitMtd);
  mJavaAudioFrameObject = newGlobalJObject2(env, mJavaAudioFrameClass,  mJavaAudioFrameInitMtd);
  CP(mJavaAudioFrameObject);
  //java audio receive func
  mJavaRecvAudioMtd = safeGetMethodID(env, mJavaAgoraJavaRecordingClass, CB_FUNC_RECEIVE_AUDIOFRAME, SN_CB_FUNC_RECEIVE_AUDIOFRAME);
  CP(mJavaRecvAudioMtd);

  //Audio Frame Type
  jclass audioTypeClass = env->FindClass(CN_AUDIO_FRAME_TYPE);
  jfieldID typeId = env->GetStaticFieldID(audioTypeClass, "AUDIO_FRAME_AAC", SN_AUDIO_FRAME_TYPE);
  mJavaAudioAacType = env->GetStaticObjectField(audioTypeClass, typeId);
  jfieldID typeIdPcm = env->GetStaticFieldID(audioTypeClass, "AUDIO_FRAME_RAW_PCM", SN_AUDIO_FRAME_TYPE);
  mJavaAudioPcmType = env->GetStaticObjectField(audioTypeClass, typeIdPcm);

  //pcm
  mJavaAudioPcmFrameClass = newGlobalJClass(env, CN_AUDIO_PCM_FRAME);
  CP(mJavaAudioPcmFrameClass);
  mJavaAudioPcmFrameInitMtd = safeGetMethodID(env, mJavaAudioPcmFrameClass, SG_MTD_INIT, SN_INIT_MTD_AUDIO_FRAME);
  CP(mJavaAudioPcmFrameInitMtd);
  mJavaAudioPcmFrameObject = newGlobalJObject2(env, mJavaAudioPcmFrameClass,  mJavaAudioPcmFrameInitMtd);
  CP(mJavaAudioPcmFrameObject);
  //aac
  mJavaAudioAacFrameClass = newGlobalJClass(env, CN_AUDIO_AAC_FRAME);
  CP(mJavaAudioAacFrameClass);
  mJavaAudioAacFrameInitMtd = safeGetMethodID(env, mJavaAudioAacFrameClass, SG_MTD_INIT, SN_INIT_MTD_AUDIO_AAC_FRAME);
  CP(mJavaAudioAacFrameInitMtd);
  mJavaAudioAacFrameObject = newGlobalJObject2(env, mJavaAudioAacFrameClass,  mJavaAudioAacFrameInitMtd);
  CP(mJavaAudioAacFrameObject);

  mJavaAudioVolumeInfoClass = newGlobalJClass(env, CN_AUDIO_VOLUME_INFO);
  CP(mJavaAudioVolumeInfoClass);
}

bool AgoraJniProxySdk::fillAacAllFields(JNIEnv* env, jobject& job, jclass& jc, const agora::linuxsdk::AudioFrame*& frame) const {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::AUDIO_FRAME_AAC) return false;
  agora::linuxsdk::AudioAacFrame *f = frame->frame.aac;
  //frame_ms_
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(job, m_AudioAacFrameFieldIDs[FID_AAC_FRAMEMS], jlong(frame_ms_));
  //aacBuf_
  long aacBufSize_ = f->aacBufSize_;
  checkAudioArraySize(env, aacBufSize_);
  if(audioArray_ == NULL) {
     //CM_LOG_DIR(m_logdir.c_str(), ERROR, "AudioArray is NULL");
     return false;
  }
  env->SetByteArrayRegion(audioArray_, 0, aacBufSize_, (jbyte*)f->aacBuf_);
  env->SetObjectField(job, m_AudioAacFrameFieldIDs[FID_AAC_BUF], audioArray_);
  //aacBufSize_
  env->SetLongField(job, m_AudioAacFrameFieldIDs[FID_AAC_BUFSIZE], jlong(aacBufSize_));
  //channels
  int channels_ = f->channels_;
  env->SetIntField(job, m_AudioAacFrameFieldIDs[FID_AAC_CHANNELS], jint(channels_));
  //bitrate
  int bitrate_ = f->bitrate_;
  env->SetIntField(job, m_AudioAacFrameFieldIDs[FID_AAC_BITRATE], jint(bitrate_));

  return true;
}

bool AgoraJniProxySdk::fillPcmAllFields(JNIEnv* env, jobject& job, jclass& jc, const agora::linuxsdk::AudioFrame*& frame) const {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::AUDIO_FRAME_RAW_PCM) return false;
  agora::linuxsdk::AudioPcmFrame *f = frame->frame.pcm;
  //frame_ms_
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_FRAMEMS], jlong(frame_ms_));
  //channels_
  long channels_ = f->channels_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_CHANNELS], jlong(channels_));
  long sample_bits_ = f->sample_bits_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_SAMPLEBITS], jlong(sample_bits_));
  //sample_rates_
  long sample_rates_ = f->sample_rates_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_SAMPLERATES], jlong(sample_rates_));
  //sampel
  long samples__ = f->samples_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_SAMPLES], jlong(samples__));
  //pcmBuf_
  long pcmBufSize_ = f->pcmBufSize_;
  checkAudioArraySize(env, pcmBufSize_);
  if(audioArray_ == NULL){
      //CM_LOG_DIR(m_logdir.c_str(), ERROR, "audio array is null");
      return false;
  }
  env->SetByteArrayRegion(audioArray_, 0, pcmBufSize_, (jbyte*)f->pcmBuf_);
  env->SetObjectField(job, m_AudioPcmFrameFieldIDs[FID_PCM_BUF], audioArray_);
  //pcmBufSize_
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_BUFSIZE], jlong(pcmBufSize_));
  return true;
}

bool AgoraJniProxySdk::fillJAudioFrameByFields(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame, jclass jcAudioFrame, jobject& jobAudioFrame) const {
  //1.find main class
  if (frame->type == agora::linuxsdk::AUDIO_FRAME_RAW_PCM) {
    //call one function
    if(!fillAudioPcmFrame(env, frame, jcAudioFrame,jobAudioFrame)){
      //CM_LOG_DIR(m_logdir.c_str(), INFO,"Warning: fillAudioPcmFrame failed!!!!!");
      return false;
    }
  }else if (frame->type == agora::linuxsdk::AUDIO_FRAME_AAC) {
    //do things here
    if(!fillAudioAacFrame(env, frame, jcAudioFrame,jobAudioFrame)){
      //CM_LOG_DIR(m_logdir.c_str(), INFO,"Warning: fillAudioAacFrame failed!!!!!");
      return false;
    }
  }
  return true;
}

bool AgoraJniProxySdk::fillAudioAacFrame(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame, \
            jclass& jcAudioFrame, jobject& jobAudioFrame) const  {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::AUDIO_FRAME_AAC) return false;
  env->SetObjectField(jobAudioFrame, m_AudioFrameFieldIDs[FID_AF_TYPE], mJavaAudioAacType);
  if(!fillAacAllFields(env, mJavaAudioAacFrameObject, jcAudioFrame, frame)){
    //CM_LOG_DIR(m_logdir.c_str(), INFO,"fillAacAllFields failed!");
    return false;
  }
  //Fill in the jobAdudioFrame
  //env->SetObjectField(jobAudioFrame, m_AudioFrameFieldIDs[FID_AF_AAC], mJavaAudioAacFrameObject);
  return true;
}
bool AgoraJniProxySdk::fillAudioPcmFrame(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame, \
            jclass& jcAudioFrame, jobject& jobAudioFrame) const  {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::AUDIO_FRAME_RAW_PCM) return false;
  env->SetObjectField(jobAudioFrame, m_AudioFrameFieldIDs[FID_AF_TYPE], mJavaAudioPcmType);
  //fill all fields of AudioPcmFrame jobject
  if(!fillPcmAllFields(env, mJavaAudioPcmFrameObject, jcAudioFrame, frame)){
    //CM_LOG_DIR(m_logdir.c_str(), INFO,"fillPcmAllFields failed!");
    return false;
  }
  //Fill in the jobAdudioFrame
  //env->SetObjectField(jobAudioFrame, m_AudioFrameFieldIDs[FID_AF_PCM], mJavaAudioPcmFrameObject);
  return true;
}
bool AgoraJniProxySdk::fillVideoOfYUV(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::VIDEO_FRAME_RAW_YUV) return false;
  if(!env || !frame) return false;
  agora::linuxsdk::VideoYuvFrame *f = frame->frame.yuv;
  if(!f) {
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"yuv frame is nullptr");
    return false;
  }
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(mJavaVideoYuvFrameObject, m_VideoYuvFrameFieldIDs[FID_FRAMEMS], jlong(frame_ms_));
  long bufSize_ = f->bufSize_;
  checkVideoArraySize(env, bufSize_);
  env->SetByteArrayRegion(videoArray_, 0, bufSize_, (jbyte*)f->buf_);
  env->SetObjectField(mJavaVideoYuvFrameObject, m_VideoYuvFrameFieldIDs[FID_YUVBUF], videoArray_);
  env->SetLongField(mJavaVideoYuvFrameObject, m_VideoYuvFrameFieldIDs[FID_YUVBUFSIZE], jlong(bufSize_));
  env->SetIntField(mJavaVideoYuvFrameObject, m_VideoYuvFrameFieldIDs[FID_YUVWIDTH],jint(f->width_));
  env->SetIntField(mJavaVideoYuvFrameObject, m_VideoYuvFrameFieldIDs[FID_YUVHEIGHT], jint(f->height_));
  env->SetIntField(mJavaVideoYuvFrameObject, m_VideoYuvFrameFieldIDs[FID_YUVYSTRIDE], jint(f->ystride_));
  env->SetIntField(mJavaVideoYuvFrameObject, m_VideoYuvFrameFieldIDs[FID_YUVUSTRIDE], jint(f->ustride_));
  env->SetIntField(mJavaVideoYuvFrameObject, m_VideoYuvFrameFieldIDs[FID_YUVVSTRIDE], jint(f->vstride_));
  //env->SetObjectField(jobVideoFrame, mJavaVideoFrameYuvFid, mJavaVideoYuvFrameObject);
  return  true;
}
bool AgoraJniProxySdk::fillVideoOfJPG(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const{
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk::fillVideoOfJPG enter" );
  if(frame->type != agora::linuxsdk::VIDEO_FRAME_JPG) return false;
  if(!env || !frame) return false;

  agora::linuxsdk::VideoJpgFrame *f = frame->frame.jpg;
  if(!f) return false;
  //4.fill all fields
  //4.1 get & set of this subclass object
  //frame_ms_
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(mJavaVideoJpgFrameObject, m_VideoJpgFrameFieldIDs[FID_JPG_FRAMEMS], jlong(frame_ms_));
  const unsigned char* buf_ = f->buf_;
  long bufSize_ = f->bufSize_;
  checkVideoArraySize(env, bufSize_);
  env->SetByteArrayRegion(videoArray_, 0, bufSize_, (jbyte*)f->buf_);
  env->SetObjectField(mJavaVideoJpgFrameObject, m_VideoJpgFrameFieldIDs[FID_JPG_BUF], videoArray_);
  //bufSize_
  env->SetLongField(mJavaVideoJpgFrameObject, m_VideoJpgFrameFieldIDs[FID_JPG_BUFSIZE], jlong(bufSize_));
  //6.fill jobVideFrame
  //env->SetObjectField(jobVideoFrame, mJavaVideoFrameJpgFid, mJavaVideoJpgFrameObject);
  return  true;
}
bool AgoraJniProxySdk::fillVideoOfH264(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type == agora::linuxsdk::VIDEO_FRAME_JPG || frame->type == agora::linuxsdk::VIDEO_FRAME_RAW_YUV || frame->type == agora::linuxsdk::VIDEO_FRAME_H265) return false;
  if(!env || !frame) return false;
  agora::linuxsdk::VideoH264Frame *f = frame->frame.h264;
  if(!f) return false;
  //frame_ms_
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(mJavaVideoH264FrameObject, m_VideoH264FrameFieldIDs[FID_H264_FRAMEMS], jlong(frame_ms_));
  //frame_num_
  long frame_num_ = f->frame_num_;
  env->SetLongField(mJavaVideoH264FrameObject, m_VideoH264FrameFieldIDs[FID_H264_FRAMENUM], jlong(frame_num_));
  //buf_
  const unsigned char* buf_ = f->buf_;
  long bufSize_ = f->bufSize_;
  checkVideoArraySize(env, bufSize_);
  env->SetByteArrayRegion(videoArray_, 0, bufSize_, (jbyte*)f->buf_);
  env->SetObjectField(mJavaVideoH264FrameObject, m_VideoH264FrameFieldIDs[FID_H264_BUF], videoArray_);
  //bufSize_
  env->SetLongField(mJavaVideoH264FrameObject, m_VideoH264FrameFieldIDs[FID_H264_BUF_SIZE], jlong(bufSize_));
  //5.get subclass field
  //env->SetObjectField(jobVideoFrame, mJavaVideoFrameH264Fid, mJavaVideoH264FrameObject);
  return  true;
}
bool AgoraJniProxySdk::fillVideoOfH265(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type == agora::linuxsdk::VIDEO_FRAME_JPG || frame->type == agora::linuxsdk::VIDEO_FRAME_RAW_YUV || frame->type == agora::linuxsdk::VIDEO_FRAME_H264) return false;
  if(!env || !frame) return false;
  agora::linuxsdk::VideoH265Frame *f = frame->frame.h265;
  if(!f) return false;
  //frame_ms_
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(mJavaVideoH265FrameObject, m_VideoH265FrameFieldIDs[FID_H265_FRAMEMS], jlong(frame_ms_));
  //frame_num_
  long frame_num_ = f->frame_num_;
  env->SetLongField(mJavaVideoH265FrameObject, m_VideoH265FrameFieldIDs[FID_H265_FRAMENUM], jlong(frame_num_));
  //buf_
  const unsigned char* buf_ = f->buf_;
  long bufSize_ = f->bufSize_;
  checkVideoArraySize(env, bufSize_);
  env->SetByteArrayRegion(videoArray_, 0, bufSize_, (jbyte*)f->buf_);
  env->SetObjectField(mJavaVideoH265FrameObject, m_VideoH265FrameFieldIDs[FID_H265_BUF], videoArray_);
  //bufSize_
  env->SetLongField(mJavaVideoH265FrameObject, m_VideoH265FrameFieldIDs[FID_H265_BUF_SIZE], jlong(bufSize_));
  //5.get subclass field
  //env->SetObjectField(jobVideoFrame, mJavaVideoFrameH265Fid, mJavaVideoH265FrameObject);
  return  true;
}
bool AgoraJniProxySdk::fillVideoFrameByFields(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass jcVideoFrame, jobject jobVideoFrame) const{
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  bool ret = false;
  if(!env || !frame || !jcVideoFrame || !jobVideoFrame){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"AgoraJniProxySdk::fillVideoFrameByFields para error!");
    return ret;
  }
  if (frame->type == agora::linuxsdk::VIDEO_FRAME_RAW_YUV) {
    if(!fillVideoOfYUV(env, frame, jcVideoFrame, jobVideoFrame)){
      //CM_LOG_DIR(m_logdir.c_str(), INFO,"fill subclass falied!");
      return false;
    }
  }else if(frame->type == agora::linuxsdk::VIDEO_FRAME_JPG){
    if(!fillVideoOfJPG(env, frame, jcVideoFrame, jobVideoFrame)) {
      //CM_LOG_DIR(m_logdir.c_str(), INFO,"fill subclass falied!");
      return false;
    }
  }else if(frame->type == agora::linuxsdk::VIDEO_FRAME_H264){
    if(!fillVideoOfH264(env, frame, jcVideoFrame, jobVideoFrame))
    {
      //CM_LOG_DIR(m_logdir.c_str(), INFO,"fillVideoOfH264 failed!");
      return false;
    }
  }else if(frame->type == agora::linuxsdk::VIDEO_FRAME_H265){
    if(!fillVideoOfH265(env, frame, jcVideoFrame, jobVideoFrame))
    {
      //CM_LOG_DIR(m_logdir.c_str(), INFO,"fillVideoOfH265 failed!");
      return false;
    }
  }else{
    return false;
  }
  return true;
}
void AgoraJniProxySdk::videoFrameReceived(unsigned int uid, const agora::linuxsdk::VideoFrame *frame) const {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!fillVideoFrameByFields(env, frame, mJavaVideoFrameClass, mJavaVideoFrameObject))
  {
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"jni fillVideoFrameByFields failed!" );
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_VIDEOFRAME_RECEIVED], jlong(long(uid)),frame->type, mJavaVideoFrameObject, frame->rotation_);
  CHECK_EXCEPTION(env, "");
  return;
}
//TODO  use the same parameter
void AgoraJniProxySdk::audioFrameReceived(unsigned int uid, const agora::linuxsdk::AudioFrame *frame) const {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!fillJAudioFrameByFields(env, frame, mJavaAudioFrameClass, mJavaAudioFrameObject)) {
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"fillJAudioFrameByFields failed!" );
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_AUDIOFRAME_RECEIVED], jlong(long(uid)), mJavaAudioFrameObject);
  CHECK_EXCEPTION(env, "");

  return;
}

void AgoraJniProxySdk::checkAudioArraySize(JNIEnv* env, long size) const {
    if(size > audioArrayLen_) {
        audioArrayLen_ = size + 10;
        if(audioArray_ != NULL) {
            env->DeleteGlobalRef(audioArray_);
        }
        jbyteArray tmp = env->NewByteArray(audioArrayLen_);
        if(tmp == NULL) {
            audioArrayLen_ = 0;
            audioArray_ = NULL;
            return;
        }
        audioArray_ = (jbyteArray)env->NewGlobalRef(tmp);
        env->DeleteLocalRef(tmp);
    }
}

void AgoraJniProxySdk::checkVideoArraySize(JNIEnv* env, long size)const  {
    if(size > videoArrayLen_) {
        videoArrayLen_ = size + 100;
        if(videoArray_ != NULL) {
            env->DeleteGlobalRef(videoArray_);
        }
        jbyteArray tmp = env->NewByteArray(videoArrayLen_);
        if(tmp == NULL) {
            videoArrayLen_ = 0;
            videoArray_ = NULL;
            return;
        }
        videoArray_ = (jbyteArray)env->NewGlobalRef(tmp);
        env->DeleteLocalRef(tmp);
    }
}

void AgoraJniProxySdk::onReceivingStreamStatusChanged(bool receivingAudio, bool receivingVideo) {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_RECEIVING_STREAM_STATUS_CHANGED]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_RECEIVING_STREAM_STATUS_CHANGED not inited");
    return;
  }

  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_RECEIVING_STREAM_STATUS_CHANGED], (jboolean)receivingAudio, (jboolean)receivingVideo);

  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onConnectionInterrupted() {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_CONNECTION_INTERRUPTED]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_CONNECTION_INTERRUPTED not inited");
    return;
  }

  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_CONNECTION_INTERRUPTED]);

  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onConnectionLost() {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_CONNECTION_LOST]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_CONNECTION_LOST not inited");
    return;
  }

  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_CONNECTION_LOST]);

  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onRemoteAudioStats(uid_t uid, const agora::linuxsdk::RemoteAudioStats& stats) {
  CHECK_PTR_RETURN(mJavaAudioStatsObject);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if (!m_CBObjectMethodIDs[MID_ON_REMOTE_AUDIO_STATS])
    return;

  env->SetIntField(mJavaAudioStatsObject, m_AudioStatsFieldIDs[FID_AUDIO_STATS::FID_QUALITY], jint((int)(stats.quality)));
  env->SetIntField(mJavaAudioStatsObject, m_AudioStatsFieldIDs[FID_AUDIO_STATS::FID_NETWORK_DELAY], jint((int)(stats.networkTransportDelay)));
  env->SetIntField(mJavaAudioStatsObject, m_AudioStatsFieldIDs[FID_AUDIO_STATS::FID_JITTER_DELAY], jint((int)(stats.jitterBufferDelay)));
  env->SetIntField(mJavaAudioStatsObject, m_AudioStatsFieldIDs[FID_AUDIO_STATS::FID_LOSS_RATE], jint((int)(stats.audioLossRate)));
  
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_REMOTE_AUDIO_STATS], jlong((long)(uid)), mJavaAudioStatsObject);
  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onRemoteVideoStats(uid_t uid, const agora::linuxsdk::RemoteVideoStats& stats) {
  CHECK_PTR_RETURN(mJavaVideoStatsObject);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if (!m_CBObjectMethodIDs[MID_ON_REMOTE_VIDEO_STATS])
    return;

  env->SetIntField(mJavaVideoStatsObject, m_VideoStatsFieldIDs[FID_VIDEO_STATS::FID_DELAY], jint((int)(stats.delay)));
  env->SetIntField(mJavaVideoStatsObject, m_VideoStatsFieldIDs[FID_VIDEO_STATS::FID_WIDTH], jint((int)(stats.width)));
  env->SetIntField(mJavaVideoStatsObject, m_VideoStatsFieldIDs[FID_VIDEO_STATS::FID_HEIGHT], jint((int)(stats.height)));
  env->SetIntField(mJavaVideoStatsObject, m_VideoStatsFieldIDs[FID_VIDEO_STATS::FID_RECEIVEDBITRATE], jint((int)(stats.receivedBitrate)));
  env->SetIntField(mJavaVideoStatsObject, m_VideoStatsFieldIDs[FID_VIDEO_STATS::FID_DECODEROUTPUTFRAMERATE], jint((int)(stats.decoderOutputFrameRate)));
  env->SetIntField(mJavaVideoStatsObject, m_VideoStatsFieldIDs[FID_VIDEO_STATS::FID_RXSTRAMTYPE], jint((int)(stats.rxStreamType)));

  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_REMOTE_VIDEO_STATS], jlong((long)(uid)), mJavaVideoStatsObject);
  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onRecordingStats(const agora::linuxsdk::RecordingStats& stats) {
  CHECK_PTR_RETURN(mJavaRecordingStatsObject);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv *env = ats.env();
  if (!env) return;
  if (!m_CBObjectMethodIDs[MID_ON_RECORDING_STATS])
    return;

  env->SetIntField(mJavaRecordingStatsObject, m_RecordingStatsFieldIDs[FID_RECORDING_STATS::FID_DURATION], jint((int)(stats.duration)));
  env->SetIntField(mJavaRecordingStatsObject, m_RecordingStatsFieldIDs[FID_RECORDING_STATS::FID_RXBYTES], jint((int)(stats.rxBytes)));
  env->SetIntField(mJavaRecordingStatsObject, m_RecordingStatsFieldIDs[FID_RECORDING_STATS::FID_RXKBITRATE], jint((int)(stats.rxKBitRate)));
  env->SetIntField(mJavaRecordingStatsObject, m_RecordingStatsFieldIDs[FID_RECORDING_STATS::FID_RXAUDIOKBITRATE], jint((int)(stats.rxAudioKBitRate)));
  env->SetIntField(mJavaRecordingStatsObject, m_RecordingStatsFieldIDs[FID_RECORDING_STATS::FID_RXVIDEOKBITRATE], jint((int)(stats.rxVideoKBitRate)));
  env->SetIntField(mJavaRecordingStatsObject, m_RecordingStatsFieldIDs[FID_RECORDING_STATS::FID_LASTMILEDELAY], jint((int)(stats.lastmileDelay)));
  env->SetIntField(mJavaRecordingStatsObject, m_RecordingStatsFieldIDs[FID_RECORDING_STATS::FID_USERCOUNT], jint((int)(stats.userCount)));
  env->SetIntField(mJavaRecordingStatsObject, m_RecordingStatsFieldIDs[FID_RECORDING_STATS::FID_CPUAPPUSAGE], jint((int)(stats.cpuAppUsage)));
  env->SetIntField(mJavaRecordingStatsObject, m_RecordingStatsFieldIDs[FID_RECORDING_STATS::FID_CPUTOTALUSAGE], jint((int)(stats.cpuTotalUsage)));

  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_RECORDING_STATS], mJavaRecordingStatsObject);
  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onAudioVolumeIndication(const agora::linuxsdk::AudioVolumeInfo* speakers, unsigned int speakerNum) {
  if(speakerNum == 0)
      return;

  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);

  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_AUDIO_VOLUME_INDICATION]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_AUDIO_VOLUME_INDICATION not inited");
    return;
  }

  jobjectArray array = (jobjectArray)env->NewObjectArray(speakerNum, mJavaAudioVolumeInfoClass, NULL); 
  for(int i = 0; i < speakerNum; i++) {
      jobject obj =env->AllocObject(mJavaAudioVolumeInfoClass);
      CP(obj);
      env->SetLongField(obj, m_AudioVolumeInfoFieldIDs[FID_AUDIO_VOLUME_INFO::FID_UID], jlong((long)(speakers[i].uid)));
      env->SetIntField(obj, m_AudioVolumeInfoFieldIDs[FID_AUDIO_VOLUME_INFO::FID_VOLUME], jint(speakers[i].volume));
      env->SetObjectArrayElement(array, i, obj);
  }

  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_AUDIO_VOLUME_INDICATION],array);

  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onActiveSpeaker(unsigned int uid) {

  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk User:%u" ,uid , " is active speaker");
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);

  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_ACTIVE_SPEAKER]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_ACTIVE_SPEAKER not inited");
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_ACTIVE_SPEAKER], jlong((long)(uid)));
  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onUserJoined(agora::linuxsdk::uid_t uid, agora::linuxsdk::UserJoinInfos &infos) {
  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk User:%u" ,uid , " joined, RecordingDir:%s" , (infos.storageDir? infos.storageDir:"NULL") );
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  std::string store_dir = std::string(infos.storageDir);
  m_logdir = store_dir;
  
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_USERJOINED]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_USERJOINED not inited");
    return;
  }
  jstring jstrRecordingDir = env->NewStringUTF(store_dir.c_str());
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_USERJOINED], jlong((long)(uid)),jstrRecordingDir);
  CHECK_EXCEPTION(env, "");
  return;
}
void AgoraJniProxySdk::onRemoteVideoStreamStateChanged(agora::linuxsdk::uid_t uid, agora::linuxsdk::RemoteStreamState state, agora::linuxsdk::RemoteStreamStateChangedReason reason) {
  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk User:%u" ,uid , " video state changed to %d, reason : %d" , state, reason);
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_USER_VIDEO_STATE_CHANGED]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_USER_VIDEO_STATE_CHANGED not inited");
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_USER_VIDEO_STATE_CHANGED], jlong((long)(uid)), jint((int)(state)), jint((int)(reason)));
  CHECK_EXCEPTION(env, "");
  return;
}

void AgoraJniProxySdk::onRemoteAudioStreamStateChanged(agora::linuxsdk::uid_t uid, agora::linuxsdk::RemoteStreamState state, agora::linuxsdk::RemoteStreamStateChangedReason reason) {
  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk User:%u" ,uid , " audio state changed to %d, reason : %d" , state, reason);
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_USER_AUDIO_STATE_CHANGED]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_USER_AUDIO_STATE_CHANGED not inited");
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_USER_AUDIO_STATE_CHANGED], jlong((long)(uid)), jint((int)(state)), jint((int)(reason)));
  CHECK_EXCEPTION(env, "");
  return;
}

void AgoraJniProxySdk::onUserOffline(agora::linuxsdk::uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason) {
  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk onUserOffline User:%u",uid, ",reason:%d",static_cast<int>(reason));
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_USEROFFLINE]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_USEROFFLINE not inited" );
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_USEROFFLINE], jlong((long)(uid)),jint(int(reason)));
  CHECK_EXCEPTION(env, "");

  return;
}
void AgoraJniProxySdk::onLeaveChannel(agora::linuxsdk::LEAVE_PATH_CODE code) {
  cout << __FUNCTION__ << endl;
  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk onLeaveChannel");
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);

  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_LEAVECHANNEL]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_LEAVECHANNEL not inited" );
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_LEAVECHANNEL], jint((int)(code)));
  CHECK_EXCEPTION(env, "");
  ats.forceDetach();
  return;
}
void AgoraJniProxySdk::onWarning(int warn) {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_WARNING]){
    //CM_LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_WARNING not inited" );
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_WARNING], warn);
  CHECK_EXCEPTION(env, "");
  return;
}

void AgoraJniProxySdk::onFirstRemoteVideoDecoded(uid_t uid, int width, int height, int elapsed) {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;

  if(!m_CBObjectMethodIDs[MID_ON_FIRST_REMOTE_VIDEO_DECODED]) {
    //CM_LOG_DIR(m_logdir.c_str(), INFO,"MID_ON_FIRST_REMOTE_VIDEO_DECODED not inited!");
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_FIRST_REMOTE_VIDEO_DECODED], jlong((long)(uid)), width, height, elapsed);
  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onFirstRemoteAudioFrame(uid_t uid, int elapsed) {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;

  if(!m_CBObjectMethodIDs[MID_ON_FIRST_REMOTE_AUDIO_FRAME]) {
    //CM_LOG_DIR(m_logdir.c_str(), INFO,"MID_ON_FIRST_REMOTE_AUDIO_FRAME not inited!");
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_FIRST_REMOTE_AUDIO_FRAME], jlong((long)(uid)), elapsed);
  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onError(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code) {
  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk onError");
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;

  if(!m_CBObjectMethodIDs[MID_ON_ERROR]) {
    //CM_LOG_DIR(m_logdir.c_str(), INFO,"MID_ON_ERROR not inited!");
    return;
  }
  if (stat_code != agora::linuxsdk::STAT_OK) {
    env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_ERROR], error, jint((int)(stat_code)));
    CHECK_EXCEPTION(env, "");
  }
  leaveChannel();
  ats.forceDetach();
  return;
}

void AgoraJniProxySdk::onJoinChannelSuccess(const char * channelId, agora::linuxsdk::uid_t uid) {
  //CM_LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk onJoinChannelSuccess,channel:%s, uid:%d", channelId, uid);
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;

  if(!m_CBObjectMethodIDs[MID_ON_JOINCHANNEL_SUCCESS]) {
    //CM_LOG_DIR(m_logdir.c_str(), INFO, "MID_ON_JOINCHANNEL_SUCCESS not inited!");
    return;
  }
  jstring jstRChannelId = env->NewStringUTF(channelId);
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_JOINCHANNEL_SUCCESS], jstRChannelId , jlong((long)(uid)));
  CHECK_EXCEPTION(env, "");
  return;
}

void AgoraJniProxySdk::onLocalUserRegistered(uid_t uid, const char* userAccount) {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv *env = ats.env();
  if (!env) return;

  if (!m_CBObjectMethodIDs[MID_ON_LOCAL_REGISTERED]) {
    return;
  }
  jstring jstUserAccount = env->NewStringUTF(userAccount);
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_LOCAL_REGISTERED], jlong((long)(uid)), jstUserAccount);
  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::onUserInfoUpdated(uid_t uid, const agora::linuxsdk::UserInfo& info) {
  std::string userAccount = info.userAccount;
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv *env = ats.env();
  if (!env) return;

  if(!m_CBObjectMethodIDs[MID_ON_USER_INFO_UPDATE]) {
    return;
  }

  jstring jstUserAccount = env->NewStringUTF(userAccount.c_str());
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_USER_INFO_UPDATE], jlong((long)(uid)), jstUserAccount);
  CHECK_EXCEPTION(env, "");
}


void AgoraJniProxySdk::onRejoinChannelSuccess(const char* channelId, agora::linuxsdk::uid_t uid) {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;

  if(!m_CBObjectMethodIDs[MID_ON_REJOINCHANNEL_SUCCESS]) {
    //CM_LOG_DIR(m_logdir.c_str(), INFO, "MID_ON_REJOINCHANNEL_SUCCESS not inited!");
    return;
  }
  jstring jstRChannelId = env->NewStringUTF(channelId);
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_REJOINCHANNEL_SUCCESS], jstRChannelId , jlong((long)(uid)));
  CHECK_EXCEPTION(env, "");
  return;
}

void AgoraJniProxySdk::onConnectionStateChanged(agora::linuxsdk::ConnectionStateType stats, agora::linuxsdk::ConnectionChangedReasonType reason) {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm, mRecordingMode == kRecordingModeDefault);
  JNIEnv* env = ats.env();
  if (!env) return;

  if(!m_CBObjectMethodIDs[MID_ON_CONNECTION_CHANGED]) {
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_CONNECTION_CHANGED], jint((int)(stats)), jint((int)(reason)));
  CHECK_EXCEPTION(env, "");
  return;
}

JNIEXPORT jint JNI_OnLoad(JavaVM* jvm, void* reserved) {
  g_jvm = jvm;
  return JNI_VERSION_1_4;
}

/*
 * Class:     AgoraJavaRecording
 * Method:    leaveChannel
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_io_agora_recording_RecordingSDK_leaveChannel
  (JNIEnv *, jobject, jlong nativeObjectRef) {
  cout<<"java call leaveChannel"<<endl;
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  if(!nativeHandle){
    return JNI_FALSE;
  }
  nativeHandle->leaveChannel();
  return JNI_TRUE;
}

char* getStringField(JNIEnv* env, jobject obj, const std::string& fieldName) {
  jclass jcObj = env->GetObjectClass(obj);
  if (!jcObj) {
    cout << "Can't find object class" << endl;
    return NULL;
  }

  jfieldID id = env->GetFieldID(jcObj, fieldName.c_str(), STRING_SIGNATURE);
  if (!id) {
    cout << "Cant get field id" << endl;
    return NULL;
  }

  jstring str = (jstring)env->GetObjectField(obj, id);
  const char* c_str = env->GetStringUTFChars(str, JNI_FALSE);
  int len = strlen(c_str);
  char* val = new char[len + 1];
  strncpy(val, c_str, len);
  val[len] = '\0';
  return val;
}

int getIntField(JNIEnv *env, jobject obj, const std::string& fieldName) {
  jclass jcObj = env->GetObjectClass(obj);
  if (!jcObj) {
    cout << "Can't find object class" << endl;
    return 0;
  }

  jfieldID id = env->GetFieldID(jcObj, fieldName.c_str(), INT_SIGNATURE);
  if (!id) {
    cout << "Cant get field id" << endl;
    return 0;
  }

  jint val = env->GetIntField(obj, id);
  return (int) val;
}

JNIEXPORT jint JNICALL Java_io_agora_recording_RecordingSDK_updateWatermarkConfigs
  (JNIEnv * env, jobject job, jlong nativeObjectRef, jobjectArray literals, jobjectArray timestamps,
   jobjectArray images) {
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  cout << __FUNCTION__ << endl;
  if(!nativeHandle){
    cout<<"setVideoMixingLayout nativeHandle is null"<<endl;
    return JNI_FALSE;
  }

  int literal_num = 0, timestamp_num = 0, image_num = 0;
  if (literals) {
    literal_num = (int)env->GetArrayLength(literals);
  }

  if (timestamps) {
    timestamp_num = (int) env->GetArrayLength(timestamps);
  }

  if (images) {
    image_num = (int) env->GetArrayLength(images);
  }
  
  cout << "literal watermark : " << literal_num <<", timestamp watermark : " << timestamp_num <<
    ", image watermark : " << image_num << endl;
  int total_num = literal_num + timestamp_num + image_num;
  if (total_num > 0) {
    agora::linuxsdk::WatermarkConfig *configs = new agora::linuxsdk::WatermarkConfig[total_num];
    for (int i = 0; i < literal_num; ++i) {
      configs[i].type = agora::linuxsdk::WATERMARK_TYPE_LITERA;
      jobject literal = env->GetObjectArrayElement(literals, i);
      if (!literal)
        continue;
      configs[i].config.litera.wm_litera = getStringField(env, literal, "wmLitera");
      configs[i].config.litera.font_size = getIntField(env, literal, "fontSize");
      configs[i].config.litera.offset_x = getIntField(env, literal, "offsetX");
      configs[i].config.litera.offset_y = getIntField(env, literal, "offsetY");
      configs[i].config.litera.wm_width = getIntField(env, literal, "wmWidth");
      configs[i].config.litera.wm_height = getIntField(env, literal, "wmHeight");
    }

    for (int i = 0; i < timestamp_num; ++i) {
      configs[literal_num + i].type = agora::linuxsdk::WATERMARK_TYPE_TIMESTAMP;
      jobject ts = env->GetObjectArrayElement(timestamps, i);
      if (!ts)
        continue;
      configs[literal_num + i].config.timestamp.font_size = getIntField(env, ts, "fontSize");
      configs[literal_num + i].config.timestamp.offset_x = getIntField(env, ts, "offsetX");
      configs[literal_num + i].config.timestamp.offset_y = getIntField(env, ts, "offsetY");
      configs[literal_num + i].config.timestamp.wm_width = getIntField(env, ts, "wmWidth");
      configs[literal_num + i].config.timestamp.wm_height = getIntField(env, ts, "wmHeight");
    }

    for (int i = 0; i < image_num; ++i) {

      configs[literal_num + timestamp_num + i].type = agora::linuxsdk::WATERMARK_TYPE_IMAGE;
      jobject img = env->GetObjectArrayElement(images, i);
      if (!img)
        continue;
      configs[literal_num + timestamp_num + i].config.image.image_path = getStringField(env, img, "imagePath");
      configs[literal_num + timestamp_num + i].config.image.offset_x = getIntField(env, img, "offsetX");
      configs[literal_num + timestamp_num + i].config.image.offset_y = getIntField(env, img, "offsetY");
      configs[literal_num + timestamp_num + i].config.image.wm_width = getIntField(env, img, "wmWidth");
      configs[literal_num + timestamp_num + i].config.image.wm_height = getIntField(env, img, "wmHeight");
    }

    nativeHandle->updateWatermarkConfigs(total_num, configs);
    for (int i = 0; i < total_num; ++i) {
      if (configs[i].type == agora::linuxsdk::WATERMARK_TYPE_LITERA) {
        if (configs[i].config.litera.wm_litera) {
          delete [] configs[i].config.litera.wm_litera;
        }
      } else if (configs[i].type == agora::linuxsdk::WATERMARK_TYPE_IMAGE) {
        if (configs[i].config.image.image_path) {
          delete [] configs[i].config.image.image_path;
        }
      }
    }
    delete [] configs;
  }

  return jint(0);

}

/*
 * Class:     AgoraJavaRecording
 * Method:    setVideoMixingLayout
 * Signature: (JLio/agora/recording/common/Common/VideoMixingLayout;)I
 */
JNIEXPORT jint JNICALL Java_io_agora_recording_RecordingSDK_setVideoMixingLayout
  (JNIEnv * env, jobject job, jlong nativeObjectRef, jobject jVideoMixLayout)
{
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  if(!nativeHandle){
    cout<<"setVideoMixingLayout nativeHandle is null"<<endl;
    return JNI_FALSE;
  }
  //convert jVideoMixLayout to c++ VideoMixLayout
	jclass jcVideoMixingLayout  = env->GetObjectClass(jVideoMixLayout); 
  if(!jcVideoMixingLayout){
    cout<<"jcVideoMixingLayout is NULL";
    return JNI_FALSE;
	}
  jfieldID jCanvasWidthID = env->GetFieldID(jcVideoMixingLayout, "canvasWidth", INT_SIGNATURE);
  jfieldID jCanvasHeightID = env->GetFieldID(jcVideoMixingLayout, "canvasHeight", INT_SIGNATURE);
  jfieldID jBackgroundColorID = env->GetFieldID(jcVideoMixingLayout, "backgroundColor", STRING_SIGNATURE);
  jfieldID jRegionCountID = env->GetFieldID(jcVideoMixingLayout, "regionCount", INT_SIGNATURE);
  jfieldID jRegionsID = env->GetFieldID(jcVideoMixingLayout, "regions", VIDEOMIXLAYOUT_SIGNATURE);
  jfieldID jAppDataID = env->GetFieldID(jcVideoMixingLayout, "appData", STRING_SIGNATURE);
  jfieldID jAppDataLengthID = env->GetFieldID(jcVideoMixingLayout, "appDataLength", INT_SIGNATURE);
  jfieldID jKeepLastFrame = env->GetFieldID(jcVideoMixingLayout, "keepLastFrame", INT_SIGNATURE);
  jfieldID jLiteralWmID = env->GetFieldID(jcVideoMixingLayout, "literalWms", VIDEOMIXLAYOUT_LITERALWM_SIGNATURE);
  jfieldID jTimestampWmID = env->GetFieldID(jcVideoMixingLayout, "timestampWms", VIDEOMIXLAYOUT_TIMESTAMPWM_SIGNATURE);
  jfieldID jImageWmID = env->GetFieldID(jcVideoMixingLayout, "imageWms", VIDEOMIXLAYOUT_IMAGEWM_SIGNATURE);
  
  if(!jCanvasWidthID || !jCanvasHeightID || !jBackgroundColorID || !jRegionCountID || !jRegionsID || !jKeepLastFrame){
    cout<<"Java_io_agora_record_AgoraJavaRecording_setVideoMixingLayout get fields failed!"<<endl;
    return JNI_FALSE;
  }
  //convert into cpp value
  jint canvasWidth = env->GetIntField(jVideoMixLayout,jCanvasWidthID); jint canvasHeight = env->GetIntField(jVideoMixLayout,jCanvasWidthID); jstring jstrBackgroundColor = (jstring)env->GetObjectField(jVideoMixLayout, jBackgroundColorID); const char* c_backgroundColor = env->GetStringUTFChars(jstrBackgroundColor, JNI_FALSE); 
  jint regionCount = env->GetIntField(jVideoMixLayout,jRegionCountID);
  jint keepLastFrame = env->GetIntField(jVideoMixLayout, jKeepLastFrame);
  
  //jstring jstrAppData = (jstring)env->GetObjectField(jVideoMixLayout,jAppDataID);
  //const char* c_jstrAppData = env->GetStringUTFChars(jstrAppData, JNI_FALSE);
  //jint appDataLength = env->GetIntField(jVideoMixLayout,jAppDataLengthID);

  agora::linuxsdk::VideoMixingLayout layout;
 
  layout.canvasWidth = int(canvasWidth);
  layout.canvasHeight = int(canvasHeight);
  layout.backgroundColor = c_backgroundColor;
  layout.regionCount = int(regionCount);
  layout.keepLastFrame = (int(keepLastFrame) == 1) ? true : false;

  //layout.appData = c_jstrAppData;
  //layout.appDataLength = int(appDataLength);
  //regions begin
  
  if(0<regionCount)
  {
   jobjectArray jobRegions =  (jobjectArray)env->GetObjectField(jVideoMixLayout, jRegionsID);
   if(!jobRegions) {
    cout<<"[ERROR]regionCount is bigger than zero,but cannot find Regions in jVideoMixLayout!"<<endl;
    return JNI_FALSE;
   }
   jint arrLen = env->GetArrayLength(jobRegions);
   if(arrLen != regionCount){
    cout<<"regionCount is not equal with arrLen"<<endl;
    return JNI_FALSE;//return ??
   }  
    agora::linuxsdk::VideoMixingLayout::Region* regionList = new agora::linuxsdk::VideoMixingLayout::Region[arrLen];
    for(int i=0; i<arrLen;++i){
      jobject region = env->GetObjectArrayElement(jobRegions, i);
      if(!region) {
          cout<<"[WARN]connot get region"<<endl;
          continue;
      }
      jclass jcRegion =env->GetObjectClass(region);
      if(!jcRegion){
        cout<<"cannot get jclass Region!"<<endl;
        break;
      }
      jfieldID jfidUid = env->GetFieldID(jcRegion, "uid", LONG_SIGNATURE);
      if(!jfidUid){
        cout<<"connot get region uid"<<endl;
        continue;
      }
      //uid
      jlong uidValue = env->GetLongField(region,jfidUid);
      //C++ uid is uint32
      regionList[i].uid = static_cast<uint32_t>(uidValue);
      //x
      jfieldID jxID = env->GetFieldID(jcRegion, "x", DOUBLE_SIGNATURE);
      if(!jxID){
        cout<<"connot get region x,uid:"<<uint32_t(uidValue)<<endl;
        continue;
      }
      jdouble jx = env->GetDoubleField(region,jxID);
      regionList[i].x = static_cast<float>(jx);

      //y
      jfieldID jyID = env->GetFieldID(jcRegion, "y", DOUBLE_SIGNATURE);
      if(!jyID){
        cout<<"connot get region y,uid:"<<int(uidValue)<<endl;
        continue;
      }
      jdouble jy = env->GetDoubleField(region,jyID);
      regionList[i].y = static_cast<float>(jy);

      //width
      jfieldID jwidthID = env->GetFieldID(jcRegion, "width", DOUBLE_SIGNATURE);
      if(!jwidthID){
        cout<<"connot get region width, uid:"<<int(uidValue)<<endl;
        continue;
      }
      jdouble jwidth = env->GetDoubleField(region,jwidthID);
      regionList[i].width = static_cast<float>(jwidth);

      //height
      jfieldID jheightID = env->GetFieldID(jcRegion, "height", DOUBLE_SIGNATURE);
      if(!jheightID){
        cout<<"connot get region height, uid:"<<int(uidValue)<<endl;
        continue;
      }
      jdouble jheight = env->GetDoubleField(region,jheightID);
      regionList[i].height = static_cast<float>(jheight);
      cout<<"user id:"<<static_cast<uint32_t>(uidValue)<<",x:"<<static_cast<float>(jx)<<",y:"<<static_cast<float>(jy)<<",width:"<<static_cast<float>(jwidth)<<",height:"<<static_cast<float>(jheight)<<",alpha:"<<static_cast<double>(i + 1)<<endl;

      jfieldID jrendermodeID = env->GetFieldID(jcRegion, "renderMode", INT_SIGNATURE);
      int rendermode = 0;
      if(!jrendermodeID) {
          cout<<"Cannot get region render mode."<<endl;
      } else {
          jint jrendermode = env->GetIntField(region, jrendermodeID);
          rendermode = jrendermode;
      }

      //alpha
      regionList[i].alpha = static_cast<double>(i + 1);
      //renderMode
      regionList[i].renderMode = rendermode;
    }  
    layout.regions = regionList;
  }
  else
    layout.regions = NULL;

  jobjectArray literals = (jobjectArray)env->GetObjectField(jVideoMixLayout, jLiteralWmID);
  jobjectArray timestamps = (jobjectArray)env->GetObjectField(jVideoMixLayout, jTimestampWmID);
  jobjectArray images = (jobjectArray)env->GetObjectField(jVideoMixLayout, jImageWmID);

  int literal_num = 0, timestamp_num = 0, image_num = 0;
  if (literals) {
    literal_num = (int)env->GetArrayLength(literals);
  }

  if (timestamps) {
    timestamp_num = (int) env->GetArrayLength(timestamps);
  }

  if (images) {
    image_num = (int) env->GetArrayLength(images);
  }
  
  cout << "literal watermark : " << literal_num <<", timestamp watermark : " << timestamp_num <<
    ", image watermark : " << image_num << endl;
  int total_num = literal_num + timestamp_num + image_num;
  if (total_num > 0) {
    agora::linuxsdk::WatermarkConfig *configs = new agora::linuxsdk::WatermarkConfig[total_num];
    for (int i = 0; i < literal_num; ++i) {
      configs[i].type = agora::linuxsdk::WATERMARK_TYPE_LITERA;
      jobject literal = env->GetObjectArrayElement(literals, i);
      if (!literal)
        continue;
      configs[i].config.litera.wm_litera = getStringField(env, literal, "wmLitera");
      configs[i].config.litera.font_file_path = getStringField(env, literal, "fontFilePath");
      configs[i].config.litera.font_size = getIntField(env, literal, "fontSize");
      configs[i].config.litera.offset_x = getIntField(env, literal, "offsetX");
      configs[i].config.litera.offset_y = getIntField(env, literal, "offsetY");
      configs[i].config.litera.wm_width = getIntField(env, literal, "wmWidth");
      configs[i].config.litera.wm_height = getIntField(env, literal, "wmHeight");
    }

    for (int i = 0; i < timestamp_num; ++i) {
      configs[literal_num + i].type = agora::linuxsdk::WATERMARK_TYPE_TIMESTAMP;
      jobject ts = env->GetObjectArrayElement(timestamps, i);
      if (!ts)
        continue;
      configs[literal_num + i].config.timestamp.font_size = getIntField(env, ts, "fontSize");
      configs[literal_num + i].config.timestamp.offset_x = getIntField(env, ts, "offsetX");
      configs[literal_num + i].config.timestamp.offset_y = getIntField(env, ts, "offsetY");
      configs[literal_num + i].config.timestamp.wm_width = getIntField(env, ts, "wmWidth");
      configs[literal_num + i].config.timestamp.wm_height = getIntField(env, ts, "wmHeight");
    }

    for (int i = 0; i < image_num; ++i) {

      configs[literal_num + timestamp_num + i].type = agora::linuxsdk::WATERMARK_TYPE_IMAGE;
      jobject img = env->GetObjectArrayElement(images, i);
      if (!img)
        continue;
      configs[literal_num + timestamp_num + i].config.image.image_path = getStringField(env, img, "imagePath");
      configs[literal_num + timestamp_num + i].config.image.offset_x = getIntField(env, img, "offsetX");
      configs[literal_num + timestamp_num + i].config.image.offset_y = getIntField(env, img, "offsetY");
      configs[literal_num + timestamp_num + i].config.image.wm_width = getIntField(env, img, "wmWidth");
      configs[literal_num + timestamp_num + i].config.image.wm_height = getIntField(env, img, "wmHeight");
    }

    layout.wm_num = total_num;
    layout.wm_configs = configs;
  }

  //regions end
  nativeHandle->setVideoMixingLayout(layout);

  {
    if (layout.regionCount) {
      delete [] layout.regions;
    }

    if (layout.wm_num) {
      for (int i = 0; i < layout.wm_num; ++i) {
        if (layout.wm_configs[i].type == agora::linuxsdk::WATERMARK_TYPE_LITERA) {
          if (layout.wm_configs[i].config.litera.wm_litera) {
            delete [] layout.wm_configs[i].config.litera.wm_litera;
          }
        } else if (layout.wm_configs[i].type == agora::linuxsdk::WATERMARK_TYPE_IMAGE) {
          if (layout.wm_configs[i].config.image.image_path) {
            delete [] layout.wm_configs[i].config.image.image_path;
          }
        }
      }
      delete [] layout.wm_configs;
    }
  }
  return jint(0);
}

/*
 * Class:     io_agora_record_AgoraJavaRecording
 * Method:    getProperties
 * Signature: (J)Lio/agora/common/RecordingEngineProperties;
 */
JNIEXPORT jobject JNICALL Java_io_agora_recording_RecordingSDK_getProperties(JNIEnv * env, jobject, jlong nativeObjectRef){
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  if(!nativeHandle){
    return JNI_FALSE;
  }
  jclass jc = env->FindClass(CN_REP);
  if(!jc){
    cout<<"cannot get jclass RecordingEngineProperties!"<<endl;    
    return JNI_FALSE;
  }
  jmethodID initMid = env->GetMethodID(jc, SG_MTD_INIT, VOID_PARA_VOID_RETURN);
  if(!initMid){
    cout<<"cannot get RecordingEngineProperties init!"<<endl;
    return JNI_FALSE;
  }
  jobject job = NULL;
  job = env->NewObject(jc, initMid);
  if(!job){
    cout<<"new object failed!"<<endl;
    return JNI_FALSE;
  }
  jfieldID storageFid = env->GetFieldID(jc, "storageDir", STRING_SIGNATURE);
  if(!storageFid){
    cout<<"storageDir fid not found!"<<endl;
    return JNI_FALSE;
  }
  char* storageDir = nativeHandle->getRecorderProperties()->storageDir;
  jstring jstrStorageDir = env->NewStringUTF(storageDir);
  env->SetObjectField(job, storageFid, jstrStorageDir);
  env->DeleteLocalRef(jc);
  return job;
}

/*
 * Class:     AgoraJavaRecording
 * Method:    setUserBackground
 * Signature: (JILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_io_agora_recording_RecordingSDK_setUserBackground(JNIEnv * env, jobject job, jlong nativeObjectRef, jint juid, jstring jpath){
    jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
    if(nativeHandle) {
        int uid = (int)juid;
        std::string path;
        const char* c_path = env->GetStringUTFChars(jpath, JNI_FALSE);
        path = c_path;
        env->ReleaseStringUTFChars(jpath, c_path);
        return nativeHandle->setUserBackground(uid, path.c_str());
    }
    return -1;
}

/*
 * Class:     AgoraJavaRecording
 * Method:    setLogLevel
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_io_agora_recording_RecordingSDK_setLogLevel
  (JNIEnv *env, jobject job, jlong nativeObjectRef, jint level) {
      jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
      if(nativeHandle) {
          int logLevel = (int) level;
          nativeHandle->setLogLevel((agora::linuxsdk::agora_log_level)logLevel);
      }
  }

/*
 * Class:     io_agora_recording_RecordingSDK
 * Method:    updateSubscribeVideoUids
 * Signature: (J[I)I
 */
JNIEXPORT jint JNICALL Java_io_agora_recording_RecordingSDK_updateSubscribeVideoUids
  (JNIEnv *env, jobject job, jlong nativeObjectRef, jintArray juids) {
    jniproxy::AgoraJniProxySdk *nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
    if (nativeHandle) {
      std::vector<uint32_t> videoUids;
      int len = env->GetArrayLength(juids);
      jint* uids = env->GetIntArrayElements(juids, NULL);
      for (int i = 0; i < len; i++) {
        videoUids.push_back(uids[i]);
      }
      nativeHandle->updateSubscribeVideoUids(videoUids.data(), videoUids.size());
    }
  }

/*
 * Class:     io_agora_recording_RecordingSDK
 * Method:    updateSubscribeAudioUids
 * Signature: (J[I)I
 */
JNIEXPORT jint JNICALL Java_io_agora_recording_RecordingSDK_updateSubscribeAudioUids
  (JNIEnv *env, jobject job, jlong nativeObjectRef, jintArray juids) {
    jniproxy::AgoraJniProxySdk *nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
    if (nativeHandle) {
      std::vector<uint32_t> audioUids;
      int len = env->GetArrayLength(juids);
      jint* uids = env->GetIntArrayElements(juids, NULL);
      for (int i = 0; i < len; i++) {
        audioUids.push_back(uids[i]);
      }
      nativeHandle->updateSubscribeAudioUids(audioUids.data(), audioUids.size());
    }
  }



/*
 * Class:     AgoraJavaRecording
 * Method:    startService
 * Signature: (J)V
 */
 JNIEXPORT jint JNICALL Java_io_agora_recording_RecordingSDK_startService(JNIEnv * env, jobject job, jlong nativeObjectRef){
   jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
   if(nativeHandle){
     return nativeHandle->startService();
   }
   return -1;
 }
/*
 * Class:     AgoraJavaRecording
 * Method:    stopService
 * Signature: (J)V
 */
JNIEXPORT jint JNICALL Java_io_agora_recording_RecordingSDK_stopService(JNIEnv * env, jobject job, jlong nativeObjectRef){
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  if(nativeHandle){
    return nativeHandle->stopService();
  }
  return -1;
}

JNIEXPORT jstring JNICALL Java_io_agora_recording_RecordingSDK_getUserAccountByUid (JNIEnv *env, jobject job, jlong nativeObjectRef, jint userid) {
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  if(nativeHandle){
    char userAccount[256] = {0};
    uint32_t num = nativeHandle->getUserAccountByUid((uint32_t)userid, userAccount, 256);
    if (num > 0) {
      jstring jua = env->NewStringUTF(userAccount);
      return jua;
    }
  }
  jstring jua = env->NewStringUTF("");
  return jua;
}

JNIEXPORT jlong JNICALL Java_io_agora_recording_RecordingSDK_getUidByUserAccount
  (JNIEnv *env, jobject job, jlong nativeObjectRef, jstring userAccount) {
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  std::string ua;
  const char * c_userAccount = env->GetStringUTFChars(userAccount, JNI_FALSE);
  ua = c_userAccount;
  if (nativeHandle) {
    return nativeHandle->getUidByUserAccount(ua.c_str());
  }
  return 0;
}


void AgoraJniProxySdk::stopJavaProc(JNIEnv* env) {
  //CM_LOG_DIR(m_logdir.c_str(), WARN,"AgoraJniProxySdk stopJavaProc");
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  jmethodID jStopCB =  env->GetMethodID(mJavaAgoraJavaRecordingClass,"stopCallBack","()V");
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, jStopCB);
  CHECK_EXCEPTION(env, "");
}

void AgoraJniProxySdk::setJavaRecordingPath(JNIEnv* env, std::string& storeDir){
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  
  jmethodID jRecordingPathCB =  env->GetMethodID(mJavaAgoraJavaRecordingClass,"recordingPathCallBack","(Ljava/lang/String;)V");
  jstring jstrRecordingDir = env->NewStringUTF(storeDir.c_str());

  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, jRecordingPathCB, jstrRecordingDir);
  CHECK_EXCEPTION(env, "");
}

jboolean createChannel(JNIEnv *env, jobject thisObj, jstring jni_appid, jstring jni_channelKey, jstring jni_channelName, jint jni_uid, const char* userAccount, jobject jni_recordingConfig, jint jlogLevel, bool useUa);
/*
 * Class:     io_agora_record_AgoraJavaRecording
 * Method:    createChannel
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILio/agora/common/RecordingConfig;II)Z
 */

JNIEXPORT jboolean JNICALL Java_io_agora_recording_RecordingSDK_createChannel(JNIEnv * env, jobject thisObj, jstring jni_appid, jstring jni_channelKey, 
      jstring jni_channelName, jint jni_uid, jobject jni_recordingConfig, jint jlogLevel)
{
  return createChannel(env, thisObj, jni_appid, jni_channelKey, jni_channelName, jni_uid, NULL, jni_recordingConfig, jlogLevel, false);
}

JNIEXPORT jboolean JNICALL Java_io_agora_recording_RecordingSDK_createChannelWithUserAccount
  (JNIEnv *env, jobject thisObj, jstring jni_appid, jstring jni_channelKey, jstring jni_channelName, jstring juserAccount, jobject jni_recordingConfig, jint jlogLevel) {

  std::string userAccount;
  const char* c_userAccount = env->GetStringUTFChars(juserAccount, JNI_FALSE);
  userAccount = c_userAccount;
  env->ReleaseStringUTFChars(juserAccount, c_userAccount);
  if(userAccount.empty()){
    cout<<"get userAccount is NULL"<<endl;
    return JNI_FALSE;
  }

  return createChannel(env, thisObj, jni_appid, jni_channelKey, jni_channelName, 0, userAccount.c_str(), jni_recordingConfig, jlogLevel, true);
}


jboolean createChannel(JNIEnv *env, jobject thisObj, jstring jni_appid, jstring jni_channelKey, jstring jni_channelName, jint jni_uid,
    const char* userAccount, jobject jni_recordingConfig, jint jlogLevel, bool useUa) {
  jniproxy::AgoraJniProxySdk jniRecorder;
  uint32_t uid = 0;
  string appId;
  string channelKey;
  string channelName;
  uint32_t channelProfile = 0;

  string decryptionMode;
  string secret;
  string mixResolution("360,640,15,500");

  int idleLimitSec=5*60;//300s

  string applitePath;
  string appliteLogPath;
  string recordFileRootDir = "";
  string cfgFilePath = "";
  int proxyType = 1;
  string proxyServer;

  int lowUdpPort = 0;//40000;
  int highUdpPort = 0;//40004;

  bool isAudioOnly=0;
  bool isVideoOnly=0;
  bool isMixingEnabled=0;
  bool autoSubscribe = true;
  bool enableCloudProxy = false;
  bool enableIntraRequest = true;
  bool enableH265Support = false;
  string subscribeVideoUids;
  string subscribeAudioUids;
  uint32_t mixedVideoAudio = agora::linuxsdk::MIXED_AV_DEFAULT;

  uint32_t getAudioFrame = agora::linuxsdk::AUDIO_FORMAT_DEFAULT_TYPE;
  uint32_t getVideoFrame = agora::linuxsdk::VIDEO_FORMAT_DEFAULT_TYPE;
  uint32_t streamType = agora::linuxsdk::REMOTE_VIDEO_STREAM_HIGH;
  int captureInterval = 5;
  int audioIndicationInterval = 0;
  int triggerMode = 0;
  int audioProfile = 0;
  string defaultVideoBgPath;
  string defaultUserBgPath;
  int lang = 1;

  signal(SIGPIPE, SIG_IGN);

  //const char* appId = NULL;
  const char* c_appId = env->GetStringUTFChars(jni_appid, JNI_FALSE);
  appId = c_appId;
  env->ReleaseStringUTFChars(jni_appid, c_appId);
  if(appId.empty()){
    cout<<"get appId is NULL"<<endl;
    return JNI_FALSE;
  }
  
  //const char* channelKey = NULL;
  const char* c_channelKey = env->GetStringUTFChars(jni_channelKey, JNI_FALSE);
  channelKey = c_channelKey;
  env->ReleaseStringUTFChars(jni_channelKey, c_channelKey);
  /*
  if(channelKey.empty()){
     cout<<"get channel key is NULL"<<endl;
  }
  */
  //const char* channelName = NULL; 
  const char* c_channelName = env->GetStringUTFChars(jni_channelName, JNI_FALSE);
  channelName = c_channelName;
  env->ReleaseStringUTFChars(jni_channelName, c_channelName);
  if(channelName.empty()){
    cout<<"channel name is empty!"<<endl;
    return JNI_FALSE;
  }

  if (!useUa) {
    uid = (int)jni_uid;
    if(uid < 0){
      cout<<"jni uid is smaller than 0, set 0!"<<endl;
      uid = 0;
    }
  } else {
    if (strlen(userAccount) > 256) {
      cout << "User account too lang" << endl;
      return JNI_FALSE;
    }
  }
  jclass jRecordingJavaConfig =env->GetObjectClass(jni_recordingConfig); 
  if(!jRecordingJavaConfig){
    cout<<"jni_recordingConfig is NULL"<<endl;
    return JNI_FALSE;
  }
  //get parameters field ID
  jfieldID idleLimitSecFieldID = env->GetFieldID(jRecordingJavaConfig, "idleLimitSec", INT_SIGNATURE);
  jfieldID channelProfileFieldID = env->GetFieldID(jRecordingJavaConfig, "channelProfile", CHANNEL_PROFILE_SIGNATURE);
  jfieldID isVideoOnlyFid = env->GetFieldID(jRecordingJavaConfig, "isVideoOnly", BOOL_SIGNATURE);
  jfieldID isAudioOnlyFid = env->GetFieldID(jRecordingJavaConfig, "isAudioOnly", BOOL_SIGNATURE);
  jfieldID isMixingEnabledFid = env->GetFieldID(jRecordingJavaConfig, "isMixingEnabled", BOOL_SIGNATURE);
		
  jfieldID mixResolutionFid = env->GetFieldID(jRecordingJavaConfig, "mixResolution", STRING_SIGNATURE);
  jfieldID mixedVideoAudioFid = env->GetFieldID(jRecordingJavaConfig, "mixedVideoAudio", MIXED_AV_CODEC_TYPE_SIGNATURE);
  jfieldID appliteDirFieldID = env->GetFieldID(jRecordingJavaConfig, "appliteDir", STRING_SIGNATURE);
  jfieldID recordFileRootDirFid = env->GetFieldID(jRecordingJavaConfig, "recordFileRootDir", STRING_SIGNATURE);
  jfieldID cfgFilePathFid = env->GetFieldID(jRecordingJavaConfig, "cfgFilePath", STRING_SIGNATURE);
  jfieldID secretFid = env->GetFieldID(jRecordingJavaConfig, "secret", STRING_SIGNATURE);
  jfieldID decryptionModeFid = env->GetFieldID(jRecordingJavaConfig, "decryptionMode", STRING_SIGNATURE);
  jfieldID lowUdpPortFid = env->GetFieldID(jRecordingJavaConfig, "lowUdpPort", INT_SIGNATURE);
  jfieldID highUdpPortFid = env->GetFieldID(jRecordingJavaConfig, "highUdpPort", INT_SIGNATURE);
  jfieldID captureIntervalFid = env->GetFieldID(jRecordingJavaConfig, "captureInterval", INT_SIGNATURE);

  jfieldID audioIndicationIntervalFid = env->GetFieldID(jRecordingJavaConfig, "audioIndicationInterval", INT_SIGNATURE);
  jfieldID streamTypeFieldID = env->GetFieldID(jRecordingJavaConfig, "streamType", REMOTE_VIDEO_STREAM_SIGNATURE);
  jfieldID decodeAudioFieldID = env->GetFieldID(jRecordingJavaConfig, "decodeAudio", AUDIO_FORMAT_TYPE_SIGNATURE);
  jfieldID decodeVideoFieldID = env->GetFieldID(jRecordingJavaConfig, "decodeVideo", VIDEO_FORMAT_TYPE_SIGNATURE);
  jfieldID triggerModeFid = env->GetFieldID(jRecordingJavaConfig, "triggerMode", INT_SIGNATURE);
  jfieldID proxyTypeFid = env->GetFieldID(jRecordingJavaConfig, "proxyType", INT_SIGNATURE);
  jfieldID proxyServerFid = env->GetFieldID(jRecordingJavaConfig, "proxyServer", STRING_SIGNATURE);
  jfieldID audioProfileFid = env->GetFieldID(jRecordingJavaConfig, "audioProfile", INT_SIGNATURE);
  jfieldID defaultVideoBgFid = env->GetFieldID(jRecordingJavaConfig, "defaultVideoBgPath", STRING_SIGNATURE);
  jfieldID defaultUserBgFid = env->GetFieldID(jRecordingJavaConfig, "defaultUserBgPath", STRING_SIGNATURE);
  jfieldID autoSubscribeFid = env->GetFieldID(jRecordingJavaConfig, "autoSubscribe", BOOL_SIGNATURE);
  jfieldID subscribedVideoUidsFid = env->GetFieldID(jRecordingJavaConfig, "subscribeVideoUids", STRING_SIGNATURE);
  jfieldID subscribedAudioUidsFid = env->GetFieldID(jRecordingJavaConfig, "subscribeAudioUids", STRING_SIGNATURE);
  jfieldID enableCloudProxyFid = env->GetFieldID(jRecordingJavaConfig, "enableCloudProxy", BOOL_SIGNATURE);
  jfieldID enableIntraRequestFid = env->GetFieldID(jRecordingJavaConfig, "enableIntraRequest", BOOL_SIGNATURE);
  jfieldID enableH265SupportFid = env->GetFieldID(jRecordingJavaConfig, "enableH265Support", BOOL_SIGNATURE);

  if (!idleLimitSecFieldID || !appliteDirFieldID || !channelProfileFieldID 
						|| !streamTypeFieldID || !decodeAudioFieldID || !decodeVideoFieldID || !isMixingEnabledFid) { 
            cout<<"get fieldID failed!";return JNI_FALSE;}
  //idle
  idleLimitSec = (int)env->GetIntField(jni_recordingConfig, idleLimitSecFieldID); 
  //appliteDir
  jstring appliteDir = (jstring)env->GetObjectField(jni_recordingConfig, appliteDirFieldID);
  const char * c_appliteDir = env->GetStringUTFChars(appliteDir, JNI_FALSE);
  applitePath = c_appliteDir;
  env->ReleaseStringUTFChars(appliteDir,c_appliteDir);
  env->DeleteLocalRef(appliteDir);
  //CHANNEL_PROFILE_TYPE
  jobject channelProfileObject = (env)->GetObjectField(jni_recordingConfig, channelProfileFieldID);
  //assert(channelProfileObject);
  jclass enumClass = env->GetObjectClass(channelProfileObject);
  if(!enumClass) {  
    cout<<"enumClass is null";
    return JNI_FALSE;
  }
  jmethodID getValue = env->GetMethodID(enumClass, "getValue", EMPTY_PARA_INT_RETURN);
  if (!getValue) {
    cout<<"method not found";
    return JNI_FALSE; /* method not found */
  }
  jint channelProfileValue = env->CallIntMethod(channelProfileObject, getValue);
  CHECK_EXCEPTION(env, "");
  channelProfile=int(channelProfileValue);
  //streamType
  jobject streamTypeObject = (env)->GetObjectField(jni_recordingConfig, streamTypeFieldID);
  jclass streamTypeClass = env->GetObjectClass(streamTypeObject);
  if(!streamTypeObject){cout<<"streamTypeEnum is NULL"; return JNI_FALSE;}
  jmethodID getValue2 = env->GetMethodID(streamTypeClass, "getValue", EMPTY_PARA_INT_RETURN);
  jint streamTypeValue = env->CallIntMethod(streamTypeObject, getValue2);
  CHECK_EXCEPTION(env, "");
  streamType = int(streamTypeValue);
  //decodeAudio
  jobject jobDecodeAudio = (env)->GetObjectField(jni_recordingConfig, decodeAudioFieldID);
  jclass jcdecodeAudio = env->GetObjectClass(jobDecodeAudio);
  if(!jcdecodeAudio) {
    cout<<"jcdecodeAudio is null";
  }
  jmethodID jmidGetValue = env->GetMethodID(jcdecodeAudio, "getValue", EMPTY_PARA_INT_RETURN);
  if (!jmidGetValue) {
    cout<<"jmidGetValue not found";
    return JNI_FALSE; /* method not found */
  }
  jint decodeAudioValue = env->CallIntMethod(jobDecodeAudio, jmidGetValue);
  CHECK_EXCEPTION(env, "");
  getAudioFrame = int(decodeAudioValue);
  env->DeleteLocalRef(jobDecodeAudio);
  env->DeleteLocalRef(jcdecodeAudio);
  //decodeVideo
  jobject jobDecodeVideo = (env)->GetObjectField(jni_recordingConfig, decodeVideoFieldID);
  jclass jcdecodeVideo = env->GetObjectClass(jobDecodeVideo);
  if(!jcdecodeVideo) {
    cout<<"jcdecodeVideo is null";
  }
  jmidGetValue = env->GetMethodID(jcdecodeVideo, "getValue", EMPTY_PARA_INT_RETURN);
  if (!jmidGetValue) {
    cout<<"jmidGetValue not found";
    return JNI_FALSE; /* method not found */
  }
  jint decodeVideoValue = env->CallIntMethod(jobDecodeVideo, jmidGetValue);
  CHECK_EXCEPTION(env, "");
  getVideoFrame = int(decodeVideoValue);
  //isMixingEnabled
  jboolean isMixingEnabledValue = env->GetBooleanField(jni_recordingConfig, isMixingEnabledFid);
  isMixingEnabled = bool(isMixingEnabledValue);
  //isVideoOnly
  jboolean jisVideoOnly = (int)env->GetBooleanField(jni_recordingConfig, isVideoOnlyFid); 
  isVideoOnly = bool(jisVideoOnly);
  //isAudioOnly
  jboolean jisAudioOnly = (int)env->GetBooleanField(jni_recordingConfig, isAudioOnlyFid); 
  isAudioOnly = bool(jisAudioOnly);
  //mixResolution
  jstring jmixResolution = (jstring)env->GetObjectField(jni_recordingConfig, mixResolutionFid);
  const char * c_mixResolution = env->GetStringUTFChars(jmixResolution, JNI_FALSE);
  mixResolution = c_mixResolution;
  env->ReleaseStringUTFChars(jmixResolution, c_mixResolution);
  env->DeleteLocalRef(jmixResolution);

  //mixedVideoAudio
  
  jobject jobMixedAV = (env)->GetObjectField(jni_recordingConfig, mixedVideoAudioFid);
  jclass jcMixedAV = env->GetObjectClass(jobMixedAV);
  if(!jcMixedAV) {
    cout<<"jcMixedAV is null";
  }
  jmidGetValue = env->GetMethodID(jcMixedAV, "getValue", EMPTY_PARA_INT_RETURN);
  if (!jmidGetValue) {
    cout<<"mixed av jmidGetValue not found";
    return JNI_FALSE; /* method not found */
  }
  jint mixedAVValue = env->CallIntMethod(jobMixedAV, jmidGetValue);
  CHECK_EXCEPTION(env, "");
  mixedVideoAudio = int(mixedAVValue);

  //recordFileRootDir
  jstring jrecordFileRootDir = (jstring)env->GetObjectField(jni_recordingConfig, recordFileRootDirFid);
  const char * c_recordFileRootDir = env->GetStringUTFChars(jrecordFileRootDir, JNI_FALSE);
  recordFileRootDir = c_recordFileRootDir;
  env->ReleaseStringUTFChars(jrecordFileRootDir, c_recordFileRootDir);
  env->DeleteLocalRef(jrecordFileRootDir);

  //cfgFilePath
  jstring jcfgFilePath = (jstring)env->GetObjectField(jni_recordingConfig, cfgFilePathFid);
  const char * c_cfgFilePath = env->GetStringUTFChars(jcfgFilePath, JNI_FALSE);
  cfgFilePath = c_cfgFilePath;
  env->ReleaseStringUTFChars(jcfgFilePath, c_cfgFilePath);
  env->DeleteLocalRef(jcfgFilePath);

  //secret
  jstring jsecret = (jstring)env->GetObjectField(jni_recordingConfig, secretFid);
  const char * c_secret = env->GetStringUTFChars(jsecret, JNI_FALSE);
  secret = c_secret;
  env->ReleaseStringUTFChars(jsecret, c_secret);
  env->DeleteLocalRef(jsecret);

  //proxyType
  proxyType = (int)env->GetIntField(jni_recordingConfig, proxyTypeFid); 

  //proxyServer
  jstring jproxy = (jstring)env->GetObjectField(jni_recordingConfig, proxyServerFid);
  const char * c_proxy = env->GetStringUTFChars(jproxy, JNI_FALSE);
  proxyServer = c_proxy;
  env->ReleaseStringUTFChars(jproxy, c_proxy);
  env->DeleteLocalRef(jproxy);

  //decryptionMode
  jstring jdecryptionMode = (jstring)env->GetObjectField(jni_recordingConfig, decryptionModeFid);
  const char * c_decryptionMode = env->GetStringUTFChars(jdecryptionMode, JNI_FALSE);
  decryptionMode = c_decryptionMode;
  env->ReleaseStringUTFChars(jdecryptionMode, c_decryptionMode);
  env->DeleteLocalRef(jdecryptionMode);
  //lowUdpPort
  lowUdpPort = (int)env->GetIntField(jni_recordingConfig, lowUdpPortFid); 
  //highUdpPort
  highUdpPort = (int)env->GetIntField(jni_recordingConfig, highUdpPortFid); 
  //captureInterval
  captureInterval = (int)env->GetIntField(jni_recordingConfig, captureIntervalFid); 
  //audioIndicationInterval
  audioIndicationInterval = (int)env->GetIntField(jni_recordingConfig, audioIndicationIntervalFid); 
  //triggerMode
  triggerMode = (int)env->GetIntField(jni_recordingConfig, triggerModeFid); 
  audioProfile = (int)env->GetIntField(jni_recordingConfig, audioProfileFid);
  jstring jdefaultVideoBg = (jstring)env->GetObjectField(jni_recordingConfig, defaultVideoBgFid);
  const char* c_defaultVideoBg = env->GetStringUTFChars(jdefaultVideoBg, JNI_FALSE);
  defaultVideoBgPath = c_defaultVideoBg;
  env->ReleaseStringUTFChars(jdefaultVideoBg, c_defaultVideoBg);
  env->DeleteLocalRef(jdefaultVideoBg);
  jstring jdefaultUserBg = (jstring)env->GetObjectField(jni_recordingConfig, defaultUserBgFid);
  const char* c_defaultUserBg = env->GetStringUTFChars(jdefaultUserBg, JNI_FALSE);
  defaultUserBgPath = c_defaultUserBg;
  env->ReleaseStringUTFChars(jdefaultUserBg, c_defaultUserBg);
  env->DeleteLocalRef(jdefaultUserBg);
  
  jboolean jAutoSubscribe = env->GetBooleanField(jni_recordingConfig, autoSubscribeFid);
  autoSubscribe = bool(jAutoSubscribe);
  jboolean jEnableCloudProxy = env->GetBooleanField(jni_recordingConfig, enableCloudProxyFid);
  enableCloudProxy = bool(jEnableCloudProxy);

  jboolean jEnableIntraRequest = env->GetBooleanField(jni_recordingConfig, enableIntraRequestFid);
  enableIntraRequest = bool(jEnableIntraRequest);

  jboolean jEnableH265Support = env->GetBooleanField(jni_recordingConfig, enableH265SupportFid);
  enableH265Support = bool(jEnableH265Support);

  jstring jSubscribeVideoUids = (jstring)env->GetObjectField(jni_recordingConfig, subscribedVideoUidsFid);
  const char* c_subscribeVideoUids = env->GetStringUTFChars(jSubscribeVideoUids, JNI_FALSE);
  subscribeVideoUids = c_subscribeVideoUids;
  env->ReleaseStringUTFChars(jSubscribeVideoUids, c_subscribeVideoUids);
  env->DeleteLocalRef(jSubscribeVideoUids);

  jstring jSubscribeAudioUids = (jstring)env->GetObjectField(jni_recordingConfig, subscribedAudioUidsFid);
  const char* c_subscribeAudioUids = env->GetStringUTFChars(jSubscribeAudioUids, JNI_FALSE);
  subscribeAudioUids = c_subscribeAudioUids;
  env->ReleaseStringUTFChars(jSubscribeAudioUids, c_subscribeAudioUids);
  env->DeleteLocalRef(jSubscribeAudioUids);
  //paser parameters end
  env->DeleteLocalRef(jni_recordingConfig);

  agora::recording::RecordingConfig config;
  //important! Get a reference to this object's class

  jclass thisJcInstance = NULL;
  thisJcInstance = env->GetObjectClass(thisObj);
  if(!thisJcInstance) {
    cout<<"Jni cannot get java instance, error!!!";
    return JNI_FALSE;
  }
  jniRecorder.setJcAgoraJavaRecording((jclass)env->NewGlobalRef(thisJcInstance));
  jniRecorder.setJobAgoraJavaRecording(env->NewGlobalRef(thisObj));
  jniRecorder.initialize();

  config.idleLimitSec = idleLimitSec;
  config.channelProfile = static_cast<agora::linuxsdk::CHANNEL_PROFILE_TYPE>(channelProfile);

  config.isVideoOnly = isVideoOnly;
  config.isAudioOnly = isAudioOnly;
  config.isMixingEnabled = isMixingEnabled;
  config.mixResolution = (isMixingEnabled && !isAudioOnly)? const_cast<char*>(mixResolution.c_str()):NULL;
  config.mixedVideoAudio = static_cast<agora::linuxsdk::MIXED_AV_CODEC_TYPE>(mixedVideoAudio);

  config.appliteDir = const_cast<char*>(applitePath.c_str());	
  config.recordFileRootDir = const_cast<char*>(recordFileRootDir.c_str());
  config.cfgFilePath = const_cast<char*>(cfgFilePath.c_str());

  config.secret = secret.empty()? NULL:const_cast<char*>(secret.c_str());
  config.decryptionMode = decryptionMode.empty()? NULL:const_cast<char*>(decryptionMode.c_str());
  config.defaultVideoBg = defaultVideoBgPath.empty()?NULL:const_cast<char*>(defaultVideoBgPath.c_str());
  config.defaultUserBg = defaultUserBgPath.empty() ? NULL : const_cast<char*>(defaultUserBgPath.c_str());

  config.lowUdpPort = lowUdpPort;
  config.highUdpPort = highUdpPort;
  config.captureInterval = captureInterval;
  config.audioIndicationInterval = audioIndicationInterval;
  config.decodeAudio = static_cast<agora::linuxsdk::AUDIO_FORMAT_TYPE>(getAudioFrame);
  config.decodeVideo = static_cast<agora::linuxsdk::VIDEO_FORMAT_TYPE>(getVideoFrame);
  config.streamType = static_cast<agora::linuxsdk::REMOTE_VIDEO_STREAM_TYPE>(streamType);
  config.triggerMode = static_cast<agora::linuxsdk::TRIGGER_MODE_TYPE>(triggerMode);
  config.audioProfile = static_cast<agora::linuxsdk::AUDIO_PROFILE_TYPE>(audioProfile);
  config.lang = static_cast<agora::linuxsdk::LANGUAGE_TYPE>(lang);
  config.proxyType = proxyType;
  config.proxyServer = proxyServer.empty()? NULL:const_cast<char*>(proxyServer.c_str());
  config.enableCloudProxy = enableCloudProxy;
  config.autoSubscribe = autoSubscribe;
  config.subscribeAudioUids = subscribeAudioUids.empty() ? NULL : subscribeAudioUids.c_str();
  config.subscribeVideoUids = subscribeVideoUids.empty() ? NULL : subscribeVideoUids.c_str();
  config.enableIntraRequest = enableIntraRequest;
  config.enableH265Support = enableH265Support;

  /*
  cout<<"appId:"<<appId<<",uid:"<<uid<<",channelKey:"<<channelKey<<",channelName:"<<channelName<<",applitePath:"
    <<applitePath<<",channelProfile:"<<channelProfile<<",getAudioFrame:"
    <<getAudioFrame<<",getVideoFrame:"<<getVideoFrame<<endl<<",idle:"<<idleLimitSec<<",lowUdpPort:"<<lowUdpPort<<",highUdpPort:"<<highUdpPort
    <<",captureInterval:"<<captureInterval<<",mixedVideoAudio:"<<mixedVideoAudio<<",mixResolution:"<<mixResolution<<",isVideoOnly:"<<isVideoOnly
    <<",isAudioOnly:"<<isAudioOnly<<",isMixingEnabled:"<<isMixingEnabled<<",triggerMode:"<<triggerMode<<",proxyServer"<<proxyServer<<", audioProfile: "<<audioProfile<<endl;
  cout<<"audio Profile :"<<audioProfile<<"audioIndicationInterval :"<<audioIndicationInterval<<endl;
  */
  int logLevel = (int)jlogLevel;
  //cout <<"logLevel : "<< logLevel;
  if(logLevel < 2) logLevel = 2;
  if(logLevel > 7) logLevel = 7;
  jniRecorder.setLogLevel((agora::linuxsdk::agora_log_level)logLevel);
  
  if(config.decodeAudio != agora::linuxsdk::AUDIO_FORMAT_DEFAULT_TYPE ||
      config.decodeVideo != agora::linuxsdk::VIDEO_FORMAT_DEFAULT_TYPE) {
    jniRecorder.setRecordingMode(kRecordingModeRawData);
  }


  /**
   * change log_config Facility per your specific purpose like agora::base::LOCAL5_LOG_FCLT
   * Default:USER_LOG_FCLT. 
   * agora::base::log_config::setFacility(agora::base::LOCAL5_LOG_FCLT);  
   */
  if (useUa) {
     if(!jniRecorder.createChannelWithUserAccount(appId, channelKey, channelName, userAccount, config))
    {
      cerr << "Failed to create agora channel: " << channelName <<endl;
      return JNI_FALSE;
    }
   
  } else {
    if(!jniRecorder.createChannel(appId, channelKey, channelName, uid, config))
    {
      cerr << "Failed to create agora channel: " << channelName <<endl;
      return JNI_FALSE;
    }
  }
  //tell java this para pointer
  jlong nativeObjectRef = jlong(&jniRecorder);
  //find java callback function and set this value
  jmethodID jmid = env->GetMethodID(thisJcInstance, "nativeObjectRef", LONG_PARA_VOID_RETURN);
  if(!jmid) {
    cout << "cannot find nativeObjectRef method " <<endl;
    return JNI_FALSE;
  }
  env->CallIntMethod(thisObj, jmid, nativeObjectRef);
  CHECK_EXCEPTION(env, "");
  std::string recordingDir = jniRecorder.getRecorderProperties()->storageDir;
  //cout<<"Recording directory is "<<jniRecorder.getRecorderProperties()->storageDir<<endl;
  jniRecorder.setJavaRecordingPath(env, recordingDir);
  while (!jniRecorder.stopped()) {
    usleep(1*1000*1000); //1s
  }

  jniRecorder.stopJavaProc(env);
  jniRecorder.leaveChannel();
  jniRecorder.release();
  cout<<"jni layer stopped!";
  //cout<<"jni layer stopped! Java_io_agora_record_AgoraJavaRecording_createChannel  end"<<endl;
  return JNI_TRUE;
}

#ifdef __cplusplus
}
#endif
