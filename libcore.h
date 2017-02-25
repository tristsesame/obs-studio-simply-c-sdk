/*
2016/10/11
基于libobs的封装
提供简单易用的接口函数
core_xx 都是对外的接口函数
其它则为内部使用的函数

2016/11/1
其它函数移至libcore-internal.h

*/

#pragma once

#ifdef LIBCORE_EXPORTS
#define LIBCORE_API __declspec(dllexport)
#else
#define LIBCORE_API __declspec(dllimport)
#endif

#include "Define.h"

/**
* init & uninit
* 初始化/反初始化
*/
extern "C" LIBCORE_API bool core_init(  const char *module_bin_path = nullptr,
										const char *module_data_path = nullptr,
										const char *module_config_path = nullptr);

extern "C" LIBCORE_API void core_uninit();


/**
* scene
* 场景及场景相关的源 相关函数
*/

/** 初始化场景 */
extern "C" LIBCORE_API bool core_create_scene(const char * scene_name);

/** 添加源至场景中 */
extern "C" LIBCORE_API bool core_scene_add_source(const char * scene_name, const char * source_name, bool visible = true);

/** 设置源在场景中显示时的缩放率 */
extern "C" LIBCORE_API void core_set_source_scale(const char * source_name, float x_scale, float y_scale);

/** 设置源在场景中显示的大小 */
extern "C" LIBCORE_API void core_set_source_size(const char * source_name, int width, int height);

/** 设置源在场景中的显示位置 */
extern "C" LIBCORE_API void core_set_source_pos(const char * source_name, int x, int y);

/** 源在场景中 z轴位置up or down */
extern "C" LIBCORE_API void core_set_source_order(const char * source_name, int z_position);

/** 源是否可见 */
extern "C" LIBCORE_API bool core_set_source_visible( const char * source_name, bool visible);

/** 视频源的位置及比例信息 */
extern "C" LIBCORE_API bool core_get_source_pos_info(const char * source_name, int* x, int* y, float* x_scale, float* y_scale);

/** 视频源的位置及大小 */
extern "C" LIBCORE_API bool core_get_source_region(const char * source_name, int* x, int* y, float* width, float* height);

/** 移除场景 todo */
extern "C" LIBCORE_API bool core_scene_remove(const char * source_name);

/** 切换场景 todo */
extern "C" LIBCORE_API bool core_scene_switch_to(const char * source_name);


/**
*audio
*音频相关处理函数
*/
/** 音频初始化设置 */
extern "C" LIBCORE_API bool core_set_audio(int samples_per_sec = 44100, int bit = 32);

/** 音频设备加载，只加载默认的输入输出设备 */
extern "C" LIBCORE_API void core_load_audio();

/** 输出声音是否静音 */
extern "C" LIBCORE_API void core_set_audio_output_mute(bool mute);

/** 输出声音音量调节 range 0.0 - 1.0 */
extern "C" LIBCORE_API void core_set_audio_output_volume(float volume);

/** 输入声音是否静音 */
extern "C" LIBCORE_API void core_set_audio_input_mute(bool mute);

/** 输入声音音量调节 range 0.0 - 1.0 */
extern "C" LIBCORE_API void core_set_audio_input_volume(float volume);

/** 指定当前输入设备 todo*/
extern "C" LIBCORE_API void core_set_audio_input_device(const char* device_name);

/** 指定当前输出设备 todo*/
extern "C" LIBCORE_API void core_set_audio_output_device(const char* device_name);

/** 当前可用的音频输入设备数量 todo */
extern "C" LIBCORE_API int core_get_audio_input_device_num();

/** 取得指定序号对应的输入设备名称 todo */
extern "C" LIBCORE_API const char* core_get_audio_input_device_name(size_t index);

/** 当前可用的音频输出设备数量 todo */
extern "C" LIBCORE_API int core_get_audio_output_device_num();

/** 取得指定序号对应的输出设备名称 todo */
extern "C" LIBCORE_API const char* core_get_audio_output_device_name(size_t index);



/**
* video
*/

/** 设置视频的预览大小及输出大小 */
extern "C" LIBCORE_API int core_set_video(HWND hwnd, int base_width, int base_height, int output_width, int output_height, int fps = 30);

/** 设置预览界面是否显示 */
extern "C" LIBCORE_API bool core_set_preview_display(bool enabled = true);

//源的移除
extern "C" LIBCORE_API bool core_source_remove(const char * source_name);

/** 取得视频元素的实际大小信息 */
extern "C" LIBCORE_API bool core_get_source_base_size(const char * source_name, int* width, int* height);

/** 取得渲染区域的大小 renderWindow中，实际使用的区域*/
//x,y 左上角坐标
//width,height,宽及高度
extern "C" LIBCORE_API bool core_get_preview_render_region(int *x, int *y, int* width, int* height);

/** 设置视频的及输出参数 */
//output_width, output_height输出大小
//fps 帧率
//rate 码流
//encoder_type 编解码器:1软编 2硬编
extern "C" LIBCORE_API int core_reset_video(int output_width, int output_height, int fps, int rate, int encoder_type);

/**
* window source
*/

/** 创建窗口类源 */
extern "C" LIBCORE_API bool core_create_win_capture_source(const char * source_name);

/** 取得可用窗口数量 */
extern "C" LIBCORE_API int core_get_win_num();

/** 取得指定窗口序号对应的窗口名称 */
extern "C" LIBCORE_API const char* core_get_win_name(size_t index);

/** 关联窗口源及其窗口 */
extern "C" LIBCORE_API bool core_set_win_capture_source(const char * win_name, const char * source_name);



/*
cam source
*/

/** 创建摄像头源 */
extern "C" LIBCORE_API bool core_create_cam_source(const char * source_name);

/** 取得可用摄像头数量 */
extern "C" LIBCORE_API int core_get_cam_num();

/** 取得指定序号对应的摄像头名称 */
extern "C" LIBCORE_API const char* core_get_cam_name(size_t index);

/** 关联摄像头源及其摄像头 */
extern "C" LIBCORE_API bool core_set_cam_source(const char * cam_name, const char * source_name);

/** 设置摄像头参数*/
extern "C" LIBCORE_API bool core_set_cam_format(const char * source_name, const char * cam_name, const char * resolution, int fps);

/** 取当前摄像头参数*/
extern "C" LIBCORE_API bool core_get_cam_format(const char * source_name, const char * cam_name, int index, char * resolution, char * fps_list);

/** 取摄像头支持分辨率数量*/
extern "C" LIBCORE_API int core_get_cam_format_num(const char * source_name, const char * cam_name);

/** 取摄像头支持分辨率 todo*/
//extern "C" LIBCORE_API bool core_get_cam_specific_format(const char * cam_name, int index, char * //resolution, int * fps);

/**
* media source
* 多媒体源及其相关函数
*/

/** 创建多媒体文件源 */
extern "C" LIBCORE_API bool core_create_file_source(const char * source_name);

/** 关联多媒体文件源及其文件 */
extern "C" LIBCORE_API bool core_set_file_source(const char *filename, const char * source_name);




/**
* rtmp service
* rtmp流媒体服务相关
*/

/** 初始化流 */
extern "C" LIBCORE_API  bool core_init_stream();

/** 设置流的参数 */
extern "C" LIBCORE_API bool core_set_stream_parameters(const char * rtmp_url = "", int videoBitrate = 2500, int audioBitrate = 160);

/** 推流（开播） */
extern "C" LIBCORE_API bool core_start_stream();

/** 停止推流（停播） */
extern "C" LIBCORE_API void core_stop_stream();

/**设置录制参数 */
extern "C" LIBCORE_API bool core_set_record_parameters(const char * record_path, const char* format = "flv");

/** 开始录制 todo */
extern "C" LIBCORE_API bool core_start_record();

/** 停止录制 todo*/
extern "C" LIBCORE_API void core_stop_record();

/**
* setup
* 系统设置
*/

/** 设置aero效果*/
extern "C" LIBCORE_API void core_set_aero_on(bool enabled);

/** 设置是否显示鼠标*/
extern "C" LIBCORE_API void core_set_mouse_on(bool enabled);

/** 设置是否显示直播参数 todo */
extern "C" LIBCORE_API void core_set_live_status_on(bool enabled);

/** 设置全屏游戏时显示直播参数 todo */
extern "C" LIBCORE_API void core_set_screen_live_status_on(bool enabled);

/** 开启快捷键 todo */
extern "C" LIBCORE_API void core_set_hotkey_on(bool enabled);

/** 设置直播快捷键 todo */
extern "C" LIBCORE_API void core_set_hotkey_streaming(const char * hotkey);

/** 设置录制快捷键 todo */
extern "C" LIBCORE_API void core_set_hotkey_record(const char * hotkey);

/** 设置麦克风快捷键 todo */
extern "C" LIBCORE_API void core_set_hotkey_mic(const char * hotkey);

/** 设置聊天窗口显示快捷键 todo */
extern "C" LIBCORE_API void core_set_hotkey_chat(const char * hotkey);

/** 音频降噪 range 0.0-1.0*/
extern "C" LIBCORE_API bool core_set_audio_denoise(float denoise);

/** 设置视频参数 todo */
extern "C" LIBCORE_API void core_set_video_output_parameter(const char * resolution, int fps, int bitrage, int encode_type);

/** 设置输出编码器设置 todo */
extern "C" LIBCORE_API void core_set_output_encoder(const char * encoder_name);

/** 取得直播流实时码率*/
extern "C" LIBCORE_API bool core_get_stream_bitrate(double *rate);

/** 取得直播流fps*/
extern "C" LIBCORE_API bool core_get_stream_fps(int* fps);

/** 取得丢帧率*/
extern "C" LIBCORE_API bool core_get_stream_dropframes_rate(double *rate);

/** 录制设置 todo */
extern "C" LIBCORE_API bool core_set_stream_record_path(const char * record_save_path);



/**
* draw selected item
* 拖拉时绘制元素的边框
*/

/** 选中元素  editing debuging*/
extern "C" LIBCORE_API bool core_source_select(const char * source_name, bool select = true);