// libcore.cpp : 定义 DLL 应用程序的导出函数。
//

#include <time.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

#include <VersionHelpers.h>

#include <util/base.h>
#include <graphics/vec2.h>
#include <media-io/audio-resampler.h>

//#include "obs.h"
#include <util/config-file.h>

#include <intrin.h>

#include "libcore.h"
#include "libcore-internal.h"

#include "core-info.h"
#include "func.h"
#include "func-draw.h"

#define WIN32_LEAN_AND_MEAN
#include <Dwmapi.h>


//保存数据
static StatusInfo status;

bool is_init()
{
	return status.bInit;
}

bool is_init_stream()
{
	return status.bInitStream;
}

bool core_init( const char *module_bin_path,
				const char *module_data_path,
				const char *module_config_path )
{
	status.bInit = obs_startup("en-US", module_config_path, nullptr);

	add_modules_path(module_bin_path, module_data_path);
	load_all_modules();

	return status.bInit;
}


void core_uninit()
{
	core_set_preview_display(false);
	//std::cout << "uninit 1" << std::endl;
	info("uninit 1");
	core_stop_stream();
	//std::cout << "uninit 3" << std::endl;
	info("uninit 3");
	clear_scene_data();
	//std::cout << "uninit 4" << std::endl;
	info("uninit 4");
	status.source_mgr.DelAllSource();
	//std::cout << "uninit 2" << std::endl;
	info("uninit 2");
	obs_shutdown();
	//std::cout << "uninit 5" << std::endl;
	info("uninit 5");
}

/*
modules
*/
void add_modules_path(const char *bin, const char *data)
{
	if (!is_init())
		return;

	obs_add_module_path(bin, data);
}

void load_all_modules()
{
	if (!is_init())
		return;

	obs_load_all_modules();
}


/*
scene 
*/
void clear_scene_data()
{
	obs_set_output_source(0, nullptr);
	obs_set_output_source(1, nullptr);
	obs_set_output_source(2, nullptr);
	obs_set_output_source(3, nullptr);
	obs_set_output_source(4, nullptr);
	obs_set_output_source(5, nullptr);

	auto cb = [](void *unused, obs_source_t *source)
	{
		obs_source_remove(source);
		UNUSED_PARAMETER(unused);
		return true;
	};

	obs_enum_sources(cb, nullptr);

	return;
}

bool core_create_scene(const char * scene_name)
{
	/* ------------------------------------------------------ */
	/* create scene and add source to scene (twice) */
	SceneContext scene = obs_scene_create(scene_name);
	if (!scene)
		return false;

	/* ------------------------------------------------------ */
	/* set the scene as the primary draw source and go */
	obs_set_output_source(0, obs_scene_get_source(scene));

	status.scene_source_mgr.SetScene(scene);

	return true;
}


/*
bool core_init_scene_sources()
{
	obs_sceneitem_t *item = nullptr;
	SourceContext source = nullptr;

	SceneContext scene = status.scene_source_mgr.GetScene();

	if (scene == nullptr)
		return false;

	vector<SourceContext> vecSrc = status.GetVideoSourceContext();

	for (size_t i = 0; i < vecSrc.size(); i++)
	{
		source = vecSrc[i];
		scene_add_source(source, true);
	}

	return true;
}
*/

/*
// 2016/10/18 liaojf
//接口修订增加 scene_name
//scene_name暂不使用，默认第一次创建的为当前scene
//以下scene函数皆同, 后续再来增加对于多个scene的支持
*/

bool core_scene_add_source(const char * scene_name, const char * source_name, bool visible)
{
	SourceContext source = status.GetSourceObj(source_name);
	return scene_add_source(scene_name,source, visible);
}

bool scene_add_source(const char * scene_name, SourceContext source, bool visible)
{
	SceneContext scene = status.scene_source_mgr.GetCurrentScene();

	if (source == nullptr || scene == nullptr)
		return false;

	obs_sceneitem_t *item = obs_scene_add(scene, source);

	if (item == nullptr)
		return false;

	struct vec2 scale;
	vec2_set(&scale, 1.0f, 1.0f);
	obs_sceneitem_set_scale(item, &scale);
	obs_sceneitem_set_visible(item, visible);

	//保留场景信息
	status.scene_source_mgr.SetItem(source, item);

	return true;
}

bool core_get_source_pos_info(const char * source_name, int* x, int* y, float *x_scale, float* y_scale)
{
	SourceContext src_ctx = status.GetSourceObj(source_name);

	if (src_ctx == nullptr)
		return false;

	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(src_ctx);

	if (item == nullptr)
		return false;

	struct vec2 pos,scale;
	obs_sceneitem_get_pos(item, &pos);

	*x = pos.x;
	*y = pos.y;

	obs_sceneitem_get_scale(item, &scale);
	*x_scale = scale.x;
	*y_scale = scale.y;

	//转换成预览窗口的坐标位置
	*x *= preview_wnd_scale;
	*y *= preview_wnd_scale;

	return true;
}


bool core_get_source_region(const char * source_name, int* x, int* y, float *width, float* height)
{

	SourceContext src_ctx = status.GetSourceObj(source_name);

	if (src_ctx == nullptr)
		return false;

	SceneItem item;

	item = status.scene_source_mgr.GetItem(src_ctx);

	if (item == nullptr)
		return false;

	struct vec2 pos, scale;
	obs_sceneitem_get_pos(item, &pos);

	*x = pos.x;
	*y = pos.y;

	obs_sceneitem_get_scale(item, &scale);
	float x_scale = scale.x;
	float y_scale = scale.y;

	//取原始大小
	//matrix4 boxTransform;
	//obs_sceneitem_get_box_transform(item, &boxTransform);

	//*width = boxTransform.t.x * x_scale;
	//*height = boxTransform.t.y * y_scale;

	*width = obs_source_get_width(src_ctx);
	*height = obs_source_get_height(src_ctx);

	//转换成预览窗口的坐标位置
	*x *= preview_wnd_scale;
	*y *= preview_wnd_scale;

	*width *= x_scale;
	*height *= y_scale;

	*width *= preview_wnd_scale;
	*height *= preview_wnd_scale;

	return true;
}


/*
video
*/

int core_set_video(HWND hwnd, 
	int base_width, int base_height,
	int output_width, int output_height,
	int fps)
{
	if (!is_init())
		return -1;

	if (!IsWindow(hwnd))
		return -1;

	RECT rect;
	GetClientRect(hwnd, &rect);

	int preview_width = rect.right - rect.left ;
	int preview_height = rect.bottom - rect.top;

	status.output_width = output_width;
	status.output_height = output_height;

	struct obs_video_info ovi;
	ovi.adapter = 0;
	ovi.fps_num = fps;
	ovi.fps_den = 1;
	ovi.graphics_module = DL_D3D11;
	ovi.output_format = VIDEO_FORMAT_RGBA;

	ovi.base_width = base_width;
	ovi.base_height = base_height;
	ovi.output_width = output_width;
	ovi.output_height = output_height;

	int rtn = obs_reset_video(&ovi);
	status.bInitVideo = (rtn != 0);

	status.hwnd_main = hwnd;

	//计算scale
	preview_wnd_width = preview_width;
	preview_wnd_height = preview_height;

	GetScaleAndCenterPos(base_width, base_height,
		preview_width - PREVIEW_EDGE_SIZE * 2,
		preview_height - PREVIEW_EDGE_SIZE * 2,
		preview_wnd_x, preview_wnd_y, preview_wnd_scale);

	//std::cout << output_width << "|" << output_height << "|" 
	//	<< preview_width - PREVIEW_EDGE_SIZE * 2 << "|" << preview_height - ////PREVIEW_EDGE_SIZE * 2
	//	<< preview_wnd_x << "|" << preview_wnd_y << "|" << preview_wnd_scale << //std::endl;

	preview_wnd_x += float(PREVIEW_EDGE_SIZE);
	preview_wnd_y += float(PREVIEW_EDGE_SIZE);

	preview_wnd_width = int(preview_wnd_scale * float(ovi.base_width));
	preview_wnd_height = int(preview_wnd_scale * float(ovi.base_height));

	return rtn;
}

bool core_set_audio(int samples_per_sec, int bit)
{
	if (!is_init())
		return false;

	/*set audio video*/
	struct obs_audio_info ai;
	ai.samples_per_sec = samples_per_sec;

	//暂时只支持mono和stereo
	if (bit == 8)
	{
		ai.speakers = SPEAKERS_MONO;
	}
	else
	{
		ai.speakers = SPEAKERS_STEREO;
	}
		
	return obs_reset_audio(&ai);
}


bool LoadAudioDevice(const char *name, int channel, obs_data_t *parent)
{
	if (!is_init())
		return false;

	obs_data_t *data = obs_data_get_obj(parent, name);
	if (!data)
		return false;

	obs_source_t *source = obs_load_source(data);
	if (source) {
		obs_set_output_source(channel, source);
		obs_source_release(source);
	}

	obs_data_release(data);

	return true;
}

//取得本地的音频设备信息
/*-----------------------------------------*/
//是否有相应设备
static inline bool HasAudioDevices(const char *source_id)
{
	const char *output_id = source_id;
	obs_properties_t *props = obs_get_source_properties(output_id);
	size_t count = 0;

	if (!props)
		return false;

	obs_property_t *devices = obs_properties_get(props, "device_id");
	if (devices)
		count = obs_property_list_item_count(devices);

	obs_properties_destroy(props);

	return count != 0;
}

void ResetAudioDevice(const char *sourceId, const char *deviceId,
	const char *deviceDesc, int channel)
{
	obs_source_t *source;
	obs_data_t *settings;
	bool same = false;

	source = obs_get_output_source(channel);

	if (source) {
		settings = obs_source_get_settings(source);
		const char *curId = obs_data_get_string(settings, "device_id");

		same = (strcmp(curId, deviceId) == 0);

		obs_data_release(settings);
		obs_source_release(source);
	}

	if (!same)
		obs_set_output_source(channel, nullptr);

	if (!same && strcmp(deviceId, "disabled") != 0) {
		obs_data_t *settings = obs_data_create();
		obs_data_set_string(settings, "device_id", deviceId);
		source = obs_source_create(sourceId, deviceDesc, settings,
			nullptr);
		obs_data_release(settings);

		obs_set_output_source(channel, source);
		obs_source_release(source);
	}

	if (source)
	{
		if (strcmp(deviceDesc, TEXT_OUTPUT_AUDIO_DEFAULT_DEVICE_DESC) == 0)
		{
			status.source_mgr.AddSource("audio_output", SourceType::Audio, AudioSourceType::Speaker, source);
		}

		if (strcmp(deviceDesc, TEXT_INPUT_AUDIO_DEFAULT_DEVICE_DESC) == 0)
		{
			status.source_mgr.AddSource("audio_input", SourceType::Audio, AudioSourceType::Mic, source);
		}
	}
}


void CreateFirstRunSources()
{
	bool hasDesktopAudio = HasAudioDevices(OUTPUT_AUDIO_SOURCE);
	bool hasInputAudio = HasAudioDevices(INPUT_AUDIO_SOURCE);

	if (hasDesktopAudio)
		ResetAudioDevice(OUTPUT_AUDIO_SOURCE, TEXT_OUTPUT_AUDIO_DEFAULT_DEVICE_ID,
		TEXT_OUTPUT_AUDIO_DEFAULT_DEVICE_DESC, 1);
	if (hasInputAudio)
		ResetAudioDevice(INPUT_AUDIO_SOURCE, TEXT_INPUT_AUDIO_DEFAULT_DEVICE_ID,
		TEXT_INPUT_AUDIO_DEFAULT_DEVICE_DESC, 3);
}

//取得音频设备信息
void get_audio_devices_info()
{
	if (firstStart)
		CreateFirstRunSources();

	firstStart = false;

}


void core_set_audio_output_mute(bool muted)
{
	SourceContext src = status.source_mgr.GetSourceObj("audio_output");
	if (src)
		obs_source_set_muted(src,muted);
}

void core_set_audio_output_volume(float volume)
{
	SourceContext src = status.source_mgr.GetSourceObj("audio_output");
	float mul = volume;
	if (mul < 0.0 || mul > 1.0)
	{
		mul = 0;
	}

	if (src)
		obs_source_set_volume(src, mul);
}



void core_set_audio_input_mute(bool muted)
{
	SourceContext src = status.source_mgr.GetSourceObj("audio_input");
	if (src)
		obs_source_set_muted(src, muted);
}

void core_set_audio_input_volume(float volume)
{
	SourceContext src = status.source_mgr.GetSourceObj("audio_input");
	float mul = volume;
	if (mul < 0.0 || mul > 1.0)
	{
		mul = 0;
	}

	if (src)
		obs_source_set_volume(src, mul);
}

/*-----------------------------------------*/

void core_load_audio()
{
	if (!is_init())
		return;

	get_audio_devices_info();
	//obs_data_t *data = get_devices_info();
	//LoadAudioDevice("DesktopAudioDevice1", 1, data);core_set_audio
	//LoadAudioDevice("AuxAudioDevice1", 3, data);
}

/*
video source func
*/

bool core_set_preview_display(bool enabled)
{
	HWND hwnd = status.hwnd_main;

	if (!IsWindow(hwnd))
		return false;

	DisplayContext display = status.source_mgr.GetDisplay(hwnd);

	if (!display)
		return false;

	//防止调用多次
	if (enabled && !status.bPreviewDisplay )
	{
		InitPrimitives();
		obs_display_add_draw_callback(display, RenderWindow, &status);
		status.bPreviewDisplay = true;
	}

	if ( !enabled && status.bPreviewDisplay )
	{
		obs_display_remove_draw_callback(display, RenderWindow, &status);
		status.bPreviewDisplay = false;
	}

	return true;
}

bool core_source_remove(const char * source_name)
{
	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return false;

	obs_source_remove(source);
	status.source_mgr.DelSource(source_name);

	return true;
}

bool core_get_source_base_size(const char * source_name, int* width, int* height)
{
	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return false;

	*width = obs_source_get_base_width(source);
	*height = obs_source_get_base_height(source);

	return true;
}


bool core_get_preview_render_region(int *x, int *y, int *width, int *height)
{
	*x = preview_wnd_x;
	*y = preview_wnd_y;
	*width = preview_wnd_width;
	*height = preview_wnd_height;

	return true;
}

int core_reset_video(int output_width, int output_height, int fps, int rate, int encoder_type)
{
	if (!is_init() || !is_init_stream() )
		return -1;

	//设置output size和 fps
	obs_video_info ovi;
	obs_get_video_info(&ovi);

	ovi.fps_num = fps;
	ovi.fps_den = 1;
	ovi.graphics_module = DL_D3D11;
	ovi.output_format = VIDEO_FORMAT_RGBA;


	ovi.output_width = output_width;
	ovi.output_height = output_height;

	int rtn = obs_reset_video(&ovi);


	//设置rate
	bool advanced = false;
	bool enforceBitrate = true;
	const char *encoder = "x264";
	const char *presetType = "Preset";
	const char *preset = "veryfast";

	obs_data_set_int(h264Settings, "bitrate", rate);


	obs_service_apply_encoder_settings(service,
		h264Settings, 0);

	video_t *video = obs_get_video();
	enum video_format format = video_output_get_format(video);

	if (format != VIDEO_FORMAT_NV12 && format != VIDEO_FORMAT_I420)
		obs_encoder_set_preferred_video_format(h264Streaming,
		VIDEO_FORMAT_NV12);

	obs_encoder_update(h264Streaming, h264Settings);

	//obs_data_release(h264Settings);

	//obs_encoder_set_video(h264Streaming, obs_get_video());
	//obs_output_set_video_encoder(streamOutput, h264Streaming);
	//obs_output_set_service(streamOutput, service);

	return rtn;
}

/*
windows source
*/

bool core_create_win_capture_source(const char * source_name)
{
	if (!is_init())
		return false;

	SourceContext source = obs_source_create("window_capture",
		"window_capture source", NULL, nullptr);

	if (!source)
		return false;

	status.AddSource(source_name, SourceType::Video, VideoSourceType::Window, source);
	//status.source_window_capture = source;

	init_windows_info(source);
	return true;

}

int core_get_win_num()
{
	return status.GetWinNums();
}

const char* core_get_win_name(size_t index)
{
	return status.GetWinName(index);
}

bool init_windows_info(obs_source_t * src)
{
	/*get properties*/
	std::vector< pair<string, string>  > vecWindow;
	obs_properties_t * ppts = obs_source_properties(src);
	obs_property_t *property = obs_properties_first(ppts);
	bool hasNoProperties = !property;

	while (property) {
		const char        *name = obs_property_name(property);
		obs_property_type type = obs_property_get_type(property);

		if (strcmp(name, "window") == 0)
		{
			size_t  count = obs_property_list_item_count(property);

			for (size_t i = 0; i < count; i++)
			{
				const char * strID = obs_property_list_item_string(property, i);
				const char * strName = obs_property_list_item_name(property, i);
				vecWindow.push_back(make_pair(strName, strID));
			}
		}

		obs_property_next(&property);
	}

	status.setWindowsName(vecWindow);
	return true;
}

//设置鼠标是否捕捉
void set_window_capture_mouse(obs_source_t * source, bool enable)
{
	obs_data_t* settings = obs_source_get_settings(source);

	if (!settings)
		return;

	obs_data_set_bool(settings, "cursor", enable);
	obs_source_update(source, settings);
	obs_data_release(settings);
	return;
}

bool core_set_win_capture_source(const char * win_name, const char * source_name)
{
	std::string strName = win_name;
	if (!IsObjNameExisted(status.vec_window, strName))
		return false;

	std::string strID = status.GetWinID(strName);

	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return false;

	obs_data_t *obsData = obs_source_get_settings(source);

	obs_data_set_string(obsData, "window", strID.c_str());
	obs_source_update(source, obsData);

	return true;
}



/*
d show setup
*/

bool core_create_cam_source(const char * source_name)
{
	if (!is_init())
		return false;

	SourceContext source = obs_source_create("dshow_input",
		"dshow_input source", NULL, nullptr);

	if (!source)
		return false;


	status.AddSource(source_name, SourceType::Video, VideoSourceType::Cam, source);
	//status.source_dshow = source;

	init_cams_info(source);
	return true;

}

int core_get_cam_num()
{
	return status.GetCamNums();
}

const char* core_get_cam_name(size_t index)
{
	return status.GetCamName(index);
}

int core_get_cam_format_num(const char * source_name, const char * cam_name)
{
	if (!is_init())
		return -1;

	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (!source)
		return -1;

	init_cams_set_info(source, cam_name);

	return status.cam_info.vec_resolution.size();
}

//设置摄像头参数
bool core_set_cam_format(const char * source_name, const char * cam_name, const char * resolution, int fps)
{

	std::string strName = cam_name;
	bool findAndSet = false;

	if (!IsObjNameExisted(status.vec_cam, strName))
		return false;

	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return false;

	//obs_data_t *settings = obs_source_get_settings(source);
	int frame_interval = MAKE_DSHOW_FRAME_INTERVAL(fps);

	std::vector< std::pair<string, string> > vecCam;

	vecCam = status.vec_cam;

	//查找device_id
	string device_id;
	for (size_t i = 0; i < vecCam.size(); i++)
	{
		if (vecCam[i].first == strName.c_str())
		{
			device_id = vecCam[i].second;
			break;
		}
	}

	if (device_id.length() == 0)
		return false;


	obs_properties_t * ppts = obs_source_properties(source);
	obs_property_t *property = obs_properties_first(ppts);
	bool hasNoProperties = !property;
	/*get properties*/
	while (property) {

		const char        *name = obs_property_name(property);
		obs_property_type type = obs_property_get_type(property);

		if (strcmp(name, "video_device_id") == 0)
		{
			break;
		}

		obs_property_next(&property);
	}

	//取分辨率
	if (property)
	{
		obs_data_t *settings = obs_data_create();
		obs_data_set_string(settings, "video_device_id", device_id.c_str());
		obs_property_modified(property, settings);
		obs_data_release(settings);
	}

	property = obs_properties_first(ppts);
	hasNoProperties = !property;

	while (property) {
		const char        *name = obs_property_name(property);
		obs_property_type type = obs_property_get_type(property);

		if (strcmp(name, "resolution") == 0)
		{
			size_t  count = obs_property_list_item_count(property);

			for (size_t i = 0; i < count; i++)
			{
				const char * str = obs_property_list_item_string(property, i);
				if (strcmp(str, resolution) == 0)
				{
					obs_data_t *settings = obs_source_get_settings(source);
					obs_data_set_string(settings, "video_device_id", device_id.c_str() );
					obs_data_set_int(settings, "res_type", 1);
					obs_data_set_string(settings, "resolution", resolution);
					obs_data_set_int(settings, "frame_interval", frame_interval);

					obs_source_update(source, settings);
					obs_property_modified(property, settings);
					obs_data_release(settings);
					findAndSet = true;
					break;
				}
			}


		}

		obs_property_next(&property);
	}

	return findAndSet;
}

bool core_get_cam_format(const char * source_name, const char * cam_name, int index, char * resolution, char * fps_list)
{
	if (status.cam_info.cam_name != cam_name)
	{
		core_get_cam_format_num(source_name, cam_name);
	}

	if (status.cam_info.cam_name != cam_name)
	{
		return false;
	}

	if (index >= status.cam_info.vec_resolution.size())
	{
		return false;
	}

	string format = status.cam_info.vec_resolution[index];
	strncpy(resolution, format.c_str(), format.size());

	vector<string> vec_fps;
	if (status.cam_info.vec_vec_fps.size() > index)
	{
		vec_fps = status.cam_info.vec_vec_fps[index];
	}

	string str_fps;
	for (int i = 0; i < vec_fps.size(); i++)
	{
		if (str_fps.size() > 200)
			break;
		if (str_fps.size() > 0)
			str_fps += ",";

		str_fps += vec_fps[i];
	}

	if ( str_fps.size() > 0 )
	{ 
		strncpy(fps_list, str_fps.c_str(), str_fps.size());
	}

	return true;
	
}

bool init_cams_info(obs_source_t * src)
{
	/*get properties*/
	//对应 摄像头 name,id
	std::vector< std::pair<string, string> > vecCam;
	obs_properties_t * ppts = obs_source_properties(src);
	obs_property_t *property = obs_properties_first(ppts);
	bool hasNoProperties = !property;

	while (property) {
		const char        *name = obs_property_name(property);
		obs_property_type type = obs_property_get_type(property);

		if (strcmp(name, "video_device_id") == 0)
		{
			size_t  count = obs_property_list_item_count(property);

			for (size_t i = 0; i < count; i++)
			{
				const char * strId = obs_property_list_item_string(property, i);
				const char * strName = obs_property_list_item_name(property, i);
				vecCam.push_back( std::make_pair(strName,strId) );
			}
		}

		obs_property_next(&property);
	}

	status.SetCamsName(vecCam);

	return true;
}

bool init_cams_set_info(obs_source_t * src, const char * device_name)
{
	CamInfo camInfo;
	camInfo.cam_name = device_name;

	std::string device_id = status.GetCamID(device_name);
	/*get properties*/
	obs_properties_t * ppts = obs_source_properties(src);
	obs_property_t *property = obs_properties_first(ppts);
	bool hasNoProperties = !property;

	while (property) {

		const char        *name = obs_property_name(property);
		obs_property_type type = obs_property_get_type(property);

		if (strcmp(name, "video_device_id") == 0)
		{
			obs_data_t *settings = obs_data_create();
			obs_data_set_string(settings, "video_device_id", device_id.c_str());
			obs_property_modified(property, settings);
			obs_data_release(settings);
			break;
		}

		obs_property_next(&property);
	}

	/*
	property = obs_properties_first(ppts);
	hasNoProperties = !property;
	while (property) {

		const char        *name = obs_property_name(property);
		obs_property_type type = obs_property_get_type(property);

		if (strcmp(name, "res_type") == 0)
		{
			obs_data_t *settings = obs_data_create();
			obs_data_set_string(settings, "video_device_id", device_id.c_str());
			obs_data_set_int(settings, "res_type", 1);
			obs_property_modified(property, settings);
			obs_data_release(settings);
			break;
		}

		obs_property_next(&property);
	}
	*/

	//取分辨率
	/*
	if (property)
	{
		obs_data_t *settings = obs_data_create();
		obs_data_set_string(settings, "video_device_id", device_name);
		obs_property_modified(property, settings);
		obs_data_release(settings);
	}
	*/

	property = obs_properties_first(ppts);
	hasNoProperties = !property;

	while (property) {
		const char        *name = obs_property_name(property);
		obs_property_type type = obs_property_get_type(property);

		if (strcmp(name, "resolution") == 0)
		{
			size_t  count = obs_property_list_item_count(property);

			for (size_t i = 0; i < count; i++)
			{
				const char * str = obs_property_list_item_string(property, i);
				camInfo.vec_resolution.push_back(str);
			}
		}

		obs_property_next(&property);
	}


	for (int k = 0; k < camInfo.vec_resolution.size(); k++)
	{
		vector<string> vec_fps;
		vector<long long>vec_frame_interval;

		//取fps
		property = obs_properties_first(ppts);
		while (property) {
			const char        *name = obs_property_name(property);
			obs_property_type type = obs_property_get_type(property);

			if (strcmp(name, "resolution") == 0)
			{
				obs_data_t *settings = obs_data_create();
				obs_data_set_string(settings, "video_device_id", device_id.c_str());
				obs_data_set_int(settings, "res_type", 1);
				obs_data_set_string(settings, "resolution", camInfo.vec_resolution[k].c_str());
				obs_property_modified(property, settings);
				obs_data_release(settings);
				break;
			}

			obs_property_next(&property);
		}

		property = obs_properties_first(ppts);
		while (property) {
			const char        *name = obs_property_name(property);
			obs_property_type type = obs_property_get_type(property);

			if (strcmp(name, "frame_interval") == 0)
			{
				size_t  count = obs_property_list_item_count(property);
				string strFps;
				for (size_t i = 0; i < count; i++)
				{
					long long frame_interval = obs_property_list_item_int(property, i);
					if (frame_interval < 80000)
					{
						continue;
					}

					int fps = MAKE_DSHOW_FPS(frame_interval);
					strFps = to_string(fps);
					
					vec_frame_interval.push_back(frame_interval);
					vec_fps.push_back(strFps);
				}

			}

			obs_property_next(&property);
		}

		camInfo.vec_vec_fps.push_back(vec_fps);
		camInfo.vec_vec_frame_interval.push_back( vec_frame_interval );

	}

	status.cam_info = camInfo;
	return true;
}

bool core_set_cam_source(const char * cam_name, const char * source_name)
{
	std::string strName = cam_name;

	if (!IsObjNameExisted(status.vec_cam, strName))
		return false;

	std::string strID = status.GetCamID(strName);

	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return false;

	obs_data_t *obsData = obs_source_get_settings(source);

	obs_data_set_string(obsData, "video_device_id", strID.c_str() );
	obs_source_update(source, obsData);

	return true;
}

/*
media source
*/
bool core_create_file_source(const char * source_name)
{
	if (!is_init())
		return false;

	SourceContext source = obs_source_create("ffmpeg_source",
		"media source", NULL, nullptr);

	if (!source)
		return false;

	status.AddSource(source_name, SourceType::Video, VideoSourceType::File, source);
	//status.source_file = source;

	return true;
}

bool core_set_file_source(const char *filename, const char * source_name)
{

	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return false;

	obs_data_t *settings = obs_source_get_settings(source);

	obs_data_set_bool(settings, "is_local_file", true);
	obs_data_set_bool(settings, "looping", false);
	obs_data_set_string(settings, "local_file", filename);
	obs_source_update(source, settings);

	return true;
}

/*
service
*/

/*init service*/
void SetRtmpURL(const char* rtmp_url)
{
	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "server", rtmp_url);

	if (service)
	{
		obs_service_update(service, settings);
	}
}

bool InitService(const char* rtmp_url)
{
	if (service)
		return true;

	const char *type = "rtmp_custom";
	//load service

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "server", rtmp_url);

	obs_data_t *hotkey_data = obs_data_create();

	service = obs_service_create(type, "default_service", settings,
		hotkey_data);

	obs_data_release(hotkey_data);
	obs_data_release(settings);


	if (!service)
		return false;

	return true;
}

bool core_init_stream()
{
	streamOutput = obs_output_create("rtmp_output", "simple_stream",
		nullptr, nullptr);

	h264Settings = obs_data_create();
	aacSettings = obs_data_create();

	h264Streaming = nullptr;
	aacStreaming = nullptr;

	h264Streaming = obs_video_encoder_create("obs_x264",
		"simple_h264_stream", nullptr, nullptr);

	if (!h264Streaming)
	{
		error("Failed to create h264 streaming encoder (simple output)");
		obs_encoder_release(h264Streaming);
		return false;
	}

	if (IsWindows8OrGreater()) {
		aacStreaming = obs_audio_encoder_create("mf_aac", "simple_aac", nullptr, 0, nullptr);
	}
	else
	{
		aacStreaming = obs_audio_encoder_create("ffmpeg_aac", "simple_aac", nullptr, 0, nullptr);
	}

	if (!aacStreaming)
	{
		error("Failed to create aac streaming encoder (simple output)");
		obs_encoder_release(aacStreaming);
		return false;
	}

	//设置录制
	fileOutput = obs_output_create("ffmpeg_muxer",
		"simple_file_output", nullptr, nullptr);

	h264Recording = h264Streaming;
	aacRecording = aacStreaming;

	//obs_encoder_release(h264Streaming);
	//obs_encoder_release(aacStreaming);

	//obs_output_release(streamOutput);

	//obs_output_release(fileOutput);

	status.bInitStream = true;
	return true;

}

bool core_set_stream_parameters(const char * rtmp_url, int videoBitrate, int audioBitrate)
{
	InitService(rtmp_url);

	bool advanced = false;
	bool enforceBitrate = true;
	const char *encoder = "x264";
	const char *presetType = "Preset";
	const char *preset = "veryfast";

	obs_data_set_int(h264Settings, "bitrate", videoBitrate);

	obs_data_set_bool(aacSettings, "cbr", true);
	obs_data_set_int(aacSettings, "bitrate", audioBitrate);

	obs_service_apply_encoder_settings(service,
		h264Settings, 0);

	video_t *video = obs_get_video();
	enum video_format format = video_output_get_format(video);

	if (format != VIDEO_FORMAT_NV12 && format != VIDEO_FORMAT_I420)
		obs_encoder_set_preferred_video_format(h264Streaming,
		VIDEO_FORMAT_NV12);

	obs_encoder_update(h264Streaming, h264Settings);
	obs_encoder_update(aacStreaming, aacSettings);

	//obs_data_release(h264Settings);
	//obs_data_release(aacSettings);

	obs_encoder_set_video(h264Streaming, obs_get_video());
	obs_encoder_set_audio(aacStreaming, obs_get_audio());

	//
	obs_output_set_video_encoder(streamOutput, h264Streaming);
	obs_output_set_audio_encoder(streamOutput, aacStreaming, 0);
	obs_output_set_service(streamOutput, service);

	//set stream output size
	obs_video_info ovi;
	obs_get_video_info(&ovi);
	obs_output_set_preferred_size(streamOutput, ovi.output_width, ovi.output_height);


	//取得场景元素位置信息
	SourceContext source;
	vector<SourceContext> vecSrc = status.GetVideoSourceContext();

	for (size_t i = 0; i < vecSrc.size(); i++)
	{
		source = vecSrc[i];
		refresh_scene_item_pos(source);
	}

	//refresh_scene_item_pos( status.source_window_capture);
	//refresh_scene_item_pos( status.source_dshow );

	return true;
}

/*
场景元素的位置大小调整接口
*/

void refresh_scene_item_pos(SourceContext src)
{
	if (src == nullptr)
		return;
	
	uint32_t        width = obs_source_get_width(src);
	uint32_t        height = obs_source_get_height(src);

	obs_sceneitem_t * item = status.scene_source_mgr.GetItem(src);

	
	if (item == nullptr)
		return;

	struct vec2 pos;
	struct vec2 scale;
	obs_sceneitem_get_pos(item, &pos);
	obs_sceneitem_get_scale(item, &scale);


	//add to scene_source_mgr
	SceneItemPosInfo info;
	info.width = width;
	info.height = height;
	info.x = pos.x;
	info.y = pos.y;
	info.x_scale = scale.x;
	info.y_scale = scale.y;

	status.scene_source_mgr.SetItemPos(src,info);

	//obs_sceneitem_set_scale
	//obs_sceneitem_set_pos
	return;
}

void set_scene_item_scale(SourceContext src_ctx, float x_scale, float y_scale)
{
	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(src_ctx);

	if (item == nullptr)
		return;

	//if (status.scene_source_mgr.GetItemPos(src_ctx, posInfo))
	//{
		struct vec2 scale;
		scale.x = x_scale;
		scale.y = y_scale;

		obs_sceneitem_set_scale(item, &scale);
	//}
}

void set_scene_pos(SourceContext src_ctx, int x, int y)
{
	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(src_ctx);

	if (item == nullptr)
		return;

	//if (status.scene_source_mgr.GetItemPos(src_ctx, posInfo))
	//{
		struct vec2 pos;
		pos.x = x;
		pos.y = y;

		obs_sceneitem_set_pos(item, &pos);
	//}
}

void core_set_source_scale(const char * source_name, float x_scale, float y_scale)
{
	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return;

	set_scene_item_scale(source, x_scale, y_scale);
}


/** 设置源在场景中显示的大小 */
void core_set_source_size(const char * source_name, int width, int height)
{
	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return;

	float x_scale = 0.0;
	float y_scale = 0.0;

	//取得原始大小
	float base_width = (float)obs_source_get_base_width(source);
	float base_height = (float)obs_source_get_base_height(source);

	if (base_width == 0 || base_height == 0)
		return;

	if (preview_wnd_scale <= 0)
		return;

	float output_width = (float)width / preview_wnd_scale;
	float output_height = (float)height / preview_wnd_scale;

	obs_video_info ovi;
	obs_get_video_info(&ovi);

	/*
	if (output_width > ovi.base_width)
		output_width = ovi.base_width;

	if (output_height > ovi.base_height)
		output_height = ovi.base_height;
	*/

	x_scale = output_width / base_width;
	y_scale = output_height / base_height;

	set_scene_item_scale(source, x_scale, y_scale);
}


void core_set_source_pos(const char * source_name, int x, int y)
{

	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return;

	//预览窗口相对位置的x,y转为捕捉大小的x,y
	if (preview_wnd_scale > 0)
	{
		x = x / preview_wnd_scale;
		y = y / preview_wnd_scale;
	}

	/*
	obs_video_info ovi;
	obs_get_video_info(&ovi);

	if (x > (int)ovi.base_width)
		x = ovi.base_width - 10;

	if (y > (int)ovi.base_height)
		y = ovi.base_height - 10;
	*/

	set_scene_pos(source, x, y);
}

/*
位置操作
*/

void set_scene_item_order(SceneItem item, enum obs_order_movement movement)
{
	obs_sceneitem_set_order(item, movement);
}

void set_scene_item_order_up(SourceContext src_ctx)
{
	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(src_ctx);

	if (item == nullptr)
		return;

	set_scene_item_order(item, obs_order_movement::OBS_ORDER_MOVE_UP);
}

void set_scene_item_order_down(SourceContext src_ctx)
{
	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(src_ctx);

	if (item == nullptr)
		return;

	set_scene_item_order(item, obs_order_movement::OBS_ORDER_MOVE_DOWN);
}

void core_set_source_order(const char * source_name, int z_position)
{
	SourceContext source = status.source_mgr.GetSourceObj(source_name);
	int maxOrders = status.source_mgr.GetSourceCount();
	int negMaxOrders = -1 * maxOrders;

	if (source == nullptr)
		return;

	if (z_position > maxOrders)
		z_position = maxOrders;

	if (z_position < negMaxOrders)
		z_position = negMaxOrders;

	if (z_position > 0)
	{ 
		while ( z_position-- )
		set_scene_item_order_up(source);
	}
	else if (z_position < 0)
	{
		while (z_position++)
			set_scene_item_order_down(source);
	}
}


//隐藏

bool set_scene_item_visible(SceneItem item, bool visible)
{
	return obs_sceneitem_set_visible(item, visible);
}

bool set_scene_item_visible_by_src(SourceContext src_ctx, bool visible)
{
	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(src_ctx);

	if (item == nullptr)
		return false;

	return set_scene_item_visible(item, visible);
}

bool core_set_source_visible(const char * source_name, bool visible)
{
	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return false;

	return set_scene_item_visible_by_src(source, visible);
}

/*
开播停播操作
*/

bool core_start_stream()
{
	bool reconnect = true;
	int retryDelay = 0;// 10;
	int maxRetries = 20;
	bool useDelay = true;
	int delaySec = 20;
	bool preserveDelay = true;

	obs_output_set_delay(streamOutput, false ? delaySec : 0,
		preserveDelay ? OBS_OUTPUT_DELAY_PRESERVE : 0);

	obs_output_set_reconnect_settings(streamOutput, maxRetries,
		retryDelay);

	return obs_output_start(streamOutput);
}

void core_stop_stream()
{
	if (obs_output_active(streamOutput)) {
		obs_output_stop(streamOutput);
	}
	return;
}

/*
录制相关
*/
bool core_set_record_parameters(const char * record_path, const char* format)
{
	obs_encoder_set_video(h264Recording, obs_get_video());
	obs_encoder_set_audio(aacRecording, obs_get_audio());


	const char *path;
	const char *recFormat;
	const char *filenameFormat;
	bool noSpace = false;
	bool overwriteIfExists = false;

	path = record_path;
	recFormat = format;
	filenameFormat = "%CCYY-%MM-%DD %hh-%mm-%ss";
	overwriteIfExists = true;
	noSpace = false;

	os_dir_t *dir = path ? os_opendir(path) : nullptr;

	if (!dir) 
	{
		return false;
	}

	os_closedir(dir);

	string strPath;
	strPath += path;

	char lastChar = strPath.back();
	if (lastChar != '/' && lastChar != '\\')
		strPath += "/";

	strPath += GenerateSpecifiedFilename(recFormat, noSpace,
		filenameFormat);

	obs_output_set_video_encoder(fileOutput, h264Recording);
	obs_output_set_audio_encoder(fileOutput, aacRecording, 0);

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings,"path",strPath.c_str());

	obs_output_update(fileOutput, settings);

	obs_data_release(settings);

	return true;
}

bool core_start_record()
{
	if (fileOutput) {
		obs_data_t *settings = obs_output_get_settings(fileOutput);
		const char *path = obs_data_get_string(settings, "path");

		//if (os_file_exists(path) )
		//	return false;

		obs_data_release(settings);
	}

	if (obs_output_start(fileOutput)) {
		return true;
	}

	return false;
}

void core_stop_record()
{
	obs_output_stop(fileOutput);
}

/*
选中元素操作
*/
bool core_source_select(const char * source_name, bool select)
{
	SourceContext source = status.source_mgr.GetSourceObj(source_name);

	if (source == nullptr)
		return false;

	SceneItem item;
	item = status.scene_source_mgr.GetItem(source);

	if (item == nullptr)
		return false;

	obs_sceneitem_select(item, select);

	return true;

}

/*
设置
*/

//aero开关
void core_set_aero_on(bool enabled)
{
	static HRESULT(WINAPI *func)(UINT) = nullptr;
	static bool failed = false;

	if (!func) {
		if (failed) {
			return;
		}

		HMODULE dwm = LoadLibraryW(L"dwmapi");
		if (!dwm) {
			failed = true;
			return;
		}

		func = reinterpret_cast<decltype(func)>(GetProcAddress(dwm,
			"DwmEnableComposition"));
		if (!func) {
			failed = true;
			return;
		}
	}

	func(enabled ? DWM_EC_ENABLECOMPOSITION : DWM_EC_DISABLECOMPOSITION);
}

//是否捕获鼠标
void core_set_mouse_on(bool enabled)
{
	//取得所有的 windows 子类型的 视频源
	vector<SourceContext> vecWindowObj = status.GetVideoSourceContext(VideoSourceType::Window);
	
	for (SourceContext source : vecWindowObj)
	{
		set_window_capture_mouse(source, enabled);
	}
}

bool init_noise_gate_filter()
{
	string id = TEXT_FILTER_NOISE_ID;
	string name = TEXT_FILTER_NOISE;
	obs_source_t *filter = nullptr;

	obs_source_t * filter_source = status.source_mgr.GetSourceObj(name);

	if (filter_source)
	{
		return true;
	}

	filter_source = obs_source_create(id.c_str(), name.c_str(),
		nullptr, nullptr);

	obs_source_t *mic_source = status.source_mgr.GetSourceObj("audio_input");

	if (filter_source && mic_source) {

		status.source_mgr.AddSource(name, SourceType::Filter, FilterSourceType::NoiseGate, filter_source);

		obs_source_filter_add(mic_source, filter_source);
	}

	return filter_source;
}

bool core_set_audio_denoise(float denoise)
{
	string id = TEXT_FILTER_NOISE_ID;
	string name = TEXT_FILTER_NOISE;
	obs_source_t *filter = nullptr;

	obs_source_t * filter_source = status.source_mgr.GetSourceObj(name);

	if (!filter_source)
	{
		init_noise_gate_filter();
	}

	filter_source = status.source_mgr.GetSourceObj(name);

	if (!filter_source)
	{
		return false;
	}

	obs_data_t *settings = obs_source_get_settings(filter_source);

	if (denoise > 1.0f)
		denoise = 1.0f;
	if (denoise < 0.0f)
		denoise = 0.0f;

	double open_threshold = denoise * 96 - 96;
	double close_threshold = open_threshold - 6.0;

	if (close_threshold < -96.0f)
		close_threshold = -96.0f;


	obs_data_set_default_double(settings, S_OPEN_THRESHOLD, open_threshold);
	obs_data_set_default_double(settings, S_CLOSE_THRESHOLD, close_threshold);
	obs_data_set_default_int(settings, S_ATTACK_TIME, 25);
	obs_data_set_default_int(settings, S_HOLD_TIME, 200);
	obs_data_set_default_int(settings, S_RELEASE_TIME, 150);

	obs_source_update(filter_source,settings);

	return true;

}

/*
设置直播状态值，码流，掉帧率
*/

/*
void stream_started()
{
	lastBytesSent = 0;
	lastBytesSentTime = 0;
	beginBytesSend = os_gettime_ns();
}

void stream_stopped()
{
	lastBytesSent = 0;
	lastBytesSentTime = 0;
	beginBytesSend = 0;
}
*/
bool core_get_stream_bitrate(double *rate)
{
	if (!streamOutput)
		return false;

	uint64_t bytesSent = obs_output_get_total_bytes(streamOutput);
	uint64_t bytesSentTime = os_gettime_ns();

	if (bytesSent == 0)
		lastBytesSent = 0;

	if (!(lastBytesSent && lastBytesSentTime))
	{
		lastBytesSent = bytesSent;
		lastBytesSentTime = bytesSentTime;
		return false;
	}


	uint64_t bitsBetween = (bytesSent - lastBytesSent) * 8;

	double timePassed = double(bytesSentTime - lastBytesSentTime) /
		1000000000.0;

	double kbitsPerSec = double(bitsBetween) / timePassed / 1000.0;

	lastBytesSent = bytesSent;
	lastBytesSentTime = bytesSentTime;

	*rate = kbitsPerSec;

	return true;

}

bool core_get_stream_fps(int *fps)
{
	if (!streamOutput)
		return false;


	int totalDropped = obs_output_get_frames_dropped(streamOutput);
	int totalFrames = obs_output_get_total_frames(streamOutput);
	int frames = totalFrames - totalDropped;

	if (lastTotalFrames == 0)
	{
		lastTotalFrames = frames;
		return false;
	}

	*fps = frames - lastTotalFrames;
	lastTotalFrames = frames;

	return true;
}


bool core_get_stream_dropframes_rate(double *rate)
{
	if (!streamOutput)
		return false;

	int totalDropped = obs_output_get_frames_dropped(streamOutput);
	int totalFrames = obs_output_get_total_frames(streamOutput);
	double percent = (double)totalDropped / (double)totalFrames * 100.0;

	if (!totalFrames)
		return false;

	*rate = percent;
	return true;
	
}