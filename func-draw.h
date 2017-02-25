#pragma once

#include <time.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

#include <algorithm>
#include <cmath>
#include <graphics/vec4.h>
#include <graphics/matrix4.h>

#include <util/platform.h>

#include <util/base.h>
#include <graphics/vec2.h>
#include <media-io/audio-resampler.h>

#include "Define.h"

#define HANDLE_RADIUS     4.0f
#define HANDLE_SEL_RADIUS (HANDLE_RADIUS * 1.5f)

gs_vertbuffer_t *gs_box = nullptr;
gs_vertbuffer_t *gs_boxLeft = nullptr;
gs_vertbuffer_t *gs_boxTop = nullptr;
gs_vertbuffer_t *gs_boxRight = nullptr;
gs_vertbuffer_t *gs_boxBottom = nullptr;
gs_vertbuffer_t *gs_circle = nullptr;

void InitPrimitives()
{

	obs_enter_graphics();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(0.0f, 1.0f);
	gs_vertex2f(1.0f, 1.0f);
	gs_vertex2f(1.0f, 0.0f);
	gs_vertex2f(0.0f, 0.0f);
	gs_box = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(0.0f, 1.0f);
	gs_boxLeft = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(1.0f, 0.0f);
	gs_boxTop = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(1.0f, 0.0f);
	gs_vertex2f(1.0f, 1.0f);
	gs_boxRight = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(0.0f, 1.0f);
	gs_vertex2f(1.0f, 1.0f);
	gs_boxBottom = gs_render_save();

	gs_render_start(true);
	for (int i = 0; i <= 360; i += (360 / 20)) {
		float pos = RAD(float(i));
		gs_vertex2f(cosf(pos), sinf(pos));
	}
	gs_circle = gs_render_save();

	obs_leave_graphics();
}

static void DrawCircleAtPos(float x, float y, matrix4 &matrix,
	float previewScale)
{
	struct vec3 pos;
	vec3_set(&pos, x, y, 0.0f);
	vec3_transform(&pos, &pos, &matrix);
	vec3_mulf(&pos, &pos, previewScale);

	gs_matrix_push();
	gs_matrix_translate(&pos);
	gs_matrix_scale3f(HANDLE_RADIUS, HANDLE_RADIUS, 1.0f);
	gs_draw(GS_LINESTRIP, 0, 0);
	gs_matrix_pop();
}

static inline bool crop_enabled(const obs_sceneitem_crop *crop)
{
	return crop->left > 0 ||
		crop->top > 0 ||
		crop->right > 0 ||
		crop->bottom > 0;
}

static bool CloseFloat(float a, float b, float epsilon = 0.01)
{
	using std::abs;
	return abs(a - b) <= epsilon;
}

//static bool DrawSelectedItem(obs_sceneitem_t *item, int x, int y, int width, int height)
static bool DrawSelectedItem(obs_scene_t *scene,obs_sceneitem_t *item, void *param)
{
	if (!obs_sceneitem_selected(item))
		return true;

	matrix4 boxTransform;
	matrix4 invBoxTransform;
	obs_sceneitem_get_box_transform(item, &boxTransform);
	matrix4_inv(&invBoxTransform, &boxTransform);

	vec3 bounds[] = {
		{ { { 0.f, 0.f, 0.f } } },
		{ { { 1.f, 0.f, 0.f } } },
		{ { { 0.f, 1.f, 0.f } } },
		{ { { 1.f, 1.f, 0.f } } },
	};

	bool visible = std::all_of(std::begin(bounds), std::end(bounds),
		[&](const vec3 &b)
	{
		vec3 pos;
		vec3_transform(&pos, &b, &boxTransform);
		vec3_transform(&pos, &pos, &invBoxTransform);
		return CloseFloat(pos.x, b.x) && CloseFloat(pos.y, b.y);
	});

	if (!visible)
		return true;

	obs_transform_info info;
	obs_sceneitem_get_info(item, &info);

	gs_load_vertexbuffer(gs_circle);

	float previewScale = preview_wnd_scale;

	DrawCircleAtPos(0.0f, 0.0f, boxTransform, previewScale);
	DrawCircleAtPos(0.0f, 1.0f, boxTransform, previewScale);
	DrawCircleAtPos(1.0f, 0.0f, boxTransform, previewScale);
	DrawCircleAtPos(1.0f, 1.0f, boxTransform, previewScale);
	DrawCircleAtPos(0.5f, 0.0f, boxTransform, previewScale);
	DrawCircleAtPos(0.0f, 0.5f, boxTransform, previewScale);
	DrawCircleAtPos(0.5f, 1.0f, boxTransform, previewScale);
	DrawCircleAtPos(1.0f, 0.5f, boxTransform, previewScale);

	gs_matrix_push();
	gs_matrix_scale3f(previewScale, previewScale, 1.0f);
	gs_matrix_mul(&boxTransform);

	obs_sceneitem_crop crop;
	obs_sceneitem_get_crop(item, &crop);

	if (info.bounds_type == OBS_BOUNDS_NONE && crop_enabled(&crop)) {
		vec4 color;
		gs_effect_t *eff = gs_get_effect();
		gs_eparam_t *param = gs_effect_get_param_by_name(eff, "color");

#define DRAW_SIDE(side, vb) \
		if (crop.side > 0) \
			vec4_set(&color, 0.0f, 1.0f, 0.0f, 1.0f); \
				else \
			vec4_set(&color, 1.0f, 0.0f, 0.0f, 1.0f); \
		gs_effect_set_vec4(param, &color); \
		gs_load_vertexbuffer(vb); \
		gs_draw(GS_LINESTRIP, 0, 0);

		DRAW_SIDE(left, gs_boxLeft);
		DRAW_SIDE(top, gs_boxTop);
		DRAW_SIDE(right, gs_boxRight);
		DRAW_SIDE(bottom, gs_boxBottom);
#undef DRAW_SIDE
	}
	else {
		gs_load_vertexbuffer(gs_box);
		gs_draw(GS_LINESTRIP, 0, 0);
	}

	gs_matrix_pop();

	return true;
}

static void DrawSceneEditing(SceneContext scene)
{
	gs_effect_t    *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

	vec4 color;
	vec4_set(&color, 1.0f, 0.0f, 0.0f, 1.0f);
	gs_effect_set_vec4(gs_effect_get_param_by_name(solid, "color"), &color);

	gs_technique_begin(tech);
	gs_technique_begin_pass(tech, 0);

	if (scene)
		obs_scene_enum_items(scene, DrawSelectedItem, nullptr);

	gs_load_vertexbuffer(nullptr);

	gs_technique_end_pass(tech);
	gs_technique_end(tech);
}