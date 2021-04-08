#!/bin/bash -x

putUsage()
{
  echo "$0 [recordDir]"
  echo -e "\trecordDir: recording files directory"
}

# args checking & putUsage
if [ $# -lt 1 ];then
  putUsage $0
  exit 1
fi

# check dependencies
if [ ! -f ./ffmpeg ];then
    echo "Please unzip ffmpeg.tar.gz firstly!"
    exit 1
fi

# check dependencies
if [ ! -f $(which python3) ];then
    echo "Please intall python3"
    echo "sudo apt install python3"
    exit 1
fi

# record files not exist
if [ ! -d $1 ];then
    echo "recordDir $1 not exist!"
    exit 1
fi

# no dummy uidtxt files in recordDir
crashUidTxt=$(find $1 -name "uid*.txt" -size 0c | wc -l)
# no TS files in recordDir
crashTSCount=$(find $1 -name "*.ts" | wc -l)
if  [ $crashTSCount=="0" ] && [ $crashUidTxt=="0" ];then
    echo "no recorder crashed"
    exit 2
fi

# remuxer TS into MP4
TSList=".crashTS.log"
find $1 -name "*.ts" > ${TSList}
for line in $(cat ${TSList})
do
    echo "remuxer $line ..."
    srcTS="${line}"
    dstMP4="${line%.*}.mp4"

    ./ffmpeg -i ${srcTS} -vcodec copy -acodec copy -n ${dstMP4} 2>&1 1>>crash-err.log
done
rm -f ${TSList}

TXTList=".uidTxt.log"
find $1 -name "uid*.txt" > ${TXTList}
for line in $(cat ${TXTList})
do
    uidTxt=$(basename ${line})
    dirPath=$(dirname ${line})
    if [ ! -s ${line} ];then
        echo "restore $line ..."
        cp -Pf "${dirPath}/.${uidTxt}" $line
    fi
#python3 uid_txt_restore.py --recordDir ${dirPath}

python3 - --recordDir ${dirPath} <<EOF
import re
import shutil
import os,sys
import argparse
import subprocess
#import pysnooper

#snoop log
#DEBUGLOG='/dev/null'
DEBUGLOG='./uid_txt_restore.log'

class argsParser:
    def __init__(self):
        self.recordDir = "./"

    def __setattr__(self, key, value):
        super(argsParser, self).__setattr__(key, value)

    def init(self, cmdArgs):
        self._parse_cmd_line(cmdArgs)

    def _parse_cmd_line(self, cmdArgs):
        self.recordDir = cmdArgs.recordDir if cmdArgs.recordDir is not None else self.recordDir

#@pysnooper.snoop(DEBUGLOG)
def parse_arglist():
    parser = argparse.ArgumentParser()
    parser.add_argument("--recordDir", action="store", type=str, default=".", help="Input file dir.")
    args = parser.parse_args()
    return args

#@pysnooper.snoop(DEBUGLOG)
def probe_av_duration(src):
    if os.path.getsize(src)<16:
        return b'0.1'

    #cmd1_ffprobe_duration = '''ffprobe -hide_banner -show_streams {in_file} 2>&1 | grep "duration="'''
    cmd2_ffprobe_duration = '''ffprobe -v error -select_streams 0 -show_entries stream=duration -of default=noprint_wrappers=1:nokey=1 {in_file}'''
    cmd_duration = cmd2_ffprobe_duration.format(**{"in_file": src})
    #print(cmd_duration)
    process3 = subprocess.Popen(cmd_duration, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    avDuration, stderr = process3.communicate()
    err3 = process3.wait()
    #print(avDuration)
    tmDuration = avDuration if avDuration!=b'' else b'0.1'
    return tmDuration

#@pysnooper.snoop(DEBUGLOG)
def uid_txt_restore1(recordDir, args):
    def _restore_txt(uidTxt, recordDir, args):
        avClips = {}
        print("restore uid txt file: "+uidTxt)
        with open(uidTxt, "r+") as fTxt:
            uidLines = fTxt.readlines()
            if len(uidLines)<1:
                return
            for uidl in uidLines:
               uidAVInfo = uidl.split()
               if len(uidAVInfo)<3:
                   return
               elif uidAVInfo[2]=="create":
                   avClips[uidAVInfo[1]] = uidAVInfo[0]
               elif uidAVInfo[2]=="close":
                   avClips.pop(uidAVInfo[1])
               else:
                   pass

            avTime = 0.0
            avDuration = 0.0
            avTMs = []
            avInfos = {}
            for (avFile, avTM) in avClips.items():
               avTime = float(avTM) + float(probe_av_duration(recordDir + '/' + avFile))
               avInfos['{:.3f}'.format(avTime)] = avFile
               avTMs = sorted(avInfos.keys())

            for tm in avTMs:
               #bufLine = tm + " " + avInfos[tm] + " close \n"
               bufLine = '{} {} close \n'.format(tm, avInfos[tm])
               print(bufLine)
               fTxt.write(bufLine)
        #fTxt.close()

    if not os.path.exists(recordDir):
        raise Exception("recordDir {} not exists".format(recordDir))
    elif os.path.isdir(recordDir):
        path_prefix = recordDir
        for root, folders, fnames in os.walk(recordDir):
            for fname in fnames:
                if fname.startswith("uid") and fname.endswith(".txt"):
                    srcFile = os.path.join(root, fname)
                    _restore_txt(srcFile, recordDir, args)
    else:
        raise ("recordDir {} is not a folder")

if __name__ == "__main__":
    if len(sys.argv) < 1:
        usageHelp='''usage: ts_restore.py [-h] [--recordDir INPUT]'''
        print(usageHelp)
        sys.exit(1)

    args = parse_arglist()
    cmdParser = argsParser()
    cmdParser.init(args)
    uid_txt_restore1(args.recordDir, cmdParser)
    print (16*"="+"restore uid_txt finished"+16*"=")
EOF
done
rm -rf ${TXTList}
exit 0
