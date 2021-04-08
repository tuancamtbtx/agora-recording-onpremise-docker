## 快速开始
- [产品介绍](https://docs.agora.io/cn/Recording/product_recording?platform=Linux)
- [C++ API](https://docs.agora.io/cn/Recording/API%20Reference/recording_cpp/index.html)

## 编译运行
- 环境准备: CentOS 6.5 x64 或者 Ubuntu 14.04 x64及以上；gcc/g++ 4.8.2及以上
- sample编译：到samples/cpp目录下，直接运行make即可编译出 录制可执行文件 recorder_local
- sample运行: 运行./recorder_local 即可看到相关用法，加上相关参数即可进行音视频录制

## 运行样例
```
./recorder_local --appId XXXXXX --uid XXX --channel XXX --appliteDir ../../bin/ 
```
注：详细参数，参考[官网](https://docs.agora.io/cn/Recording/recording_cmd_cpp?platform=Linux%20CPP)

## Sample代码框架
### 说明
Agora Recording CPP API 包含两个接口类：
- IRecordingEngine 接口类包含应用程序调用的主要方法
- IRecordingEngineEventHandler 接口类用于向应用程序发送回调通知
### 代码框架
- include/IAgoraRecordingEngine.h 所有程序调用和回调API
- include/IAgoraLinuxSdkCommon.h  所有程序调用和回调用到的数据结构
- ==samples/agorasdk/AgoraSdk.cpp== 该cpp文件包含了CPP API所有程序调用以及回调的实现。熟悉该文件，即可基本掌握CPP API的使用方法
- ==samples/cpp/main.cpp== 该cpp是recorder_local程序入口，主要进行参数解析和调用AgoraSdk.cpp的方法，启动录制进程。
- samples/base/opt_parser.cpp mian.cpp文件中进行参数解析的实现
- ==集成方可在sample基础上删除main.cpp，将AgoraSdk.cpp的方法调用集成到自己的业务逻辑中，即可完成基本功能的快速集成。==