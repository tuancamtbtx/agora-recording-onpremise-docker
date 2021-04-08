> This Java API is a data encapsulation of the C++ sample code with JNI, and is therefore slightly different from that of the C++ API in structure. The Agora SDK (sample code shared by C++ and Java) implements the C++ recording APIs and callbacks, goes through data encapsulation in the JNI layer, and then works as the Java interface and class of the Native SDK through the JNI proxy.

- The {@link io.agora.recording.RecordingSDK RecordingSDK} class provides the main methods that can be invoked by your application.
- The {@link io.agora.recording.RecordingEventHandler RecordingEventHandler} class enables callbacks to your application.

@section RecordingSDK RecordingSDK Class

<table>
<tr>
<th>Method</th>
<th>Description</th>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#createChannel createChannel}</td>
<td>Creates a channel.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#createChannelWithUserAccount createChannelWithUserAccount}</td>
<td>Creates a channel with the user account.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#getUidByUserAccount getUidByUserAccount}</td>
<td>Gets the user ID by passing in the user account. </td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#getUserAccountByUid getUserAccountByUid}</td>
<td>Gets the user account by passing in the user ID.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#setVideoMixingLayout setVideoMixingLayout}</td>
<td>Sets the video mixing layout.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#updateWatermarkConfigs updateWatermarkConfigs}</td>
<td>Adds, updates, or deletes the watermark configurations.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#updateSubscribeVideoUids updateSubscribeVideoUids}</td>
<td>Updates the UIDs of the users whose video streams you want to record.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#updateSubscribeAudioUids updateSubscribeAudioUids}</td>
<td>Updates the UIDs of the users whose video streams you want to record.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#leaveChannel leaveChannel}</td>
<td>Allows the recording server to leave the channel.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#getProperties getProperties}</td>
<td>Retrieves the recording properties.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#startService startService}</td>
<td>Manually starts the recording.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#stopService stopService}</td>
<td>Manually pauses the recording.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#setUserBackground setUserBackground}</td>
<td>Sets the user background image.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingSDK#setLogLevel setLogLevel}</td>
<td>Sets the log level.</td>
</tr>
</table>


@section RecordingEventHandler RecordingEventHandler Class

<table>
<tr>
<th>Callback</th>
<th>Description</th>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onWarning onWarning}</td>
<td>Occurs when a warning occurs.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onError onError}</td>
<td>Occurs when an error occurs.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onConnectionLost onConnectionLost}</td>
<td>Occurs when the SDK loses connection to the server. </td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onConnectionInterrupted onConnectionInterrupted}</td>
<td>Occurs when the connection between the SDK and the server is interrupted.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onConnectionStateChanged onConnectionStateChanged}</td>
<td>Occurs when the network connection state changes.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onJoinChannelSuccess onJoinChannelSuccess}</td>
<td>Occurs when the recording server joins a channel.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onLocalUserRegistered onLocalUserRegistered}</td>
<td>Occurs when the recording server successfully registers a user account.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onUserInfoUpdated onUserInfoUpdated}</td>
<td>Occurs when the SDK gets the user ID and user account of a remote user.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onRejoinChannelSuccess onRejoinChannelSuccess}</td>
<td>Occurs when the recording server rejoins the channel after being disconnected due to network problems.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onLeaveChannel onLeaveChannel}</td>
<td>Occurs when the recording server leaves the channel.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onUserJoined onUserJoined}</td>
<td>Occurs when a user joins the channel.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onUserOffline onUserOffline}</td>
<td>Occurs when a user leaves the channel or goes offline.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#audioFrameReceived audioFrameReceived}</td>
<td>Occurs when the raw audio data is received.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#videoFrameReceived videoFrameReceived}</td>
<td>Occurs when the raw video data is received.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onAudioVolumeIndication onAudioVolumeIndication}</td>
<td>Reports the list of users who are speaking and their volumes.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onActiveSpeaker onActiveSpeaker}</td>
<td>Occurs when a speaker is detectesd in the channel.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onFirstRemoteAudioFrame onFirstRemoteAudioFrame}</td>
<td>Occurs when the first remote audio frame is received.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onFirstRemoteVideoDecoded onFirstRemoteVideoDecoded}</td>
<td>Occurs when the first remote video frame is decoded. </td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onReceivingStreamStatusChanged onReceivingStreamStatusChanged}</td>
<td>Occurs when the status of receiving the audio or video stream changes. </td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onRemoteAudioStreamStateChanged onRemoteAudioStreamStateChanged}</td>
<td>Occurs when the state of a remote user's audio stream changes.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onRemoteVideoStreamStateChanged onRemoteVideoStreamStateChanged}</td>
<td>Occurs when the state of a remote user's video stream changes.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onRemoteVideoStats onRemoteVideoStats}</td>
<td>Reports the statistics of the video stream from the remote user.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onRemoteAudioStats onRemoteAudioStats}</td>
<td>Reports the statistics of the audio stream from the remote user.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#onRecordingStats onRecordingStats}</td>
<td>Reports the statistics of the recording once every two seconds.</td>
</tr>
<tr>
<td>{@link io.agora.recording.RecordingEventHandler#recordingPathCallBack recordingPathCallBack}</td>
<td>Reports the directory of the recorded files.</td>
</tr>
</table>
