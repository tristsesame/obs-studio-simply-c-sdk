#pragma once

#include <time.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

#include <util/base.h>
#include <graphics/vec2.h>
#include <media-io/audio-resampler.h>

#include "Define.h"
#include "func-draw.h"

#define MAKE_DSHOW_FPS(interval)                 (10000000LL/(interval))
#define MAKE_DSHOW_FRAME_INTERVAL(fps)		(10000000LL / (fps) )

using namespace std;

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

/*
static void RenderWindow(void *data, uint32_t cx, uint32_t cy)
{
	obs_video_info ovi;
	obs_get_video_info(&ovi);

	//preview_wnd_width = int(preview_wnd_scale * float(ovi.base_width));
	//preview_wnd_height = int(preview_wnd_scale * float(ovi.base_height));

	gs_viewport_push();
	gs_projection_push();

	gs_ortho(0.0f, float(ovi.base_width), 0.0f, float(ovi.base_height),
		-100.0f, 100.0f);

	gs_set_viewport(preview_wnd_x, preview_wnd_y,
		preview_wnd_width, preview_wnd_height);

	//obs_source_video_render(src);
	obs_render_main_view();

	gs_load_vertexbuffer(nullptr);

	gs_projection_pop();
	gs_viewport_pop();

	SceneContext scene = (SceneContext)data;

	DrawSceneEditing(scene);
}
*/

static bool IsObjNameExisted(std::vector< std::pair<string, string> > vec_str, std::string str_name)
{
	for (size_t i = 0; i < vec_str.size(); i++)
	{
		if (str_name == vec_str[i].first)
			return true;
	}
	return false;
}

static bool IsObjNameExisted(std::vector<string> vec_str, std::string str_name)
{
	for (size_t i = 0; i < vec_str.size(); i++)
	{
		if (str_name == vec_str[i])
			return true;
	}
	return false;
}


string GenerateSpecifiedFilename(const char *extension, bool noSpace,
	const char *format)
{
	time_t now = time(0);
	struct tm *cur_time;
	cur_time = localtime(&now);

	const size_t spec_count = 23;
	const char *spec[][2] = {
		{ "%CCYY", "%Y" },
		{ "%YY", "%y" },
		{ "%MM", "%m" },
		{ "%DD", "%d" },
		{ "%hh", "%H" },
		{ "%mm", "%M" },
		{ "%ss", "%S" },
		{ "%%", "%%" },

		{ "%a", "" },
		{ "%A", "" },
		{ "%b", "" },
		{ "%B", "" },
		{ "%d", "" },
		{ "%H", "" },
		{ "%I", "" },
		{ "%m", "" },
		{ "%M", "" },
		{ "%p", "" },
		{ "%S", "" },
		{ "%y", "" },
		{ "%Y", "" },
		{ "%z", "" },
		{ "%Z", "" },
	};

	char convert[128] = {};
	string sf = format;
	string c;
	size_t pos = 0, len;

	while (pos < sf.length()) {
		len = 0;
		for (size_t i = 0; i < spec_count && len == 0; i++) {

			if (sf.find(spec[i][0], pos) == pos) {
				if (strlen(spec[i][1]))
					strftime(convert, sizeof(convert),
					spec[i][1], cur_time);
				else
					strftime(convert, sizeof(convert),
					spec[i][0], cur_time);

				len = strlen(spec[i][0]);

				c = convert;
				if (c.length() && c.find_first_not_of(' ') !=
					std::string::npos)
					sf.replace(pos, len, convert);
			}
		}

		if (len)
			pos += strlen(convert);
		else if (!len && sf.at(pos) == '%')
			sf.erase(pos, 1);
		else
			pos++;
	}

	if (noSpace)
		replace(sf.begin(), sf.end(), ' ', '_');

	sf += '.';
	sf += extension;

	return (sf.length() < 256) ? sf : sf.substr(0, 255);
}