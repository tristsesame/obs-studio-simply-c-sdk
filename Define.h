/*
*  2016.7.26
*  mail:purpledaisy@163.com
*/

#pragma once

#include "obs.h"

#include <intrin.h>


#define TEXT_WINDOW_CAPTURE obs_module_text("WindowCapture")
#define TEXT_WINDOW         obs_module_text("WindowCapture.Window")
#define TEXT_MATCH_PRIORITY obs_module_text("WindowCapture.Priority")
#define TEXT_MATCH_TITLE    obs_module_text("WindowCapture.Priority.Title")
#define TEXT_MATCH_CLASS    obs_module_text("WindowCapture.Priority.Class")
#define TEXT_MATCH_EXE      obs_module_text("WindowCapture.Priority.Exe")
#define TEXT_CAPTURE_CURSOR obs_module_text("CaptureCursor")
#define TEXT_COMPATIBILITY  obs_module_text("Compatibility")

#define INPUT_AUDIO_SOURCE  "wasapi_input_capture"
#define OUTPUT_AUDIO_SOURCE "wasapi_output_capture"

#define TEXT_OUTPUT_AUDIO_DEFAULT_DEVICE_ID "default"
#define TEXT_OUTPUT_AUDIO_DEFAULT_DEVICE_DESC "DesktopAudioDevice1"
#define TEXT_INPUT_AUDIO_DEFAULT_DEVICE_ID "default"
#define TEXT_INPUT_AUDIO_DEFAULT_DEVICE_DESC "AuxAudioDevice1"

/* --------------------------------------------------- */

typedef obs_source_t* SourceContext;
typedef obs_scene_t* SceneContext;
typedef obs_display_t* DisplayContext;
typedef obs_properties_t* SourcePorperty;
typedef obs_sceneitem_t * SceneItem;

/*
class SourceContext {
	obs_source_t *source;

public:
	inline bool IsValid() { return source != nullptr; }
	inline SourceContext() { source = nullptr; }
	inline SourceContext(obs_source_t *source) : source(source) {}
	inline ~SourceContext() { obs_source_release(source); }
	inline operator obs_source_t*() { return source; }
};



class SceneContext {
	obs_scene_t *scene;

public:
	inline bool IsValid() { return scene != nullptr; }
	inline SceneContext() { scene = nullptr; }
	inline SceneContext(obs_scene_t *scene) : scene(scene) {}
	inline ~SceneContext() { obs_scene_release(scene); }
	inline operator obs_scene_t*() { return scene; }
};



class DisplayContext {
	obs_display_t *display;

public:
	inline bool IsValid() { return display != nullptr; }
	inline DisplayContext() { display = nullptr; }
	inline DisplayContext(obs_display_t *display) : display(display) {}
	inline ~DisplayContext() { obs_display_destroy(display); }
	inline operator obs_display_t*() { return display; }
};



class SourcePorperty {
	obs_properties_t *ppts;

public:
	inline bool IsValid() { return ppts != nullptr; }
	inline SourcePorperty() { ppts = nullptr; }
	inline SourcePorperty(obs_properties_t *ppts) : ppts(ppts) {}
	inline ~SourcePorperty() { obs_properties_destroy(ppts); }
	inline operator obs_properties_t*() { return ppts; }
};

*/
