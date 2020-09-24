# -*- coding: utf-8 -*-

import sys
sys.path.append(u"../")
#sys.path.append(u"obs-plugins/32bit/")
from win32gui import *
from _ctypes import byref
from ctypes import pythonapi, c_void_p, py_object
from Common.CXCommon import *
from Common.ErrorLogger import *
import ctypes, sys, time, traceback,os
from PySide.QtCore import *
from PySide.QtGui import *

def cur_file_dir():
     #获取脚本路径
     path = sys.path[0]
     #判断为脚本文件还是py2exe编译后的文件，如果是脚本文件，则返回的是脚本的目录，如果是py2exe编译后的文件，则返回的是编译后的文件路径
     if os.path.isdir(path):
         return path
     elif os.path.isfile(path):
         return os.path.dirname(path)
#打印结果
curr_dir_before = cur_file_dir()

class LibCore(object):
    def __init__(self):
        try:
            self.__obs = ctypes.CDLL("libcore.dll")
            ctypes.CDLL("obs-plugins/32bit/obs-ffmpeg.dll")
            ctypes.CDLL("obs-plugins/32bit/rtmp-services.dll")
            #ctypes.CDLL("../../obs-plugins.bak/32bit/win-capture.dll")
            #ctypes.CDLL("../../obs-plugins.bak/32bit/win-dshow.dll")
        except Exception as inst:
            self.__log(inst)
        
    def __log(self,inst):
        errlogger.write(inst, traceback)

    def init(self):
        try:
            return self.__obs.core_init()
        except Exception as inst:
            self.__log(inst)

    def uninit(self):
        try:
            return self.__obs.core_uninit()
        except Exception as inst:
            self.__log(inst)


    def init_scene(self):
        try:
            self.__obs.core_init_scene()
        except Exception as inst:
            self.__log(inst)

    def set_video(self,hwnd,output_width,output_height):
        try:
            pythonapi.PyCObject_AsVoidPtr.restype = c_void_p
            pythonapi.PyCObject_AsVoidPtr.argtypes = [ py_object ]
            hwnd = pythonapi.PyCObject_AsVoidPtr(hwnd)
            print "hwnd:", hwnd
            return self.__obs.core_set_video(hwnd,output_width,output_height)
        except Exception as inst:
            self.__log(inst)

    def set_audio(self):
        try:
            self.__obs.core_set_audio()
        except Exception as inst:
            self.__log(inst)

    def load_audio(self):
        try:
            self.__obs.core_load_audio()
        except Exception as inst:
            self.__log(inst)

    def add_moudles_path(self,bin,data):
        try:
            self.__obs.core_add_moudles_path(bin,data)
        except Exception as inst:
            self.__log(inst)

    def load_all_modules(self):
        try:
            self.__obs.core_load_all_modules()
        except Exception as inst:
            self.__log(inst)

    def create_win_capture_source(self):
        try:
            self.__obs.core_create_win_capture_source.restype = ctypes.c_bool
            return self.__obs.core_create_win_capture_source()
        except Exception as inst:
            self.__log(inst)

    def get_win_nums(self):
        try:
            return self.__obs.core_get_win_nums()
        except Exception as inst:
            self.__log(inst)


    def get_win_name(self,index):
        try:
            self.__obs.core_get_win_name.restype = ctypes.c_char_p
            return self.__obs.core_get_win_name(index)
        except Exception as inst:
            self.__log(inst)


    def set_win_capture_source(self,index):
        try:
            return self.__obs.core_set_win_capture_source(index)
        except Exception as inst:
            self.__log(inst)

    def create_dshow_source(self):
        try:
            self.__obs.core_create_dshow_source.restype = ctypes.c_bool
            return self.__obs.core_create_dshow_source()
        except Exception as inst:
            self.__log(inst)

    def get_cam_nums(self):
        try:
            return self.__obs.core_get_cam_nums()
        except Exception as inst:
            self.__log(inst)


    def get_cam_name(self,index):
        try:
            self.__obs.core_get_cam_name.restype = ctypes.c_char_p
            return self.__obs.core_get_cam_name(index)
        except Exception as inst:
            self.__log(inst)


    def set_dshow_source(self,index):
        try:
            return self.__obs.core_set_dshow_source(index)
        except Exception as inst:
            self.__log(inst)


    def init_stream(self):
        try:
            self.__obs.core_init_stream()
        except Exception as inst:
            self.__log(inst)


    def start_stream(self,rtmp_url,video_bit_rate,audio_bit_rate):
        try:
            self.__obs.core_start_stream(rtmp_url,video_bit_rate,audio_bit_rate)
        except Exception as inst:
            self.__log(inst)


if __name__ == "__main__":
    #set hwnd
    # Create a Qt application
    app = QApplication(sys.argv)
    # Create a Label and show it
    preview = QWidget()


    obs = LibCore()
    print obs.init()
    print GetCurrentFileDir()

    obs.set_audio()
    hwnd = preview.winId()
    k = obs.set_video(hwnd,1920,1080)
    print "set_video:" , k

    #obs.add_moudles_path("F:/GitWork/YBGameDemo/obs-plugins/32bit", "F:/GitWork/testDemo/data/%module%")
    obs.add_moudles_path("F:\\GitWork\\testDemo\\obs-plugins\\32bit", "F:\\GitWork\\testDemo\\data\\obs-plugins\\%module%")
    obs.load_all_modules()

    print "       "
    print "**************************"
    obs.init_stream()
    obs.load_audio()

    print obs.create_win_capture_source()
    k = obs.get_win_nums()
    for i in range(k):
        print i,"->",obs.get_win_name(i)
    print "**************************"
    print obs.set_win_capture_source(0)
    print "**************************"


    print obs.create_dshow_source()
    k = obs.get_cam_nums()
    for i in range(k):
        print i, "->", obs.get_cam_name(i)
    print "**************************"
    print obs.set_dshow_source(2)
    print "**************************"

    obs.init_scene()
    obs.start_stream("rtmp://liveup-rtmp.xxx.xxx/stream/108", 2500, 160)
    #print qtv.GetDefaultAudioDeviceName()

    preview.show()
    # Enter Qt application main loop
    app.exec_()

    print obs.uninit()
    sys.exit()



    
