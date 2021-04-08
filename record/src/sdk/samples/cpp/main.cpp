#include <unistd.h>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <sstream> 
#include <string>
#include <vector>
#include <algorithm>

#include "IAgoraLinuxSdkCommon.h"
#include "IAgoraRecordingEngine.h"

#include "base/atomic.h"
#include "base/opt_parser.h" 
#include "agorasdk/AgoraSdk.h"
using std::string;
using std::cout;
using std::cerr;
using std::endl;

using agora::base::opt_parser;
using agora::linuxsdk::VideoFrame;
using agora::linuxsdk::AudioFrame;

#define CM_LOG(level, fmt, ...)
atomic_bool_t g_bSignalStop;
atomic_bool_t g_bSignalStartService;
atomic_bool_t g_bSignalStopService;

void signal_handler(int signo) {
  (void)signo;

  // cerr << "Signal " << signo << endl;
  g_bSignalStop = true;
}

void start_service(int signo) {
    (void)signo;
    g_bSignalStartService = true;
}

void stop_service(int signo) {
    (void)signo;
    g_bSignalStopService = true;
}

int main(int argc, char * const argv[]) {
  uint32_t uid = 0;
  string appId;
  string channelKey;
  string name;
  uint32_t channelProfile = 0;
  uint32_t audioProfile = 0;

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
  string videoBg = "";
  string userBg = "";
  string subscribeVideoUids = "";
  string subscribeAudioUids = "";
  string userAccount = "";
  string maxResolutionUserAccount = "";

  int lowUdpPort = 0;//40000;
  int highUdpPort = 0;//40004;

  bool isAudioOnly=0;
  bool isVideoOnly=0;
  bool isMixingEnabled=0;
  bool autoSubscribe = true;
  bool enableCloudProxy = false;
  bool enableIntraRequest = true;
  bool enableH265Support = false;
  uint32_t mixedVideoAudio= static_cast<int>(agora::linuxsdk::MIXED_AV_CODEC_TYPE::MIXED_AV_DEFAULT);

  uint32_t getAudioFrame = agora::linuxsdk::AUDIO_FORMAT_DEFAULT_TYPE;
  uint32_t getVideoFrame = agora::linuxsdk::VIDEO_FORMAT_DEFAULT_TYPE;
  uint32_t streamType = agora::linuxsdk::REMOTE_VIDEO_STREAM_HIGH;
  int captureInterval = 5;
  int width = 0;
  int height = 0;
  int fps = 0;
  int kbps = 0;
  int triggerMode = agora::linuxsdk::AUTOMATICALLY_MODE;
  int audioIndicationInterval = 0;
  int logLevel = agora::linuxsdk::AGORA_LOG_LEVEL_INFO;
  int layoutMode = 0;
  int maxResolutionUid = -1;
  uint32_t keepLastFrame = 0;
  /**
   * change log_config Facility per your specific purpose like agora::base::LOCAL5_LOG_FCLT
   * Default:USER_LOG_FCLT. 
   * agora::base::log_config::setFacility(agora::base::LOCAL5_LOG_FCLT);
   */
  g_bSignalStop = false;
  g_bSignalStartService = false;
  g_bSignalStopService = false;

  signal(SIGQUIT, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGPIPE, SIG_IGN);

  opt_parser parser;

  parser.add_long_opt("appId", &appId, "App Id/must", agora::base::opt_parser::require_argu);

  parser.add_long_opt("channel", &name, "Channel Id/must", agora::base::opt_parser::require_argu);
  parser.add_long_opt("appliteDir", &applitePath, "directory of app lite 'AgoraCoreService', Must pointer to 'Agora_Recording_SDK_for_Linux_FULL/bin/' folder/must",
          agora::base::opt_parser::require_argu);
  parser.add_long_opt("uid", &uid, "User Id default is 0/option");
  parser.add_long_opt("userAccount", &userAccount, "User account, default uid is used/option");

  parser.add_long_opt("channelKey", &channelKey, "channelKey/option");
  parser.add_long_opt("channelProfile", &channelProfile, "channel_profile:(0:COMMUNICATION),(1:broadcast) default is 0/option");

  parser.add_long_opt("isAudioOnly", &isAudioOnly, "Default 0:A/V, 1:AudioOnly (0:1)/option");
  parser.add_long_opt("isVideoOnly", &isVideoOnly, "Default 0:A/V, 1:VideoOnly (0:1)/option");
  parser.add_long_opt("isMixingEnabled", &isMixingEnabled, "Mixing Enable? (0:1)/option");
  parser.add_long_opt("mixResolution", &mixResolution, "change default resolution for vdieo mix mode/option");
  parser.add_long_opt("mixedVideoAudio", &mixedVideoAudio, "mixVideoAudio:(0:seperated Audio,Video) (1:mixed Audio & Video with legacy codec) (2:mixed Audio & Video with new codec), default is 0 /option");

  parser.add_long_opt("decryptionMode", &decryptionMode, "decryption Mode, default is NULL/option");
  parser.add_long_opt("secret", &secret, "input secret when enable decryptionMode/option");

  parser.add_long_opt("idle", &idleLimitSec, "Default 300s, should be above 3s/option");
  parser.add_long_opt("recordFileRootDir", &recordFileRootDir, "recording file root dir/option");

  parser.add_long_opt("lowUdpPort", &lowUdpPort, "default is random value/option");
  parser.add_long_opt("highUdpPort", &highUdpPort, "default is random value/option");

  parser.add_long_opt("getAudioFrame", &getAudioFrame, "default 0 (0:save as file, 1:aac frame, 2:pcm frame, 3:mixed pcm frame) (Can't combine with isMixingEnabled) /option");
  parser.add_long_opt("getVideoFrame", &getVideoFrame, "default 0 (0:save as file, 1:encoded video frame, eg:h.264,h.265, 2:yuv, 3:jpg buffer, 4:jpg file, 5:jpg, file and video file) (Can't combine with isMixingEnabled) /option");
  parser.add_long_opt("captureInterval", &captureInterval, "default 5 (Video snapshot interval (second))");
  parser.add_long_opt("cfgFilePath", &cfgFilePath, "config file path / option");
  parser.add_long_opt("streamType", &streamType, "remote video stream type(0:STREAM_HIGH,1:STREAM_LOW), default is 0/option");
  parser.add_long_opt("triggerMode", &triggerMode, "triggerMode:(0: automatically mode, 1: manually mode) default is 0/option");
  parser.add_long_opt("proxyType", &proxyType, "proxyType:proxyServer format type, 0:self socks5 proxy server, 1:cloud proxy domain, 2:proxy LBS server list. default is 1/option");
  parser.add_long_opt("proxyServer", &proxyServer, "proxyServer:format proxyType:content, ie: 0:'ip:port', 1:'LBS domain:port', 2:'LBS_ip1,LBS_ip2:port' /option");
  parser.add_long_opt("audioProfile", &audioProfile, "audio quality: (0: single channelstandard 1: single channel high quality 2:multiple channel high quality");
  parser.add_long_opt("audioIndicationInterval", &audioIndicationInterval, "audioIndicationInterval:(0: no indication, audio indication interval(ms)) default is 0/option");
  parser.add_long_opt("defaultVideoBg", &videoBg, "default video background/option");
  parser.add_long_opt("defaultUserBg", &userBg, "default user background/option");
  parser.add_long_opt("logLevel", &logLevel, "log level default INFO/option");
  parser.add_long_opt("layoutMode", &layoutMode, "layoutMode:(0: default layout, 1:bestFit Layout mode, 2:vertical presentation Layout mode) default is 0/option");
  parser.add_long_opt("maxResolutionUid", &maxResolutionUid, "uid with maxest resolution under vertical presentation Layout mode if uid is used  ( default is -1 /option");
  parser.add_long_opt("maxResolutionUserAccount", &maxResolutionUserAccount, "user account with maxest resolution under vertical presentation layout mode if userAccount is used ( default is empty /option");
  parser.add_long_opt("autoSubscribe", &autoSubscribe, "Auto subscribe video/audio streams of each uid. (0: false 1: true, default 1 /option)");
  parser.add_long_opt("subscribeVideoUids", &subscribeVideoUids, "subscribe video stream of specific uids. uids seperated by commas, like 1234,2345/option");
  parser.add_long_opt("subscribeAudioUids", &subscribeAudioUids, "subscribe audio stream of specific uids. uids sperated by commas, like 1234,2345 /option");
  parser.add_long_opt("keepLastFrame", &keepLastFrame, "whether keep user's last video frame when no video stream(0: render user background image or corlor. 2. keep last farme. default is 0)");
  parser.add_long_opt("enableCloudProxy", &enableCloudProxy, "enable cloud proxy or not(0 : not, 1: enable, default 0/option");
  parser.add_long_opt("enableIntraRequest", &enableIntraRequest, "enable Intra Request or not(0 : not, 1: enable, default 1/option)");
  parser.add_long_opt("enableH265Support", &enableH265Support, "enable H.265 video codec or not(0 : not, 1: enable, default 0/option)");

  if (!parser.parse_opts(argc, argv) || appId.empty() || name.empty()) {
    std::ostringstream sout;
    parser.print_usage(argv[0], sout);
    cout<<sout.str()<<endl;
    return -1;
  }

  if (userAccount.length() > 0 && layoutMode == 2 && 
      maxResolutionUserAccount.empty() && maxResolutionUid != 0) {
    cout << "join channel with user account, maxResolutionUserAccount should be used instread of maxResolutionUid." << endl;
    return -1;
  }

  if (userAccount.length() == 0 && layoutMode == 2 &&
      !maxResolutionUserAccount.empty()) {
    cout << "join channel with uid, maxResolutionUid should be used instread of maxResolutionUserAccount." << endl;
    return -1;
  }
 
  if(triggerMode == agora::linuxsdk::MANUALLY_MODE) {
      signal(SIGUSR1, start_service);
      signal(SIGUSR2, stop_service);
  }
  
  if(recordFileRootDir.empty())
      recordFileRootDir = ".";

  //Once recording video under video mixing model, client needs to config width, height, fps and kbps
  if(isMixingEnabled && !isAudioOnly) {
     if(4 != sscanf(mixResolution.c_str(), "%d,%d,%d,%d", &width,
                  &height, &fps, &kbps)) {
        CM_LOG(ERROR, "Illegal resolution: %s", mixResolution.c_str());
        return -1;
     }
  }

  CM_LOG(INFO, "uid %" PRIu32 " from vendor %s is joining channel %s",
          uid, appId.c_str(), name.c_str());

  agora::AgoraSdk recorder;
  agora::recording::RecordingConfig config;
  config.idleLimitSec = idleLimitSec;
  config.channelProfile = static_cast<agora::linuxsdk::CHANNEL_PROFILE_TYPE>(channelProfile);

  config.isVideoOnly = isVideoOnly;
  config.isAudioOnly = isAudioOnly;
  config.isMixingEnabled = isMixingEnabled;
  config.mixResolution = (isMixingEnabled && !isAudioOnly)? mixResolution.c_str():NULL;
  config.mixedVideoAudio = static_cast<agora::linuxsdk::MIXED_AV_CODEC_TYPE>(mixedVideoAudio);

  config.appliteDir = applitePath.c_str();
  config.recordFileRootDir = recordFileRootDir.c_str();
  config.cfgFilePath = cfgFilePath.c_str();

  config.secret = secret.empty()? NULL : secret.c_str();
  config.decryptionMode = decryptionMode.empty()? NULL : decryptionMode.c_str();
  config.proxyType = proxyType;
  config.proxyServer = proxyServer.empty()? NULL : proxyServer.c_str();

  config.lowUdpPort = lowUdpPort;
  config.highUdpPort = highUdpPort;
  config.captureInterval = captureInterval;
  config.audioIndicationInterval = audioIndicationInterval;

  config.decodeAudio = static_cast<agora::linuxsdk::AUDIO_FORMAT_TYPE>(getAudioFrame);
  config.decodeVideo = static_cast<agora::linuxsdk::VIDEO_FORMAT_TYPE>(getVideoFrame);
  config.streamType = static_cast<agora::linuxsdk::REMOTE_VIDEO_STREAM_TYPE>(streamType);
  config.triggerMode = static_cast<agora::linuxsdk::TRIGGER_MODE_TYPE>(triggerMode);
  config.enableCloudProxy = enableCloudProxy;
  config.autoSubscribe = autoSubscribe;
  config.enableIntraRequest = enableIntraRequest;
  config.enableH265Support = enableH265Support;

  if (!autoSubscribe) {
    config.subscribeVideoUids = subscribeVideoUids.c_str();
    config.subscribeAudioUids = subscribeAudioUids.c_str();
  }

  if(audioProfile > 2) 
      audioProfile = 2;

  config.audioProfile = (agora::linuxsdk::AUDIO_PROFILE_TYPE)audioProfile;
  config.defaultVideoBg = videoBg.empty() ? NULL : const_cast<char*>(videoBg.c_str());
  config.defaultUserBg = userBg.empty() ? NULL : const_cast<char*>(userBg.c_str());

  recorder.updateMixModeSetting(width, height, isMixingEnabled ? !isAudioOnly:false);
  recorder.updateLayoutSetting(layoutMode, maxResolutionUid, maxResolutionUserAccount);
  if(logLevel < agora::linuxsdk::AGORA_LOG_LEVEL_FATAL)
      logLevel = agora::linuxsdk::AGORA_LOG_LEVEL_FATAL;
  if(logLevel > agora::linuxsdk::AGORA_LOG_LEVEL_DEBUG)
      logLevel = agora::linuxsdk::AGORA_LOG_LEVEL_DEBUG;
  recorder.setLogLevel((agora::linuxsdk::agora_log_level)logLevel);
  recorder.setKeepLastFrame(keepLastFrame);

  bool result = true;
  if (userAccount.length() > 0) {
    result = recorder.createChannelWithUserAccount(appId, channelKey, name, userAccount, config);
  } else {
    result = recorder.createChannel(appId, channelKey, name, uid, config);
  }

  if (!result) {
    cerr << "Failed to create agora channel: " << name << endl;
    cout << "Recording directory is " << recorder.getRecorderProperties()->storageDir << endl;
    return -1;
  }

  cout << "Recording directory is " << recorder.getRecorderProperties()->storageDir << endl;
  recorder.updateStorageDir(recorder.getRecorderProperties()->storageDir);
  
  while (!recorder.stopped() && !g_bSignalStop) {
      if(g_bSignalStartService) {
          recorder.startService();
          g_bSignalStartService = false;
      }

      if(g_bSignalStopService) {
          recorder.stopService();
          g_bSignalStopService = false;
      }

      sleep(1);
  }

  recorder.leaveChannel();
  recorder.release();

  cerr << "Stopped \n";
  return 0;
}

