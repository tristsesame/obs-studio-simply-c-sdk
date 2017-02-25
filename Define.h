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

#define TEXT_FILTER_NOISE "noise_gate_name"
#define TEXT_FILTER_NOISE_ID "noise_gate_filter"

#define S_OPEN_THRESHOLD               "open_threshold"
#define S_CLOSE_THRESHOLD              "close_threshold"
#define S_ATTACK_TIME                  "attack_time"
#define S_HOLD_TIME                    "hold_time"
#define S_RELEASE_TIME                 "release_time"


#define PREVIEW_EDGE_SIZE 10

/* --------------------------------------------------- */

typedef obs_source_t* SourceContext;
typedef obs_scene_t* SceneContext;
typedef obs_display_t* DisplayContext;
typedef obs_properties_t* SourcePorperty;
typedef obs_sceneitem_t * SceneItem;

obs_service_t * service = nullptr;
obs_data_t *h264Settings = nullptr;
obs_data_t *aacSettings = nullptr;

//rtmp
obs_encoder_t *h264Streaming = nullptr;
obs_encoder_t *aacStreaming = nullptr;
obs_output_t* streamOutput = nullptr;

//record
obs_encoder_t *aacRecording = nullptr;
obs_encoder_t *h264Recording = nullptr;
obs_output_t* fileOutput = nullptr;

static bool firstStart = true;

static int preview_wnd_width = 800;
static int preview_wnd_height = 600;

static float preview_wnd_scale = 0.0;

static int  preview_wnd_x = 0;
static int  preview_wnd_y = 0;

static uint64_t lastBytesSent = 0;
static uint64_t lastBytesSentTime = 0;
static uint64_t beginBytesSend = 0;

static uint64_t lastTotalFrames = 0;


//static const int cx = 800;
//static const int cy = 600;
