/*
2016/11/1
内部使用函数定义
*/

#pragma once


#include "Define.h"

/*
log
*/
#define do_log(level, format, ...) \
	blog(level, "[libcore : '%s'] " format, \
			"obs interface ", ##__VA_ARGS__)

#define warn(format, ...)  do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    format, ##__VA_ARGS__)
#define debug(format, ...) do_log(LOG_DEBUG,   format, ##__VA_ARGS__)
#define error(format, ...) do_log(LOG_DEBUG,   format, ##__VA_ARGS__)


/*
init & uninit
*/
void clear_scene_data();


/*
modules
*/
void add_modules_path(const char *bin, const char *data);
void load_all_modules();

/*
scene
*/


//添加源至场景中
bool scene_add_source(const char * scene_name, SourceContext source, bool visible = true);

//取得场景元素位置信息
void refresh_scene_item_pos(SourceContext src);
void set_scene_item_scale(SourceContext src_ctx, float x_scale, float y_scale);
void set_scene_pos(SourceContext src_ctx, int x, int y);



//场景元素z轴位置up or down
void set_scene_item_order(SceneItem item, enum obs_order_movement movement);
void set_scene_item_order_up(SourceContext src_ctx);
void set_scene_item_order_down(SourceContext src_ctx);


//scene item visible
bool set_scene_item_visible(SceneItem item, bool visible);
bool set_scene_item_visible_by_src(SourceContext src_ctx, bool visible);


/*
source
*/


/*
audio
*/
void get_audio_devices_info();



/*
window source
*/

bool init_windows_info(obs_source_t * src);

//设置鼠标是否捕捉
void set_window_capture_mouse(obs_source_t * src, bool enable);

/*
cam source
*/

//摄像头列表
bool init_cams_info(obs_source_t * src);
//摄像头分辨率，fps等
bool init_cams_set_info(obs_source_t *src, const char * device_name);

/*
filter source
*/

//初始化 filter source, noise_gate, 未有则添加
bool init_noise_gate_filter();


/*
rtmp service
*/
bool InitService();
void InitPrimitives();
void SetRtmpURL(const char* rtmp_url);


/*
streaming status
直播状态值
*/
/*
void stream_started();
void stream_stopped();
*/