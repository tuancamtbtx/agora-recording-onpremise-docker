package io.agora.recording;

import java.io.File;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.List;

import io.agora.recording.common.Common.AudioFrame;
import io.agora.recording.common.Common.VideoFrame;
import io.agora.recording.common.Common.VideoMixingLayout;
import io.agora.recording.common.RecordingConfig;
import io.agora.recording.common.RecordingEngineProperties;
import io.agora.recording.common.Common.AudioVolumeInfo;
import io.agora.recording.common.Common.LiteraWatermarkConfig;
import io.agora.recording.common.Common.TimestampWatermarkConfig;
import io.agora.recording.common.Common.ImageWatermarkConfig;
import io.agora.recording.common.Common.RemoteAudioStats;
import io.agora.recording.common.Common.RemoteVideoStats;
import io.agora.recording.common.Common.RecordingStats;
import io.agora.recording.common.Common.REMOTE_STREAM_STATE;
import io.agora.recording.common.Common.REMOTE_STREAM_STATE_CHANGED_REASON;
import io.agora.recording.common.Common.CONNECTION_STATE_TYPE;
import io.agora.recording.common.Common.CONNECTION_CHANGED_REASON_TYPE;

public class RecordingSDK {

  private List<RecordingEventHandler> recordingEventHandlers = null;
  private long nativeHandle = 0;
  /** The maximum length of the user account.  */
  public static int MAX_USER_ACCOUNT_LENGTH = 256;

  /** Load Java library. */

  static {
    System.loadLibrary("recording");
  }

  /** Main methods that can be invoked by your application.*/
  public RecordingSDK() {
    recordingEventHandlers = new ArrayList<RecordingEventHandler>();
  }

  /** To register observer to receive Recording event notification. */
  public void registerOberserver(RecordingEventHandler recordingEventHandler) {
    if (!recordingEventHandlers.contains(recordingEventHandler)) {
      recordingEventHandlers.add(recordingEventHandler);
    }
  }

  /** To remove previously registered observer. */
  public void unRegisterOberserver(RecordingEventHandler recordingEventHandler) {
    if (recordingEventHandlers.contains(recordingEventHandler)) {
      recordingEventHandlers.remove(recordingEventHandler);
    }
  }

  /** This method creates a channel and enables the recording server to join the channel.
   *
   * @param appId Set `appId` of the recording server the same as that of the Native/Web SDK. For more information, see <a href="https://docs.agora.io/en/Recording/token?platform=All%20Platforms">Getting an App ID</a>. 
   *
   * @param channelKey The dynamic key for authentication. Set `channelKey` of the recording server the same as that of the Native/Web SDK. If the Native/Web SDK uses a token, `channelKey` must be set as the token. For more information, see <a href="https://docs.agora.io/en/Recording/token?platform=All%20Platforms">Use Security Keys</a>. In the Recording SDK, `requestToken` and `renewToken` are private interfaces. Therefore, ensure that you set <a href="https://docs.agora.io/en/Recording/token?platform=All%20Platforms#Generate_Token">expireTimestamp</a> as 0 when generating a token, which means that the privilege, once generated, never expires.
   *
   * @param name The name of the channel to be recorded.
   *
   * @param uid The unique identifier of a user. A channel does not accept duplicate uids. Otherwise, there will be unpredictable behaviors. 
   * <ul>
   *   <li>If you set `uid` as 0, the SDK randomly assigns a uid and returns it in the {@link io.agora.recording.RecordingEventHandler#onJoinChannelSuccess onJoinChannelSuccess}.</li>
   *   <li>If you set your own `uid`, it should be a 32-bit unsigned integer ranging from 1 to (2<sup>32</sup>-1).User ID.</li>
   * </ul>
   * @param config Detailed recording configuration. See {@link io.agora.recording.common.RecordingConfig RecordingConfig}.
   *
   * @param logLevel Sets the log level. Only logs in the selected level and levels preceding the selected level are generated.
   * <ul>
   *   <li>1: Fatal.</li>
   *   <li>2: Error.</li>
   *   <li>3: Warn.</li>
   *   <li>4: Notice.</li>
   *   <li>5: Info.</li>
   *   <li>6: Debug.<li>
   * </ul>
   *
   * @return
   * <ul>
   *   <li>0: Success.</li>
   *   <li>< 0: Failure.</li>
   * </ul>
   */
  public native boolean createChannel(String appId, String channelKey, String name, int uid, RecordingConfig config, int logLevel);

  /** This method creates a channel and enables the recording server to join with the user account.
   * 
   * After the recording server successfully joins the channel, the SDK triggers the {@link io.agora.recording.RecordingEventHandler.onLocalUserRegistered onLocalUserRegistered} and {@link io.agora.recording.RecordingEventHandler.onJoinChannelSuccess onJoinChannelSuccess} callbacks on the local client.
   * 
   * @note To ensure smooth communication, use the same parameter type to identify the users in the same channel. Hence, the parameter type of the recording server's identifier should be the same as that of the other users joining the channel with the Agora Native/Web SDK.
   * 
   * @param appId Set `appId` of the recording server the same as that of the Native/Web SDK. A channel does not accept duplicate uids. Otherwise, there will be unpredictable behaviors. For more information, see <a href="https://docs.agora.io/en/Recording/token?platform=All%20Platforms">Getting an App ID</a>. 
   * 
   * @param channelKey The dynamic key for authentication. Set `channelKey` of the recording server the same as that of the Native/Web SDK. If the Native/Web SDK uses a token, `channelKey` must be set as the token. For more information, see <a href="https://docs.agora.io/en/Recording/token?platform=All%20Platforms">Use Security Keys</a>. In the Recording SDK, `requestToken` and `renewToken` are private interfaces. Therefore, ensure that you set <a href="https://docs.agora.io/en/Recording/token?platform=All%20Platforms#Generate_Token">expireTimestamp</a> as 0 when generating a token, which means that the privilege, once generated, never expires.
   * 
   * @param name The name of the channel to be recorded.
   * 
   * @param userAccount The user account of the recording server. The maximum length of this parameter is 255 bytes. Ensure that you set this parameter and do not set it as null. Supported character scopes are:
   * <ul>
   *   <li>The 26 lowercase English letters: a to z.</li>
   *   <li>The 26 uppercase English letters: A to Z.</li>
   *   <li>The 10 numbers: 0 to 9.</li>
   *   <li>The space.</li>
   *   <li>"!", "#", "$", "%", "&", "(", ")", "+", "-", ":", ";", "<", "=", ".", ">", "?", "@", "[", "]", "^", "_", " {", "}", "|", "~", ",".</li>
   * </ul>
   * 
   * @param config Detailed recording configuration. See {@link io.agora.recording.common.RecordingConfig RecordingConfig}.
   * 
   * @param logLevel Sets the log level. Only logs in the selected level and levels preceding the selected level are generated.
   * <ul>
   *   <li>1: Fatal.</li>
   *   <li>2: Error.</li>
   *   <li>3: Warn.</li>
   *   <li>4: Notice.</li>
   *   <li>5: Info.</li>
   *   <li>6: Debug.<li>
   * </ul>
   * 
   * @return
   * <ul>
   *   <li>0: Success.</li>
   *   <li>< 0: Failure.</li>
   * </ul>
   */
  public native boolean createChannelWithUserAccount(String appId, String channelKey, String name, String userAccount, RecordingConfig config, int logLevel);

  /** Gets the user ID by passing in the user account.
   * 
   * After a remote user joins the channel, the SDK gets the user ID and user account of the remote user and triggers the {@link io.agora.recording.RecordingEventHandler.onUserInfoUpdated onUserInfoUpdated} callback on the local client.
   * 
   * After receiving the {@link io.agora.recording.RecordingEventHandler.onUserInfoUpdated onUserInfoUpdated} callback, you can call the `getUidByUserAccount` method to get the user ID of the remote user by passing in the user account.
   * 
   * @param userAccount The user account of the remote user. Ensure that you set this parameter.
   * 
   * @return The user ID of the remote user.
   */
  public long getUidByUserAccount(String userAccount) {
    if (nativeHandle != 0) {
        return getUidByUserAccount(nativeHandle, userAccount);
    }

    return 0;
  }

  private native long getUidByUserAccount(long nativeHandle, String userAccount);

  /** Gets the user account by passing in the user ID.
   * 
   * After a remote user joins the channel, the SDK gets the user ID and user account of the remote user and triggers the {@link io.agora.recording.RecordingEventHandler.onUserInfoUpdated onUserInfoUpdated} callback on the local client.
   * 
   * After receiving the {@link io.agora.recording.RecordingEventHandler.onUserInfoUpdated onUserInfoUpdated} callback, you can call the `getUserAccountByUid` method to get the user account of the remote user from the UserInfo object by passing in the user ID.
   * 
   * @param uid The user ID of the remote user. Ensure that you set this parameter.
   * 
   * @return The user account of the remote user.
   */
  public String getUserAccountByUid(int uid) {
    if (nativeHandle != 0) {
      return getUserAccountByUid(nativeHandle, uid);
    }
    return "";
  }

  private native String getUserAccountByUid(long nativeHandle, int uid);

  /** This method allows the recording server to leave the channel and release the thread resources.
   *
   * @return
   * <ul>
   *   <li>0: Success.</li>
   *   <li>< 0: Failure.</li>
   * </ul>
   */
  public synchronized boolean leaveChannel() {
    if (nativeHandle != 0) {
      long temp = nativeHandle;
      nativeHandle = 0;
      return leaveChannel(temp);
    }
    return false;
  }

  private native boolean leaveChannel(long nativeHandle);

  /** This method sets the video layout in composite recording mode.
   * 
   * @note If you record video in composite recording mode, you must call this method to set the video layout.
   *
   * @param layout Layout setting. See {@link io.agora.recording.common.Common.VideoMixingLayout VideoMixingLayout}.
   *
   * @return
   * <ul>
   *   <li>0: Success.</li>
   *   <li>< 0: Failure.</li>
   * </ul>
   */
  public int setVideoMixingLayout(VideoMixingLayout layout) {
    return setVideoMixingLayout(nativeHandle, layout);
  }

  private native int setVideoMixingLayout(long nativeHandle, VideoMixingLayout layout);
  
  /** This method adds, updates, or deletes the watermark configurations.
   * 
   * The Agora Recording SDK supports three types of watermarks: text watermarks, timestamp watermarks, and image watermarks.
   * 
   * <ul>
   * <li>If you do not set `LiteraWatermarkConfig`, `TimestampWatermarkConfig`, or `ImageWatermarkConfig` to add watermarks when calling the {@link io.agora.recording.RecordingSDK.setVideoMixingLayout setVideoMixingLayout} method, you can directly call the `updateWatermarkConfigs` method to add watermarks. </li>
   * <li>If you set `LiteraWatermarkConfig`, `TimestampWatermarkConfig`, or `ImageWatermarkConfig` to add watermarks when calling the {@link io.agora.recording.RecordingSDK.setVideoMixingLayout setVideoMixingLayout} method, you can call the `updateWatermarkConfigs` method to add, update, or delete watermarks. If you pass null as parameters in the `updateWatermarkConfigs` method, you delete watermarks that have been added. </li>
   * </ul>
   * 
   * @note Watermarks apply only to the videos recorded in composite recording mode (the {@link io.agora.recording.common.RecordingConfig.isMixingEnabled isMixingEnabled} parameter in the {@link io.agora.recording.common.RecordingConfig RecordingConfig} is set as true).
   * 
   * @param literaWms Adds text watermarks. Pointer to an array of {@link io.agora.recording.common.Common#LiteraWatermarkConfig LiteraWatermarkConfig}. You can add up to ten text watermarks.
   * 
   * @param timestampWms Adds a timestamp watermark. Pointer to {@link io.agora.recording.common.Common#TimestampWatermarkConfig TimestampWatermarkConfig}. You can only add one timestamp watermark.
   * 
   * @param imgWms Adds image watermarks. Pointer to an array of {@link io.agora.recording.common.Common#ImageWatermarkConfig ImageWatermarkConfig}. You can add up to four image watermarks.
   * 
   * @return
   * <ul>
   * <li>0: Success.</li>
   * <li>< 0: Failure.</li>
   * </ul>
   */
  public int updateWatermarkConfigs(LiteraWatermarkConfig[] literaWms, TimestampWatermarkConfig[] timestampWms, ImageWatermarkConfig[] imgWms) {	
    return updateWatermarkConfigs(nativeHandle, literaWms, timestampWms, imgWms);	
  }	

  private native int updateWatermarkConfigs(long nativeHandle, LiteraWatermarkConfig[] literaWms, TimestampWatermarkConfig[] timestampWms, ImageWatermarkConfig[] imgWms);

  /** This method updates the UIDs of the users whose video streams you want to record.
   * 
   * @note Ensure that you set the {@link io.agora.recording.common.RecordingConfig#autoSubscribe autoSubscribe} parameter in the {@link io.agora.recording.common.RecordingConfig RecordingConfig} as false before calling this method.
   * 
   * @param uids An array of UIDs whose video streams you want to record in the string format, such as {"1","2","3"}.
   * 
   * @return
   * <ul>
   * <li>0: Success.</li>
   * <li>< 0: Failure.</li>
   * </ul>
   */
  public int updateSubscribeVideoUids(int[] uids) {
    return updateSubscribeVideoUids(nativeHandle, uids);
  }

  private native int updateSubscribeVideoUids(long nativeHandle, int[] uids);

  /** This method updates the UIDs of the users whose audio streams you want to record.
   * 
   * @note Ensure that you set the {@link io.agora.recording.common.RecordingConfig.autoSubscribe autoSubscribe} parameter in the {@link io.agora.recording.common.RecordingConfig RecordingConfig} as false before calling this method.
   * 
   * @param uids An array of UIDs whose audio streams you want to record in the string format, such as {"1","2","3"}.
   * 
   * @return
   * <ul>
   * <li>0: Success.</li>
   * <li>< 0: Failure.</li>
   * </ul>
   */
  public int updateSubscribeAudioUids(int[] uids) {
    return updateSubscribeAudioUids(nativeHandle, uids);
  }

  private native int updateSubscribeAudioUids(long nativeHandle, int[] uids);

  /** This method manually starts a recording.
   * 
   * The method is only valid when you set {@link io.agora.recording.common.RecordingConfig#triggerMode triggerMode} in {@link io.agora.recording.common.RecordingConfig RecrordingConfig} as 1 (manually) when joining the channel.
   *
   * @return
   * <ul>
   *   <li>0: Success.</li>
   *   <li>< 0: Failure.</li>
   * </ul>
   */
  public int startService() {
    return startService(nativeHandle);
  }

  private native int startService(long nativeHandle);

  /** This method manually pauses the recording.
   * 
   * The method is only valid when you set {@link io.agora.recording.common.RecordingConfig#triggerMode triggerMode} in {@link io.agora.recording.common.RecordingConfig RecordingConfig} as 1 (manually) when joining the channel.
   *
   * @return
   * <ul>
   *   <li>0: Success.</li>
   *   <li>< 0: Failure.</li>
   * </ul>
   */
  public int stopService() {
    return stopService(nativeHandle);
  }

  private native int stopService(long nativeHandle);

  /** This method allows you to retrieve the recording properties.
   *
   * @note
   * <ul>
   *   <li>Call this method after joining the channel.</li>
   *   <li>The recording properties only include the relative path of the recording files.</li>
   *   <li>Both the `getProperties` method and the {@link RecordingEventHandler#onUserJoined onUserJoined} callback report the relative path of the recorded files and recording log. The difference between these two functions is that the recording SDK only triggers the {@link RecordingEventHandler#onUserJoined onUserJoined} callback when a remote user joins the channel. </li>
   * </ul>
   *
   * @return See {@link io.agora.recording.common.RecordingEngineProperties RecordingEngineProperties}.
   */
  public RecordingEngineProperties getProperties() {
    return getProperties(nativeHandle);
  }
  
  private native RecordingEngineProperties getProperties(long nativeHandle);

  /** This method sets the background image of a specified user. 
   * 
   * When the user is online but does not send any video stream, the background image is displayed.
   * 
   * @note The background image is not displayed for users using the Agora Web SDK.
   *
   * @param uid The UID of the user for whom the background image to be set.
   * @param imagePath The path of the image file. Only supports local images in JPEG format.
   * 
   * @return
   * <ul>
   *   <li>0: Success.</li>
   *   <li>< 0: Failure.</li>
   * </ul>
   */
  public int setUserBackground(int uid, String imagePath) {
    return setUserBackground(uid, imagePath);
  }

  private native int setUserBackground(long nativeHandle, int uid, String image_path);
  
  /** This method sets the log level.
   * 
   * Only log in the selected level and levels preceding the selected level are generated. The default value of the log level is 5.
   * 
   * @param level The log level: 
   * <ul>
   *   <li>1: Fatal.</li>
   *   <li>2: Error.</li>
   *   <li>3: Warn.</li>
   *   <li>4: Notice.</li>
   *   <li>5: Info.</li>
   *   <li>6: Debug.<li>
   * </ul>
   * 
   * @return
   * <ul>
   *   <li>0: Success.</li>
   *   <li>< 0: Failure.</li>
   * </ul>
   */
  public void setLogLevel(int level) {
    setLogLevel(nativeHandle, level);
  }

  private native void setLogLevel(long nativeHandle, int level);

  /*
   * Brief: When call createChannel successfully, JNI will call back this
   * method to set the recording engine.
   */
  private void nativeObjectRef(long nativeHandle) {
    this.nativeHandle = nativeHandle;
  }

  /*
   * Brief: Callback when recording server successfully left the channel
   * 
   * @param reason leave path reason, please refer to the define of
   * LEAVE_PATH_CODE
   */
  private void onLeaveChannel(int reason) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
      oberserver.onLeaveChannel(reason);
    }
  }

  /*	
   * Brief: Callback when an error occurred during the runtime of recording	
   * engine
   * 
   * @param error Error code, please refer to the define of ERROR_CODE_TYPE	
   * 	
   * @param error State code, please refer to the define of STAT_CODE_TYPE	
   */
  private void onError(int error, int stat_code) {
    nativeHandle = 0;
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
      oberserver.onError(error, stat_code);
    }
  }

  /*	
   * Brief: Callback when an warning occurred during the runtime of recording
   * engine
   * 
   * @param warn Warning code, please refer to the define of WARN_CODE_TYPE	    }
   */
  private void onWarning(int warn) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
      oberserver.onWarning(warn);
    }
  }

  /*
   * Brief: Callback when a user left the channel or gone offline	
   *
   * @param uid user ID
   * @param reason offline reason, please refer to the define of 
   * USER_OFFLINE_REASON_TYPE	
   */
  private void onUserOffline(long uid, int reason) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
      oberserver.onUserOffline(uid, reason);
    }
  }

  /*
   * Brief: Callback when a user's audio state changed
   * 
   * @param uid user ID
   * @state 0 for normal stream, 1 for no stream
   * 
   * @param reason offline reason, please refer to the define of
   * USER_OFFLINE_REASON_TYPE
   */
  private void onRemoteAudioStreamStateChanged(long uid, int state, int reason) {
    for (RecordingEventHandler observer : recordingEventHandlers) {
      observer.onRemoteAudioStreamStateChanged(uid, REMOTE_STREAM_STATE.values()[state], REMOTE_STREAM_STATE_CHANGED_REASON.values()[reason]);
    }
  }

  /*
   * Brief: Callback when a user's video state changed
   * 
   * @param uid user ID
   * @state 0 for normal stream , 1 for no stream
   * 
   * @param reason offline reason, please refer to the define of
   * USER_OFFLINE_REASON_TYPE
   */
  private void onRemoteVideoStreamStateChanged(long uid, int state, int reason) {
    for (RecordingEventHandler observer : recordingEventHandlers) {
      observer.onRemoteVideoStreamStateChanged(uid, REMOTE_STREAM_STATE.values()[state], REMOTE_STREAM_STATE_CHANGED_REASON.values()[reason]);
    }
  }

  /*
   * Brief: Callback when another user successfully joined the channel
   * 
   * @param uid user ID
   * 
   * @param recordingDir user recorded file directory
   */
  private void onUserJoined(long uid, String recordingDir) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
      oberserver.onUserJoined(uid, recordingDir);
    }
  }

  /*	
	 * Brief: Callback when received a audio frame	
	 * 	
	 * @param uid user ID	
	 * 	
	 * @param type type of audio frame, please refer to the define of AudioFrame	
	 * 	
	 * @param frame reference of received audio frame	
	 */
  private void audioFrameReceived(long uid, AudioFrame frame) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
      oberserver.audioFrameReceived(uid, frame);
    }
  }

  /*	
   * Brief: Callback when user is the active speaker 	
   * @param uid  user ID	
   */
  private void onActiveSpeaker(long uid) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
            oberserver.onActiveSpeaker(uid);
    }
  }

  /*
   * Brief: Callback to indicate audio volume
   * @param AudioVolumeInfo[]  audio info arrary
   */
  private void onAudioVolumeIndication(AudioVolumeInfo[] infos) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
            oberserver.onAudioVolumeIndication(infos);
    }
  }

  /*
   * Brief: Callback to indicate receiving stream status changed
   * @param receivingAudio  receiving audio stream status
   * @param receivingVideo  receiving video stream status
   */
  private void onReceivingStreamStatusChanged(boolean receivingAudio, boolean receivingVideo) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
            oberserver.onReceivingStreamStatusChanged(receivingAudio, receivingVideo);
    }
  }

  /*
   * when the network can not worked well, the function will be called
   */
  private void onConnectionLost() {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
            oberserver.onConnectionLost();
    }
  }

  /*
   * when local user disconnected by accident, the function will be called(then SDK will try to reconnect itself)
   */
  private void onConnectionInterrupted() {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
            oberserver.onConnectionInterrupted();
    }
  }
  /*
   * Brief: when the first remote video frame decoded, the function will be called
   *
   * @param uid the UID of the remote user
   *
   * @param width the width of the video frame
   *
   * @param height the height of the video frame
   *
   * @param elapsed the time elapsed from channel joined in ms
   */
  private void onFirstRemoteVideoDecoded(long uid, int width, int height, int elapsed) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
            oberserver.onFirstRemoteVideoDecoded(uid, width, height, elapsed);
    }
  }

  /*
   * Brief: when the first remote audio frame arrived, the function will be called
   *
   * @param uid the UID of the remote user
   *
   * @param elapsed the time elapsed from channel joined in ms
   */
  private void onFirstRemoteAudioFrame(long uid, int elapsed) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
            oberserver.onFirstRemoteAudioFrame(uid, elapsed);
    }
  }

  /*
   * Brief: Callback when received a video frame
   *
   * @param uid user ID
   *
   * @param type type of video frame, please refer to the define of VideoFrame
     *
     * @param frame reference of received video frame
     *
     * @param rotation rotation of video
     */
  private void videoFrameReceived(long uid, int type, VideoFrame frame, int rotation) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
      oberserver.videoFrameReceived(uid, type, frame, rotation);
    }
  }

  /*
   * Brief: Callback when JNI layer exited
   */
  private synchronized void stopCallBack() {
    nativeHandle = 0;
  }

  /*	
   * Brief: Callback when call createChannel successfully	
   * 	
   * @param path recording file directory	
   */
  private void recordingPathCallBack(String path) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
      oberserver.recordingPathCallBack(path);
    }
  }

  /*	
   * Brief: Callback when the user hase successfully joined the specified channel
   * @param channelID    channel ID onError
   * @param uid          User ID
   */
  private void onJoinChannelSuccess(String channelId, long uid) {
    for (RecordingEventHandler oberserver : recordingEventHandlers) {
      oberserver.onJoinChannelSuccess(channelId, uid);
    }
  }

  private void onLocalUserRegistered(long uid, String userAccount) {
    for (RecordingEventHandler observer : recordingEventHandlers) {
      observer.onLocalUserRegistered(uid, userAccount);
    }
  }

  private void onUserInfoUpdated(long uid, String userAccount) {
    for (RecordingEventHandler observer : recordingEventHandlers) {
      observer.onUserInfoUpdated(uid, userAccount);
    }
  }

  void onRejoinChannelSuccess(String channelId, long uid) {	
    for (RecordingEventHandler observer : recordingEventHandlers) {	
      observer.onRejoinChannelSuccess(channelId, uid);
    }
  }

  void onConnectionStateChanged(int state, int reason) {
    for (RecordingEventHandler observer : recordingEventHandlers) {
      observer.onConnectionStateChanged(CONNECTION_STATE_TYPE.values()[state], CONNECTION_CHANGED_REASON_TYPE.values()[reason]);
    }
  }

  void onRemoteVideoStats(long uid, RemoteVideoStats stats) {
    for (RecordingEventHandler observer : recordingEventHandlers) {
      observer.onRemoteVideoStats(uid, stats);
    }
  }

  void onRemoteAudioStats(long uid, RemoteAudioStats stats) {
    for (RecordingEventHandler observer : recordingEventHandlers) {
      observer.onRemoteAudioStats(uid, stats);
    }
  }

  void onRecordingStats(RecordingStats stats) {
    for (RecordingEventHandler observer : recordingEventHandlers) {
      observer.onRecordingStats(stats);
    }
  }

}
