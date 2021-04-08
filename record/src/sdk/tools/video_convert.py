#!/usr/bin/env python

import os
import re
import sys
import glob
import subprocess
import copy
from optparse import OptionParser

HOME = os.path.dirname(os.path.realpath(__file__))
pathEnv=os.getenv('PATH')
os.environ['PATH']= "%s" %(HOME) + ":" + pathEnv 
dest_fps = 15
default_resolution = False
target_width = 0
target_height = 0
session_tag="new session started"

class Logger:
    def __init__(self, logfile):
        self.terminal = sys.stdout
        self.log = open(logfile, "a")
        sys.stdout = self
    def write(self, message):
        self.terminal.write(message)
        self.log.write(message) 

class AudioClip:
    def __init__(self):
        self.num = 0
        self.filename = []
        self.start_time = []
        self.end_time = []
        self.channels = 1

    def update_audio_info(self, i, stime, etime):
        self.start_time[i] = stime
        self.end_time[i] = etime

    def put_file(self, name):
        if not (name in self.filename):
            self.filename.append(name)
            self.start_time.append(0.0)
            self.end_time.append(0.0)
            self.num = self.num + 1
        return self.filename.index(name)

    def max_length(self):
        return max(self.end_time)

    def print_filename(self, offset_time):
        str = ""
        for i in range(self.num):
            if i > 0:
                len = self.start_time[i] - self.end_time[i-1]
            else:
                len = self.start_time[0] - offset_time
            if len < 0.001:
                len = 0.001
            str = str + ("-f lavfi -t %.3f -i anullsrc=channel_layout=mono:sample_rate=48000 " % len)
            str = str + ("-i %s " % self.filename[i])
        return str

    def print_audio_info(self, i):
        print "Audio Clip %d: %s: start_time=%.3f, end_time=%.3f" % (i, self.filename[i], self.start_time[i], self.end_time[i])

    def filter_valid_audio_items(self):
        print "filter_valid_audio_items"
        invalid_set = []
        invalid_nums = 0
        for i in range(self.num):
            len = self.end_time[i] - self.start_time[i]
            print "Audio Clip len %.3f" % len
            if len < 1.0:
                invalid_set.append(i)
        for i in invalid_set:
            print "Audio Clip remove a invalid item %d: %s: start_time=%.3f, end_time=%.3f" % (i, self.filename[i - invalid_nums], self.start_time[i - invalid_nums], self.end_time[i - invalid_nums])
            del self.filename[i - invalid_nums]
            del self.start_time[i - invalid_nums]
            del self.end_time[i - invalid_nums]
            invalid_nums = invalid_nums + 1
        self.num = self.num - invalid_nums

    def print_ffmpeg(self, output_file, offset_time):
        if self.num >= 1:
            str = "ffmpeg " + self.print_filename(offset_time)
            str = str + "-filter_complex \"concat=n=%d:v=0:a=1[audio]\" " % (self.num * 2)
            str = str + " -map \"[audio]\" -to %f -y -ac %d %s" % (self.max_length() - offset_time, self.channels, output_file)
        elif self.num == 1:
            str = "ffmpeg -i %s -c:a copy %s" % (self.filename[0], output_file)
        else:
            str = ""
        str = str + " 2>&1 | tee -a convert.log"
        print "==============================audio ffmpeg====================================="
        print str
        return str

class RotateClip:
    def __init__(self):
        self.start_time = 0.0
        self.end_time = 0.0
        self.rotation = 0


class VideoClip:
    def __init__(self):
        self.num = 0
        self.filename = []
        self.start_time = []
        self.end_time = []
        self.width = []
        self.height = []
        self.audio_file = ""
        self.audio_start_time = 0.0
        self.audio_end_time = 0.0
        self.max_width = 0
        self.max_height = 0
        self.rotateclip = dict()
        self.repeat = []

    def update_audio_info(self, audio_stime, audio_etime):
        self.audio_start_time = audio_stime
        self.audio_end_time = audio_etime

    def update_video_info(self, i, video_stime, video_etime):
        self.start_time[i] = video_stime
        self.end_time[i] = video_etime

    def put_rotateclip(self, index):
        if not self.rotateclip.has_key(index):
            self.rotateclip[index] = []
    
    def get_rotateclip_len(self, index):
        return len(self.rotateclip[index])
 

    def put_file(self, name):
        if not (name in self.filename):
            self.filename.append(name)
            self.start_time.append(0.0)
            self.end_time.append(0.0)
            self.width.append(0)
            self.height.append(0)
            self.repeat.append(0)
            self.num = self.num + 1
        return self.filename.index(name)

    def filter_valid_video_items(self):
        print "filter_valid_video_items"
        invalid_set = []
        invalid_nums = 0
        rotate_index = 0
        for i in range(self.num):
            len = self.end_time[i] - self.start_time[i]
            print "Video Clip len %.3f" % len
            if len < 1.0:
                invalid_set.append(i)

        for i in invalid_set:
            print "Video Clip remove a invalid item %d: %s: start_time=%.3f, end_time=%.3f" % (i, self.filename[i - invalid_nums], self.start_time[i - invalid_nums], self.end_time[i - invalid_nums])
            del self.filename[i - invalid_nums]
            del self.start_time[i - invalid_nums]
            del self.end_time[i - invalid_nums]
            del self.rotateclip[i]
            invalid_nums = invalid_nums + 1
        self.num = self.num - invalid_nums
        for k,v in self.rotateclip.items():
            print "kk: %d , vv: %.3d-%.3f-%d " % (k, v[0].start_time,v[0].end_time,v[0].rotation)
            tmp = copy.deepcopy(self.rotateclip[k])
            del self.rotateclip[k]
            self.rotateclip[rotate_index] = copy.deepcopy(tmp)
            rotate_index = rotate_index + 1

    def max_resolution(self):
        self.max_width = max(self.width)
        self.max_height = max(self.height)
        return self.max_width, self.max_height

    def target_resolution(self):
        self.max_resolution()
        if default_resolution:
            self.target_width = self.max_width
            self.target_height = self.max_height
        else:
            self.target_width = target_width
            self.target_height = target_height
        return  self.target_width, self.target_height
    
    def max_length(self):
        if self.num > 0:
            return max(max(self.end_time), self.audio_end_time)
        else:
            return self.audio_end_time
    
    def audio_delay_needed(self):
        return self.audio_file != "" and self.audio_start_time > 0.05
    def audio_apad_needed(self):
        return self.audio_file != "" and self.max_length() > self.audio_end_time
    def repeat_needed(self):
        for i in range(len(self.repeat)):
            if self.repeat[i]:
                return True
            else:
                continue
        return False

    def print_filter(self, offset_time):
        str = "" 
        if self.audio_delay_needed():
            audio_delay = int(self.audio_start_time*1000)
            str = "[0]adelay=%d" % audio_delay
            if self.audio_apad_needed():
                str = str + ",apad"
            str = str + "[audio];"
        elif self.audio_apad_needed():
                str = str + "[0]apad[audio];"
        source = "1"
        sink = "out2"
        need_repeat_flag = self.repeat_needed()
        for i in range(self.num):
            totalrc = len(self.rotateclip[i])
            if self.rotateclip[i][0].rotation != 0 or totalrc > 1:
                rcs = self.rotateclip[i]
                sTime = rcs[0].start_time
                if totalrc == 1:
                    str = str + "[%d:v]trim=start=%.3f:end=%.3f,rotate=%d*PI/180,setpts=PTS-STARTPTS[%d_rc];" % \
                            ((i+2), rcs[0].start_time - sTime, rcs[0].end_time - sTime, rcs[0].rotation, (i+2)) 
                else:
                    for j in range(len(self.rotateclip[i])):
                        tmp = "[%d:v]trim=start=%.3f:end=%.3f,rotate=%d*PI/180,setpts=PTS-STARTPTS[%d_rc%d];" % \
                                ((i+2), rcs[j].start_time - sTime, rcs[j].end_time - sTime, rcs[j].rotation, (i+2), j)
                        str = str + tmp
                    for j in range(len(self.rotateclip[i])):
                        tmp = "[%d_rc%d]" % ((i+2), j)
                        str = str + tmp
                    str = str + "concat=n=%d[%d_rc];" % (totalrc, (i+2))
                src = "%d_rc" % (i+2)
            else:
                src = "%d" % (i+2)

            sink = "out%d" % (i+2)
            if i == self.num - 1:
                sink = "video"

            if need_repeat_flag:
                eof_action = "pass"
                if self.repeat[i] == 1:
                    eof_action = "repeat:enable='between(t, %.3f, %.3f)'" % (self.start_time[i], self.start_time[i + 1])
                tmp = "[%s]scale=%dx%d,setpts=PTS-STARTPTS+%.3f/TB[scale%s];[%s][scale%s]overlay=eof_action=%s[%s];" % \
                      (src, self.target_width, self.target_height, self.start_time[i] - offset_time, src, source, src,
                       eof_action, sink)
                str = str + tmp
            else:
                tmp = "[%s]scale=%dx%d,setpts=PTS-STARTPTS+%.3f/TB[scale%s];[%s][scale%s]overlay=eof_action=pass[%s];" % \
                      (src, self.target_width, self.target_height, self.start_time[i] - offset_time, src, source, src,
                       sink)
                str = str + tmp

            source = sink
        return str[:-1]

    def print_filter_with_resolution(self, offset_time, target_width, target_height):
        str = ""
        if self.audio_delay_needed():
            audio_delay = int(self.audio_start_time*1000)
            str = "[0]adelay=%d" % audio_delay
            if self.audio_apad_needed():
                str = str + ",apad"
            str = str + "[audio];"
        elif self.audio_apad_needed():
                str = str + "[0]apad[audio];"
        source = "1"
        sink = "out2"
        need_repeat_flag = self.repeat_needed()
        for i in range(self.num):
            totalrc = len(self.rotateclip[i])
            if self.rotateclip[i][0].rotation != 0 or totalrc > 1:
                rcs = self.rotateclip[i]
                sTime = rcs[0].start_time
                if totalrc == 1:
                    str = str + "[%d:v]trim=start=%.3f:end=%.3f,rotate=%d*PI/180,setpts=PTS-STARTPTS[%d_rc];" % \
                            ((i+2), rcs[0].start_time - sTime, rcs[0].end_time - sTime, rcs[0].rotation, (i+2))
                else:
                    for j in range(len(self.rotateclip[i])):
                        tmp = "[%d:v]trim=start=%.3f:end=%.3f,rotate=%d*PI/180,setpts=PTS-STARTPTS[%d_rc%d];" % \
                                ((i+2), rcs[j].start_time - sTime, rcs[j].end_time - sTime, rcs[j].rotation, (i+2), j)
                        str = str + tmp
                    for j in range(len(self.rotateclip[i])):
                        tmp = "[%d_rc%d]" % ((i+2), j)
                        str = str + tmp
                    str = str + "concat=n=%d[%d_rc];" % (totalrc, (i+2))
                src = "%d_rc" % (i+2)
            else:
                src = "%d" % (i+2)

            sink = "out%d" % (i+2)
            if i == self.num - 1:
                sink = "video"

            if need_repeat_flag:
                eof_action = "pass"
                if self.repeat[i] == 1:
                    eof_action = "repeat:enable='between(t, %.3f, %.3f)'" % (self.start_time[i], self.start_time[i + 1])
                tmp = "[%s]scale=%dx%d,setpts=PTS-STARTPTS+%.3f/TB[scale%s];[%s][scale%s]overlay=eof_action=%s[%s];" % \
                      (src, target_width, target_height, self.start_time[i] - offset_time, src, source, src,
                       eof_action, sink)
                str = str + tmp
            else:
                tmp = "[%s]scale=%dx%d,setpts=PTS-STARTPTS+%.3f/TB[scale%s];[%s][scale%s]overlay=eof_action=pass[%s];" % \
                      (src, target_width, target_height, self.start_time[i] - offset_time, src, source, src, sink)
                str = str + tmp
            source = sink
        return str[:-1]
   
    def print_filename(self):
        str = ""
        for i in range(self.num):
            str = str + ("-i %s " % self.filename[i])
        return str
   
    def print_ffmpeg(self, output_file, offset_time):
        if self.audio_file == "":
            str = "ffmpeg -f lavfi -i anullsrc "
        else:
            str = "ffmpeg -i %s " % self.audio_file
        str = str + "-f lavfi -i \"color=black:s=%dx%d:r=15\" " % (self.target_width, self.target_height)
        str = str + self.print_filename()
        str = str + "-filter_complex \"%s\" " % self.print_filter(offset_time)
        if self.audio_file == "":
            map_option = "-map \"[video]\""
        else:
            if self.audio_delay_needed() or self.audio_apad_needed():
                map_option = "-map \"[audio]\" -map \"[video]\" -c:a aac"
            else:
                map_option = "-map 0:a:0 -map \"[video]\" -c:a copy"
        str = str + " %s -c:v libx264 -r %d -preset veryfast -shortest -to %f -y %s" % (map_option, dest_fps, self.max_length() - offset_time, output_file)
        str = str + " 2>&1 | tee -a convert.log"
        print "=================================video ffmpeg ========================"
        print str
        return str

    def print_ffmpeg_with_resolution(self, output_file, offset_time, target_width, target_height):
        if self.audio_file == "":
            str = "ffmpeg -f lavfi -i anullsrc=channel_layout=mono:sample_rate=48000 "
        else:
            str = "ffmpeg -i %s " % self.audio_file
        str = str + "-f lavfi -i \"color=black:s=%dx%d:r=15\" " % (target_width, target_height)
        if self.num > 0:
            str = str + self.print_filename()
            str = str + "-filter_complex \"%s\" " % self.print_filter_with_resolution(offset_time, target_width, target_height)
        if self.audio_file == "":
            map_option = "-map 0:a:0 -map \"[video]\" -c:a aac "
        elif self.num == 0:
            map_option = ""
        else:
            if self.audio_delay_needed() or self.audio_apad_needed():
                map_option = "-map \"[audio]\" -map \"[video]\" -c:a aac"
            else:
                map_option = "-map 0:a:0 -map \"[video]\" -c:a copy"
        str = str + " %s -c:v libx264 -r %d -preset veryfast -shortest -to %f " % (map_option, dest_fps, self.max_length() - offset_time)

        str = str + " -y %s" % output_file
        str = str + " 2>&1 | tee -a convert.log"
        print "=================================video ffmpeg ========================"
        print str
        return str

    def print_audio_info(self):
        print "Audio Clip: %s: start_time=%.3f, end_time=%.3f" % (self.audio_file, self.audio_start_time, self.audio_end_time)
    
    def print_video_info(self, i):
        print "Video Clip %d: %s: start_time=%.3f, end_time=%.3f, width=%d, height=%d" % \
            (i, self.filename[i], self.start_time[i], self.end_time[i], self.width[i], self.height[i])
    
def filter_pat(f, pat):
    regex=re.compile(pat)
    lines=f.readlines()
    detected=False
    for line in lines:
        if regex.match(line):
            lines.remove(line)
            detected=True

        # remove line that corresponding file size is 0
        items = line.split(" ");

    return (detected,lines)
    
class UserInfoPerMergedTxt:
    def __init__(self, ts, path):
        self.detect_start = False
        self.last_ts = ts
        self.path = path

    def detectStartedTxt():#detect 0.000
        return self.detect_start



class UserInfoMergedTxtDict:
    def __init__(self, uid, index, ts, path):
        self.uid = uid
        self.mergedTxtDict = dict()
        self.mergedTxtDict[index] = UserInfoPerMergedTxt(ts, path)

    def update(self, uid, index, ts, path):
        if uid != self.uid:
            print("Error:uid mismatch:%s:%s" %(uid, self.uid))

        if not self.mergedTxtDict.has_key(index):
            self.mergedTxtDict[index] = UserInfoPerMergedTxt(ts, path)
        else:
            print("Error:already has mergedTxtDict[%d]:%s" %(index, mergedTxtDict[index]))

    def hasTheMergedIndex(self, index):
        return self.mergedTxtDict.has_key(index)
                    

    def outputMergedUserInfoByIndex(self):
        for i in mergedTxtDict[index]:
            print("uid:%s, data:%s" %(self.uid, self.mergedTxtDict[i]))

class UserAvClip:
    def __init__(self, start_time, end_time, uid_file, opt):
        self.start_time = start_time
        self.end_time = end_time
        self.uid_file = uid_file
        self.option = opt
        self.max_width = 0
        self.max_height = 0
        self.uid = 0

    def parse(self):

        self.uid = os.path.splitext(self.uid_file)[0][4:]
        print "UID:" + self.uid

        self.clip = VideoClip()
        self.audio_clip = AudioClip()
        with open(self.uid_file) as f:
            av_1st_stime = [False]
            (hasSessionDetect, lines) = filter_pat(f, session_tag)
            print("DEBUG:%s:%s" % (hasSessionDetect, lines))
            for line in lines:
                items = line.split(" ")
                if os.path.getsize(items[1]) == 0:
                    continue;
                # audio file
                if items[1][-3:] == "aac":

                    if self.option == 3:
                        continue
                    index = self.audio_clip.put_file(items[1])
                    if items[2] == "create":
                        if not av_1st_stime[0]:
                            av_1st_stime.append(float(items[0]))  # mark it.

                        self.audio_clip.start_time[index] = float(items[0])
                    elif items[2] == "close":
                        self.audio_clip.end_time[index] = float(items[0])
                    elif items[2] == "info":
                        if int(items[3][9:]) > self.audio_clip.channels:
                            self.audio_clip.channels = int(items[3][9:])

                # video file
                if items[1][-3:] == "mp4":
                    if self.option == 2:
                        continue
                    index = self.clip.put_file(items[1])
                    self.clip.put_rotateclip(index)
                    if items[2] == "create":
                        if not av_1st_stime[0]:
                            av_1st_stime.append(float(items[0]))  # mark it.

                        self.clip.start_time[index] = float(items[0])
                        self.clip.rotateclip[index].append(RotateClip())
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].start_time = float(items[0])
                    elif items[2] == "info":
                        self.clip.start_time[index] = float(items[0])
                        self.clip.width[index] = int(items[3][6:])
                        self.clip.height[index] = int(items[4][7:])
                        rotation = int(items[5][9:])
                        # if rotation == 90 or rotation == 270:
                        #    clip.width[index], clip.height[index] = clip.height[index], clip.width[index]
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].rotation = rotation

                    elif items[2] == "rotate":
                        rotation = int(items[3][9:])
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].end_time = float(items[0])
                        self.clip.rotateclip[index].append(RotateClip())
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].start_time = float(items[0])
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].rotation = rotation

                    elif items[2] == "close":
                        self.clip.end_time[index] = float(items[0])
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].end_time = float(items[0])

                    if len(items) >= 4 and items[3].find('codecswitch') != -1:
                        self.clip.repeat[index - 1] = 1
                # video file
                if items[1][-4:] == "webm":
                    if self.option == 2:
                        continue
                    index = self.clip.put_file(items[1])
                    self.clip.put_rotateclip(index)
                    if items[2] == "create":
                        if not av_1st_stime[0]:
                            av_1st_stime.append(float(items[0]))  # mark it.
                        self.clip.start_time[index] = float(items[0])
                        self.clip.rotateclip[index].append(RotateClip())
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].start_time = float(items[0])
                    elif items[2] == "info":
                        self.clip.start_time[index] = float(items[0])
                        self.clip.width[index] = int(items[3][6:])
                        self.clip.height[index] = int(items[4][7:])
                        rotation = int(items[5][9:])
                        # if rotation == 90 or rotation == 270:
                        #    clip.width[index], clip.height[index] = clip.height[index], clip.width[index]
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].rotation = rotation
                    elif items[2] == "rotate":
                        rotation = int(items[3][9:])
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].end_time = float(items[0])
                        self.clip.rotateclip[index].append(RotateClip())
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].start_time = float(items[0])
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].rotation = rotation
                    elif items[2] == "close":
                        self.clip.end_time[index] = float(items[0])
                        self.clip.rotateclip[index][self.clip.get_rotateclip_len(index) - 1].end_time = float(items[0])

            if not self.option:
                self.clip.update_audio_info(self.clip.audio_start_time - av_1st_stime[1],
                                            self.clip.audio_end_time - av_1st_stime[1])

                for i in range(self.audio_clip.num):
                    self.audio_clip.update_audio_info(i, self.audio_clip.start_time[i] - av_1st_stime[1],
                                                      self.audio_clip.end_time[i] - av_1st_stime[1])

                for i in range(self.clip.num):
                    self.clip.update_video_info(i, self.clip.start_time[i] - av_1st_stime[1],
                                                self.clip.end_time[i] - av_1st_stime[1])

            #remove invalid audio/vedio items
            self.audio_clip.filter_valid_audio_items()
            self.clip.filter_valid_video_items()

            self.clip.print_audio_info()
            for i in range(self.audio_clip.num):
                self.audio_clip.print_audio_info(i)
            for i in range(self.clip.num):
                self.clip.print_video_info(i)

            if self.clip.num != 0:
                self.clip.max_resolution()
                self.max_width = self.clip.max_width
                self.max_height = self.clip.max_height

    def convert(self, suffix, offset_time, target_width, target_height, use_quality):
        child_env = os.environ.copy()
        if self.audio_clip.num == 0 and self.clip.num == 0:
            return ""

        if self.audio_clip.num >= 1:
            print "Generate Audio File"
            tmp_audio = self.uid + "_tmp.m4a"
            command = self.audio_clip.print_ffmpeg(tmp_audio, offset_time)
            self.clip.audio_file = tmp_audio
            self.clip.audio_start_time = 0.0
            self.clip.audio_end_time = self.audio_clip.max_length()
            print command
            subprocess.Popen(command, shell=True, env=child_env).communicate()

        if target_width > 0 and target_height > 0:
            print "Generate MP4 file:"
            print "Output resolution:", target_width, target_height
            output_file = self.uid + suffix + "_qa.mp4"
            return_file = self.uid + suffix + ".mp4"
            command = self.clip.print_ffmpeg_with_resolution(output_file, offset_time, target_width, target_height)

            print command
            subprocess.Popen(command, shell=True, env=child_env).communicate()
            if use_quality:
                qacmd = "ffmpeg -i " + output_file + " -q 0 " + return_file + " 2>&1 | tee -a convert.log"
                subprocess.Popen(qacmd, shell=True, env=child_env).communicate()
                os.system('rm -f ' + output_file)
            else:
                mvcmd = "mv " + output_file + " " + return_file
                subprocess.Popen(mvcmd, shell=True, env=child_env).communicate()
            print "\n\n"
        else:
            return_file = self.uid + ".m4a"
            command = "cp -f %s %s" % (self.clip.audio_file, return_file)
            os.system(command)
        # remove tmp files
        os.system('rm -f *_tmp.m4a')
        return return_file

class UserAv:
    def __init__(self, uid):
        self.uid = uid
        self.avClips = dict()
        self.num = 0
        self.widths = []
        self.heights = []
        self.useQuality = False

    def addClip(self, clip):
        self.avClips[self.num] = clip
        self.num += 1
        self.widths.append(clip.max_width)
        self.heights.append(clip.max_height)
        if not self.useQuality:
            if clip.max_width == 0 and clip.max_height == 0:
                self.useQuality = True

    def getMaxWidth(self):
        if default_resolution:
            return max(self.widths)
        else:
            return target_width

    def getMaxHeight(self):
        if default_resolution:
            return max(self.heights)
        else:
            return target_height


def UidFileConvert(uid_file, suffix, option, offset_time):    
        print "Offset_time : " + str(offset_time)
        child_env = os.environ.copy()

        uid = os.path.splitext(uid_file)[0][4:]
        print "UID:"+uid
            
        clip = VideoClip()
        audio_clip = AudioClip()
        with open(uid_file) as f:
            av_1st_stime = [False]
            (hasSessionDetect, lines)=filter_pat(f, session_tag)
            print("DEBUG:%s:%s" %(hasSessionDetect, lines))
            for line in lines:
                items = line.split(" ")
                if os.path.getsize(items[1]) == 0:
                    continue;
                #audio file
                if items[1][-3:] == "aac":

                    if option == 3:
                        continue
                    index = audio_clip.put_file(items[1])
                    if items[2] == "create":
                        if not av_1st_stime[0]:
                            av_1st_stime.append(float(items[0])) #mark it.

                        audio_clip.start_time[index] = float(items[0])
                    elif items[2] == "close":
                        audio_clip.end_time[index] = float(items[0])
                    elif items[2] == "info":
                        if int(items[3][9:]) > audio_clip.channels:
                            audio_clip.channels = int(items[3][9:])
                        
                #video file
                if items[1][-3:] == "mp4":
                    if option == 2:
                        continue
                    index = clip.put_file(items[1])
                    clip.put_rotateclip(index)
                    if items[2] == "create":
                        if not av_1st_stime[0]:
                            av_1st_stime.append(float(items[0])) #mark it.

                        clip.start_time[index] = float(items[0])
                        clip.rotateclip[index].append(RotateClip())
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].start_time = float(items[0])
                    elif items[2] == "info":
                        clip.start_time[index] = float(items[0])
                        clip.width[index] = int(items[3][6:])
                        clip.height[index] = int(items[4][7:])
                        rotation = int(items[5][9:])
                        #if rotation == 90 or rotation == 270:
                        #    clip.width[index], clip.height[index] = clip.height[index], clip.width[index]
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].rotation = rotation

                    elif items[2] == "rotate":
                        rotation = int(items[3][9:])
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].end_time = float(items[0])
                        clip.rotateclip[index].append(RotateClip())
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].start_time = float(items[0])
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].rotation = rotation

                    elif items[2] == "close":
                        clip.end_time[index] = float(items[0])
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].end_time = float(items[0])
                    if len(items) >= 4 and items[3].find('codecswitch') != -1:
                        clip.repeat[index -1] = 1
                #video file
                if items[1][-4:] == "webm":
                    if option == 2:
                        continue
                    index = clip.put_file(items[1])
                    clip.put_rotateclip(index)
                    if items[2] == "create":
                        if not av_1st_stime[0]:
                            av_1st_stime.append(float(items[0])) #mark it.
                        clip.start_time[index] = float(items[0])
                        clip.rotateclip[index].append(RotateClip())
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].start_time = float(items[0])
                    elif items[2] == "info":
                        clip.start_time[index] = float(items[0])
                        clip.width[index] = int(items[3][6:])
                        clip.height[index] = int(items[4][7:])
                        rotation = int(items[5][9:])
                        #if rotation == 90 or rotation == 270:
                        #    clip.width[index], clip.height[index] = clip.height[index], clip.width[index]
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].rotation = rotation
                    elif items[2] == "rotate":
                        rotation = int(items[3][9:])
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].end_time = float(items[0])
                        clip.rotateclip[index].append(RotateClip())
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].start_time = float(items[0])
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].rotation = rotation
                    elif items[2] == "close":
                        clip.end_time[index] = float(items[0])
                        clip.rotateclip[index][clip.get_rotateclip_len(index) - 1].end_time = float(items[0])

            if not option:
                clip.update_audio_info(clip.audio_start_time - av_1st_stime[1],
                        clip.audio_end_time - av_1st_stime[1])

                for i in range(audio_clip.num):
                    audio_clip.update_audio_info(i, audio_clip.start_time[i] - av_1st_stime[1],
                            audio_clip.end_time[i] - av_1st_stime[1])

                for i in range(clip.num):
                    clip.update_video_info(i, clip.start_time[i] - av_1st_stime[1],
                            clip.end_time[i] - av_1st_stime[1])

            #remove invalid audio/video items
            audio_clip.filter_valid_audio_items()
            clip.filter_valid_video_items()

            clip.print_audio_info()
            for i in range(audio_clip.num):
                audio_clip.print_audio_info(i)
            for i in range(clip.num):
                clip.print_video_info(i)

        if audio_clip.num == 0 and clip.num == 0:
            return ""
                    
        if audio_clip.num >= 1:
                print "Generate Audio File"
                tmp_audio = uid+"_tmp.m4a"
                command = audio_clip.print_ffmpeg(tmp_audio, offset_time)
                clip.audio_file = tmp_audio
                clip.audio_start_time = 0.0
                clip.audio_end_time = audio_clip.max_length()
                print command
                subprocess.Popen(command, shell=True, env=child_env).communicate()
        elif audio_clip.num == 1:
                clip.audio_file = audio_clip.filename[0]
                clip.audio_start_time = audio_clip.start_time[0]
                clip.audio_end_time = audio_clip.end_time[0]
        if clip.num > 0:
                print "Generate MP4 file:"
                print "Output resolution:", clip.target_resolution()
                output_file = uid + suffix + ".mp4"
                command =  clip.print_ffmpeg(output_file, offset_time)
        else:
                tmp_audio = uid+"_tmp.m4a"
                output_file = uid+".m4a"
                if audio_clip.num >= 1:
                    command = "mv %s %s" % (tmp_audio, output_file)
                elif audio_clip.num == 1:
                    command = "ffmpeg -i %s -c:a copy -y %s" % (clip.audio_file, output_file)
        print command
        subprocess.Popen(command, shell=True, env=child_env).communicate()
        print "\n\n"
        #remove tmp files
        os.system('rm -f *_tmp.m4a')
        return output_file
    

def SessionConvert(folder_name, opt, saving):
    child_env = os.environ.copy()
    if not os.path.isdir(folder_name):
        print "Folder " + folder_name + " does not exist"
        return

    os.chdir(folder_name)
    os.system('rm -f *_merge.txt')
    all_uid_file = sorted(glob.glob("uid_*.txt"))
    if opt == 0 :
        for uid_file in all_uid_file:
            UidFileConvert(uid_file, "_av", 0, 0)

        f = open("convert-done.txt", "w")
        f.close()
        return

    dict_uid = dict()
    dict_uid_index = dict()
    for uid_file in all_uid_file:
        uid = uid_file.split("_")[1];

        start_ts = -1.0
        end_ts = start_ts
        detectedOncePerTxt = False 
        key = uid
        with open(uid_file) as f:
            (hasSessionDetect, lines) = filter_pat(f, session_tag)
        
            for line in lines:
                items = line.split(" ")
                if not detectedOncePerTxt and (hasSessionDetect or float(items[0]) == 0.000): 
                    if not dict_uid_index.has_key(uid):
                        dict_uid_index[uid] = 0
                    else:
                        dict_uid_index[uid] += 1
                    detectedOncePerTxt =True
                        
                key = uid + "_" + str(dict_uid_index[uid])

                if not dict_uid.has_key(key):
                    dict_uid[key] = UserAv(key)

                if os.path.getsize(items[1]) == 0:
                    continue;

                if opt == 2 and items[1].split(".")[1] != "aac":  # aac
                    continue
                elif opt == 3 and items[1].split(".")[1] == "aac":  # mp4
                    continue

                if start_ts < 0:
                    if float(items[0]) > 0:
                        start_ts = float(items[0])
                    else:
                        start_ts = 0
                items[0] = "%.3f" % (float(items[0])) 

                end_ts = float(items[0])
                
        clip = UserAvClip(start_ts, end_ts, uid_file, opt)
        clip.parse()
        dict_uid[key].addClip(clip)
   
    temp_files = []
    for index in dict_uid.keys():
        usr = dict_uid[index]
        print "Merge for uid : " + usr.uid
        concat_file = index + "_filelist.txt"
        file = open(concat_file, "a")
        last_ts = 0
        merged_index = 0
        channels = 0
        for i in usr.avClips.keys():
            clip = usr.avClips[i]
            if clip.audio_clip.channels > channels:
                channels = clip.audio_clip.channels
            if saving:
                last_ts = clip.start_time
            output_file = clip.convert("_" + str(merged_index) + "_av", last_ts, usr.getMaxWidth(), usr.getMaxHeight(), usr.useQuality)
            if output_file != "":
                file.write("file \'" + output_file + "'\n")
                merged_index += 1
                if not saving:
                    last_ts = clip.end_time
                temp_files.append(output_file)

        file.close()
        if merged_index == 0:
            continue

        target_file_name = usr.uid + "_merge"
        if opt == 1:
            target_file_name += "_av.mp4"
        elif opt == 2:
            target_file_name += ".m4a"
        else:
            target_file_name += ".mp4"

        command = "ffmpeg -f concat -i " + concat_file ;
        if channels > 0:
            command += " -vc copy -ac " + str(channels) + " "
        else:
            command +=  " -c copy "

        command += target_file_name
        print command
        subprocess.Popen(command, shell=True, env=child_env).communicate()

    os.system('rm -f *_filelist.txt')
    for tem_file in temp_files:
        os.system('rm -f ' + tem_file)
    
    #write a convert done file
    f = open("convert-done.txt", "w")
    f.close()
    return    

parser = OptionParser()
parser.add_option("-f", "--folder", type="string", dest="folder", help = "Convert folder", default = "")
parser.add_option("-m", "--mode", type="int", dest="mode", help = "Convert merge mode, \
[0: txt merge A/V(Default); 1: uid merge A/V; 2: uid merge audio; 3: uid merge video]", default = 0)
parser.add_option("-p", "--fps", type="int", dest="fps", help = "Convert fps, default 15", default = 15)
parser.add_option("-s", "--saving", action="store_true", dest="saving", help = "Convert Do not time sync", default = False)
parser.add_option("-r", "--resolution", type="int", dest="resolution", nargs=2, help = "Specific resolution to convert '-r width height' \nEg:'-r 640 360'", default=(0,0))

(options, args) = parser.parse_args()
if not options.folder:
    parser.print_help()
    parser.error("Not set folder")
if options.mode < 0 or options.mode > 3:
    parser.error("Invalid mode")
if options.fps <= 0:
    parser.error("Invalid fps")

if options.resolution[0] < 0 or options.resolution[1] < 0:
    parser.error("Invalid resolution width/height")
elif options.resolution[0] == 0 and options.resolution[1] == 0:
    default_resolution = True
else:
    target_width = options.resolution[0]
    target_height = options.resolution[1]

os.system("rm -f " + options.folder + "/convert.log")
Logger(options.folder + "/convert.log")

if options.fps < 5:
    print "fps < 5, set to 5"
    dest_fps = 5
elif options.fps > 120:
    print "fps > 120, set to 120"
    dest_fps = 120
else:
    dest_fps = options.fps

SessionConvert(options.folder, options.mode, options.saving)

