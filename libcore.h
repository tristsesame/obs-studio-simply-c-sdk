/*
*  2016.7.26
*  mail:purpledaisy@163.com
*/

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 LIBCORE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// LIBCORE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#pragma once

#ifdef LIBCORE_EXPORTS
#define LIBCORE_API __declspec(dllexport)
#else
#define LIBCORE_API __declspec(dllimport)
#endif

#include "Define.h"

/*
init & uninit
*/

extern "C" LIBCORE_API bool core_init(const char *module_config_path = nullptr);
extern "C" LIBCORE_API bool core_uninit();

/*
scene
*/
extern "C" LIBCORE_API bool core_init_scene();


/*
video
*/

extern "C" LIBCORE_API int core_set_video(HWND hwnd, int output_width, int output_height);


/*
audio
*/
void get_audio_devices_info();

extern "C" LIBCORE_API bool core_set_audio();

extern "C" LIBCORE_API void core_load_audio();

//音量设置

//output
extern "C" LIBCORE_API void core_audio_set_output_mute(bool mute);
//range 0.0 - 1.0
extern "C" LIBCORE_API void core_audio_set_output_volume(float volume);

//input
extern "C" LIBCORE_API void core_audio_set_input_mute(bool mute);
//range 0.0 - 1.0
extern "C" LIBCORE_API void core_audio_set_input_volume(float volume);

/*
modules
*/
extern "C" LIBCORE_API void core_add_moudles_path(const char *bin, const char *data);
extern "C" LIBCORE_API void core_load_all_modules();

/*
window source
*/

extern "C" LIBCORE_API bool core_create_win_capture_source();
extern "C" LIBCORE_API int core_get_win_nums();
extern "C" LIBCORE_API const char* core_get_win_name(size_t index);
extern "C" LIBCORE_API bool core_set_win_capture_source(size_t index);


bool init_windows_info(obs_source_t * src);

/*
cam source
*/
extern "C" LIBCORE_API bool core_create_dshow_source();
extern "C" LIBCORE_API int core_get_cam_nums();
extern "C" LIBCORE_API const char* core_get_cam_name(size_t index);
extern "C" LIBCORE_API bool core_set_dshow_source(size_t index);

bool init_cams_info(obs_source_t * src);


/*
scene
*/

//取得场景元素位置信息
void refresh_scene_item_pos(SourceContext src);

extern "C" LIBCORE_API void set_secene_win_scale(float x_scale, float y_scale);
extern "C" LIBCORE_API void set_secene_win_pos(int x, int y);

extern "C" LIBCORE_API void set_secene_cam_scale(float x_scale, float y_scale);
extern "C" LIBCORE_API void set_secene_cam_pos(int x, int y);


/*
rtmp service
*/
bool InitService(const char* rtmp_url);
void InitPrimitives();
extern "C" LIBCORE_API  void core_init_stream();

extern "C" LIBCORE_API bool core_set_stream_parameters(const char * rtmp_url = "", int videoBitrate = 2500, int audioBitrate = 160);

//开播停播操作
extern "C" LIBCORE_API bool core_start_stream();
extern "C" LIBCORE_API void core_stop_stream();