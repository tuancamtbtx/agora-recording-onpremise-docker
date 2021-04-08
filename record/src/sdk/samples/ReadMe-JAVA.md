## 快速开始
- [产品介绍](https://docs.agora.io/cn/Recording/product_recording?platform=Linux)
- [JAVA API](https://docs.agora.io/cn/Recording/API%20Reference/recording_java/index.html)

## Sample概要
Java API是对Cpp中的sample code通过JNI做的二次封装。所以和原生录制SDK提供的C++ API不完全一样。samples/agorasdk(C++ 的sample code)实现录制原生C++ API的接口以及对callback的处理,然后jni层再封装agorasdk,通过jni  proxy层提供native接口供Java层调用.

## 编译运行
编译后bin文件夹下会生成 librecording.so ,此动态库是封装了samples下agorasdk APIs后生成的动态库文件,java demo可以通过jni的方式调用.
### 环境准备:
- CentOS 6.5 x64 或者 Ubuntu 14.04 x64及以上；gcc/g++ 4.8.2及以上
- Java 环境,可以正常编译并运行hello world.建议Java Version 8
### 编译：
1.  ==source build.sh pre_set XXX==。 XXX表示jni所在路径，可通过locate jni.h 或者 find /usr -name jni.h 等方式查找
```
例如：source build.sh pre_set /usr/lib/jvm/java-xxxxx-amd64/include;
```
2.  build使用: 
```
./build.sh build ;
```
3.  clean使用
```
./build.sh clean 
```

### 运行样例：

```
java -Djava.library.path=io/agora/recording/ RecordingSampleM --appId XXXXXX  --uid XXX --channel XXX --appliteDir ../../../bin/
```
注：详细参数，参考[官网](https://docs.agora.io/cn/Recording/recording_cmd_java?platform=Linux%20Java)


## Sample代码框架
### 说明
Agora Recording JAVA API 包含两个接口类：
- RecordingSDK.java 接口类包含应用程序调用的主要方法。
- RecordingEventHandler .java 接口类用于向应用程序发送回调通知。
### 代码框架
- ==java/native==： java api对应的jni c++ 接口以及对samples/agorasdk的jni封装。该层接收来自java的调用以及来自c++的回调并透传到java
- ==io/agora/recording/common== 所有Java程序调用和回调用到的数据结构
- ==io/agora/recording/test RecordingSample.java/RecordingSampleM.java==文件包含了JAVA API所有程序调用以及回调的实现。熟悉该文件，即可基本掌握JAVA API的使用方法
- ==集成方可在sample基础上将RecordingSample.java的方法调用集成到自己的业务逻辑中，即可完成基本功能的快速集成。==