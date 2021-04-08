#!/bin/bash
createBinFloder()
{
  binDir="bin"
  if [ -d "$binDir" ];then
  echo
  else
  mkdir "$binDir"
  fi
}
build_java()
{ 
  createBinFloder
   
  jniLayer="./native/jni"
  javaClassPath="./bin"

  javac src/io/agora/recording/common/*.java -d bin
  javac src/io/agora/recording/*.java -d bin
  javac src/io/agora/recording/test/*.java -d bin -Xlint:unchecked

  #clean previous jni file
  rm -f $jniLayer/io_agora_recording_RecordingSDK.h
  javah -d $jniLayer -classpath $javaClassPath io.agora.recording.RecordingSDK
}

build_cpp()
{
  make -f ./native/.makefile JNIINCLUDEPATH=$JNI_PATH
  mv ./bin/librecording.so ./bin/io/agora/recording/.
}
clean_java()
{
  rm -f bin/*.class
  rm -rf bin/io
  rm -rf bin/demo
}
clean_cpp()
{
  make clean -f ./native/.makefile
}
build()
{
  build_java
  build_cpp
  echo "build all done!"
}
clean()
{ 
  clean_java
  clean_cpp
  echo "clean all done!"
}

pre_set()
{
  JNI_PATH=$1
  export JNI_PATH
    
  CLASSPATH=`pwd`/bin
  export CLASSPATH
  
#  LD_LIBRARY_PATH=`pwd`/bin
#  export LD_LIBRARY_PATH

}

cmdhelp()
{
  echo "Usage:"
  echo "source build.sh pre_set jni_path"
  echo "$1 build"
  echo "$1 clean"

  #echo "$1 build_java"
  #echo "$1 build_cpp"
  #echo "$1 clean_java"
  #echo "$1 clean_cpp"
}
run()
{
  for param in $@
  do
    case $param in
       "build")
          build
          ;;
       "clean")
          clean
          ;;
       "build_java")
          #build_java
          ;;
       "build_cpp")
          #build_cpp
          ;;
        "clean_java")
          #clean_java
          ;;
        "clean_cpp")
          #clean_cpp
          ;;
       *)
       cmdhelp $0
       ;;
    esac
  done
}
if [ $# -eq "0" ];then
  cmdhelp $0
elif [ $# -eq "1" ];then
  run $1
elif [ $# -eq "2" ];then
  if [ "$1" = 'pre_set' ];then
    pre_set $2
  else
    cmdhelp $0
  fi
else
  cmdhelp $0
fi
