/*
*  2016.7.26
*  mail:purpledaisy@163.com
*/

// libcore.cpp : 定义 DLL 应用程序的导出函数。
//

#include <time.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

#include <util/base.h>
#include <graphics/vec2.h>
#include <media-io/audio-resampler.h>

//#include "obs.h"
#include <util/config-file.h>

#include <intrin.h>

#include "libcore.h"
//#include "obs-scene.h"

#include "core-info.h"

obs_service_t * service = nullptr;
obs_data_t *h264Settings = nullptr;
obs_data_t *aacSettings = nullptr;

obs_encoder_t *h264Streaming = nullptr;
obs_encoder_t *aacStreaming = nullptr;

obs_output_t* streamOutput = nullptr;

static bool firstStart = true;

static const int cx = 800;
static const int cy = 600;

static DisplayContext CreateDisplay(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	gs_init_data info = {};
	info.cx = rc.right;
	info.cy = rc.bottom;
	info.format = GS_RGBA;
	info.zsformat = GS_ZS_NONE;
	info.window.hwnd = hwnd;

	return obs_display_create(&info);
}



/* --------------------------------------------------- */
static inline void GetScaleAndCenterPos(
	int baseCX, int baseCY, int windowCX, int windowCY,
	int &x, int &y, float &scale)
{
	double windowAspect, baseAspect;
	int newCX, newCY;

	windowAspect = double(windowCX) / double(windowCY);
	baseAspect = double(baseCX) / double(baseCY);

	if (windowAspect > baseAspect) {
		scale = float(windowCY) / float(baseCY);
		newCX = int(double(windowCY) * baseAspect);
		newCY = windowCY;
	}
	else {
		scale = float(windowCX) / float(baseCX);
		newCX = windowCX;
		newCY = int(float(windowCX) / baseAspect);
	}

	x = windowCX / 2 - newCX / 2;
	y = windowCY / 2 - newCY / 2;
}


static void RenderWindow(void *data, uint32_t cx, uint32_t cy)
{
	obs_source_t *src = (obs_source_t *)data;
	uint32_t sourceCX = max(obs_source_get_width(src), 1u);
	uint32_t sourceCY = max(obs_source_get_height(src), 1u);

	int   x, y;
	int   newCX, newCY;
	float scale = 1.0;
	x = 0;
	y = 0;

	GetScaleAndCenterPos(sourceCX, sourceCY, cx, cy, x, y, scale);

	newCX = int(scale * float(sourceCX));
	newCY = int(scale * float(sourceCY));

	gs_viewport_push();
	gs_projection_push();
	gs_ortho(0.0f, float(sourceCX), 0.0f, float(sourceCY),
		-100.0f, 100.0f);
	gs_set_viewport(x, y, newCX, newCY);

	obs_source_video_render(src);

	gs_projection_pop();
	gs_viewport_pop();

	/*
	obs_render_main_view();

	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(cx);
	UNUSED_PARAMETER(cy);
	*/
}

struct StatusInfo
{
	//初始化是否成功
	bool bInit;
	//init video
	bool bInitVideo;
	//obs source win_capture
	SourceContext source_window_capture;
	//dshow_imput source
	SourceContext source_dshow;

	//所有源数据,暂时只用于管理音频源
	//SourceInfoMgr sources;

	//用于管理视频源的位置信息,便于进行调整
	SceneItemsMgr scene_source_mgr;



	SourceContext source_audio_input;
	SourceContext source_audio_output;

	std::vector<std::string> vec_window;
	std::vector<std::string> vec_cam;

	//hwnd
	HWND hwnd;

	//size
	int base_width = 0;
	int base_height = 0;
	int output_width = 0;
	int output_height = 0;

};

//保存数据
static StatusInfo status;

void setWindowsName(std::vector<std::string> vec)
{
	status.vec_window = vec;
}

int GetWinNums()
{
	return status.vec_window.size();
}

const char* GetWinName(int nIndex)
{
	if (nIndex > GetWinNums())
		return NULL;

	return status.vec_window[nIndex].c_str();
}


void SetCamsName(std::vector<std::string> vec)
{
	status.vec_cam = vec;
}


int GetCamNums()
{
	return status.vec_cam.size();
}

const char* GetCamName(int nIndex)
{
	if (nIndex > GetCamNums())
		return NULL;

	return status.vec_cam[nIndex].c_str();
}


bool is_init()
{
	return status.bInit;
}

bool core_init(const char *module_config_path)
{
	status.bInit = obs_startup("en-US", module_config_path, nullptr);
	return status.bInit;
}

bool core_uninit()
{
	obs_shutdown();
	return true;
}

bool core_init_scene()
{
	/* ------------------------------------------------------ */
	/* create scene and add source to scene (twice) */
	SceneContext scene = obs_scene_create("lt 1st scene");
	if (!scene)
		return false;

	obs_sceneitem_t *itemWin = NULL;
	obs_sceneitem_t *itemCam = NULL;
	struct vec2 scale;

	vec2_set(&scale, 1.0f, 1.0f);

	SourceContext sourceWin = status.source_window_capture;
	SourceContext sourceCam = status.source_dshow;

	if (sourceWin != NULL)
	{
		itemWin = obs_scene_add(scene, sourceWin);
		obs_sceneitem_set_scale(itemWin, &scale);
		obs_sceneitem_set_visible(itemWin, true);
	}


	if (sourceCam != NULL)
	{
		itemCam = obs_scene_add(scene, sourceCam);
		obs_sceneitem_set_scale(itemCam, &scale);
		obs_sceneitem_set_visible(itemCam, true);
	}

	/* ------------------------------------------------------ */
	/* set the scene as the primary draw source and go */
	obs_set_output_source(0, obs_scene_get_source(scene));

	/* ------------------------------------------------------ */
	/* create display for output and set the output render callback */

	DisplayContext display = CreateDisplay(status.hwnd);

	if (sourceWin != NULL)
	{
		obs_display_add_draw_callback(display, RenderWindow, sourceWin);
	}

	if (sourceCam != NULL)
	{
		obs_display_add_draw_callback(display, RenderWindow, sourceCam);
	}


	//保留场景信息
	if (sourceWin != NULL)
	{
		status.scene_source_mgr.SetItem(sourceWin, itemWin);
	}

	if (sourceCam != NULL)
	{
		status.scene_source_mgr.SetItem(sourceCam, itemCam);
	}
	

	return true;

}

int core_set_video(HWND hwnd, int output_width, int output_height)
{
	if (!is_init())
		return -1;

	if (!IsWindow(hwnd))
		return -2;

	status.hwnd = hwnd;
	status.output_width = output_width;
	status.output_height = output_height;

	RECT rc;
	GetClientRect(hwnd, &rc);

	struct obs_video_info ovi;
	ovi.adapter = 0;
	ovi.fps_num = 30000;
	ovi.fps_den = 1001;
	ovi.graphics_module = DL_D3D11;
	ovi.output_format = VIDEO_FORMAT_RGBA;

	//ovi.base_width = rc.right;
	//ovi.base_height = rc.bottom;
	ovi.base_width = output_width;
	ovi.base_height = output_height;
	ovi.output_width = output_width;
	ovi.output_height = output_height;

	int rtn = obs_reset_video(&ovi);
	status.bInitVideo = (rtn != 0);
	return rtn;
}

bool core_set_audio()
{
	if (!is_init())
		return false;

	/*set audio video*/
	struct obs_audio_info ai;
	ai.samples_per_sec = 44100;
	ai.speakers = SPEAKERS_STEREO;
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
		if ( strcmp(deviceDesc,TEXT_OUTPUT_AUDIO_DEFAULT_DEVICE_DESC) == 0 )
			status.source_audio_output = source;

		if (strcmp(deviceDesc, TEXT_INPUT_AUDIO_DEFAULT_DEVICE_DESC) == 0 )
			status.source_audio_input = source;
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


void core_audio_set_output_mute(bool muted)
{
	SourceContext src = status.source_audio_output;
	if (src)
		obs_source_set_muted(src,muted);
}

void core_audio_set_output_volume(float volume)
{
	SourceContext src = status.source_audio_output;
	float mul = volume;
	if (mul < 0.0 || mul > 1.0)
	{
		mul = 0;
	}

	if (src)
		obs_source_set_volume(src, mul);
}



void core_audio_set_input_mute(bool muted)
{
	SourceContext src = status.source_audio_input;
	if (src)
		obs_source_set_muted(src, muted);
}

void core_audio_set_input_volume(float volume)
{
	SourceContext src = status.source_audio_input;
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
	//LoadAudioDevice("DesktopAudioDevice1", 1, data);
	//LoadAudioDevice("AuxAudioDevice1", 3, data);
}

void core_add_moudles_path(const char *bin, const char *data)
{
	if (!is_init())
		return;

	//std::cout << "bin:" << bin << std::endl;
	//std::cout << "data:" << data << std::endl;

	obs_add_module_path(bin,data);
}

void core_load_all_modules()
{
	if (!is_init())
		return;

	obs_load_all_modules();
}


bool core_create_win_capture_source()
{
	if (!is_init())
		return false;

	SourceContext source = obs_source_create("window_capture",
		"window_capture source", NULL, nullptr);

	if (!source)
		return false;

	status.source_window_capture = source;

	init_windows_info(source);
	return true;

}

int core_get_win_nums()
{
	return GetWinNums();
}

const char* core_get_win_name(size_t index)
{
	return GetWinName(index);
}

bool init_windows_info(obs_source_t * src)
{
	/*get properties*/
	std::vector<std::string> vecWindow;
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
				const char * str = obs_property_list_item_string(property, i);
				vecWindow.push_back(str);
			}
		}

		obs_property_next(&property);
	}

	setWindowsName(vecWindow);
	return true;
}

bool core_set_win_capture_source(size_t index)
{
	if (index >= status.vec_window.size())
		return false;

	std::string strName = status.vec_window[index];

	obs_data_t *obsData = obs_source_get_settings(status.source_window_capture);


	obs_data_set_string(obsData, "window", strName.c_str());
	obs_source_update(status.source_window_capture, obsData);

	return true;
}



/*
d show setup
*/

bool core_create_dshow_source()
{
	if (!is_init())
		return false;

	SourceContext source = obs_source_create("dshow_input",
		"dshow_input source", NULL, nullptr);

	if (!source)
		return false;

	status.source_dshow = source;

	init_cams_info(source);
	return true;

}

int core_get_cam_nums()
{
	return GetCamNums();
}

const char* core_get_cam_name(size_t index)
{
	return GetCamName(index);
}

bool init_cams_info(obs_source_t * src)
{
	/*get properties*/
	std::vector<std::string> vecCam;
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
				const char * str = obs_property_list_item_string(property, i);
				vecCam.push_back(str);
			}
		}

		obs_property_next(&property);
	}

	SetCamsName(vecCam);

	return true;
}

bool core_set_dshow_source(size_t index)
{
	if (index >= status.vec_cam.size())
		return false;

	std::string strName = status.vec_cam[index];

	obs_data_t *obsData = obs_source_get_settings(status.source_dshow);

	std::cout << strName << std::endl;
	obs_data_set_string(obsData, "video_device_id", strName.c_str());
	obs_source_update(status.source_dshow, obsData);

	return true;
}


/*
service
*/

/*init service*/
bool InitService(const char* rtmp_url)
{
	const char *type = "rtmp_custom";
	//load service

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "server", rtmp_url);

	obs_data_t *hotkey_data = obs_data_create();

	service = obs_service_create(type, "default_service", settings,
		hotkey_data);

	//obs_service_release(service);

	//obs_data_release(hotkey_data);
	//obs_data_release(settings);
	//obs_data_release(data);


	//server
	/*
	service = obs_service_create("rtmp_common", "default_service", nullptr,
	nullptr);
	if (!service)
	return false;
	obs_service_release(service);
	*/
	return true;
}

void InitPrimitives()
{

	obs_enter_graphics();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(0.0f, 1.0f);
	gs_vertex2f(1.0f, 1.0f);
	gs_vertex2f(1.0f, 0.0f);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertbuffer_t* box = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(0.0f, 1.0f);
	gs_vertbuffer_t* boxLeft = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(1.0f, 0.0f);
	gs_vertbuffer_t* boxTop = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(1.0f, 0.0f);
	gs_vertex2f(1.0f, 1.0f);
	gs_vertbuffer_t* boxRight = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(0.0f, 1.0f);
	gs_vertex2f(1.0f, 1.0f);
	gs_vertbuffer_t* boxBottom = gs_render_save();

	gs_render_start(true);
	for (int i = 0; i <= 360; i += (360 / 20)) {
		float pos = RAD(float(i));
		gs_vertex2f(cosf(pos), sinf(pos));
	}
	gs_vertbuffer_t* circle = gs_render_save();

	obs_leave_graphics();
}

void core_init_stream()
{
	streamOutput = obs_output_create("rtmp_output", "simple_stream",
		nullptr, nullptr);
	//obs_output_release(streamOutput);

	h264Settings = obs_data_create();
	aacSettings = obs_data_create();

	h264Streaming = nullptr;
	aacStreaming = nullptr;

	h264Streaming = obs_video_encoder_create("obs_x264",
		"simple_h264_stream", nullptr, nullptr);
	if (!h264Streaming)
		throw "Failed to create h264 streaming encoder (simple output)";
	//obs_encoder_release(h264Streaming);
	aacStreaming = obs_audio_encoder_create("mf_aac", "simple_aac", nullptr, 0, nullptr);
	if (!aacStreaming)
		throw "Failed to create aac streaming encoder (simple output)";
	//if (aacStreaming) {
	//	obs_encoder_release(aacStreaming);
	//	return;
	//}

}

bool core_set_stream_parameters(const char * rtmp_url, int videoBitrate, int audioBitrate)
{
	InitService(rtmp_url);
	//InitPrimitives();

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

	obs_data_release(h264Settings);
	obs_data_release(aacSettings);

	obs_encoder_set_video(h264Streaming, obs_get_video());
	std::cout << "aacstream: " << (int*)aacStreaming << std::endl;
	obs_encoder_set_audio(aacStreaming, obs_get_audio());


	//
	obs_output_set_video_encoder(streamOutput, h264Streaming);
	obs_output_set_audio_encoder(streamOutput, aacStreaming, 0);
	obs_output_set_service(streamOutput, service);


	//取得场景元素位置信息
	refresh_scene_item_pos( status.source_window_capture);
	refresh_scene_item_pos( status.source_dshow );

	return true;
}

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

void set_secene_win_scale(float x_scale, float y_scale)
{
	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(status.source_window_capture);

	if (item == nullptr)
		return;

	if (status.scene_source_mgr.GetItemPos(status.source_window_capture, posInfo))
	{
		struct vec2 scale;
		scale.x = x_scale;
		scale.y = y_scale;

		obs_sceneitem_set_scale(item, &scale);
	}
}


void set_secene_win_pos(int x, int y)
{
	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(status.source_window_capture);

	if (item == nullptr)
		return;

	if (status.scene_source_mgr.GetItemPos(status.source_window_capture, posInfo))
	{
		struct vec2 pos;
		pos.x = x;
		pos.y = y;

		obs_sceneitem_set_pos(item, &pos);
	}
}


void set_secene_cam_scale(float x_scale, float y_scale)
{
	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(status.source_dshow);

	if (item == nullptr)
		return;

	if (status.scene_source_mgr.GetItemPos(status.source_dshow, posInfo))
	{
		struct vec2 scale;
		scale.x = x_scale;
		scale.y = y_scale;

		obs_sceneitem_set_scale(item, &scale);
	}
}


void set_secene_cam_pos(int x, int y)
{
	SceneItem item;
	SceneItemPosInfo posInfo;
	item = status.scene_source_mgr.GetItem(status.source_dshow);

	if (item == nullptr)
		return;

	if (status.scene_source_mgr.GetItemPos(status.source_dshow, posInfo))
	{
		struct vec2 pos;
		pos.x = x;
		pos.y = y;

		obs_sceneitem_set_pos(item, &pos);
	}
}


bool core_start_stream()
{
	bool reconnect = true;
	int retryDelay = 10;
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