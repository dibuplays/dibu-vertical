#include "inverse-bulge-filter.h"

#include <obs-module.h>

struct inverse_bulge_data {
	obs_source_t *context;
	gs_effect_t *effect;
	gs_eparam_t *curve_param;
	gs_eparam_t *edge_param;
	float curve;
	float edge;
};

static const char *inverse_bulge_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("FullViewFilterName");
}

static void inverse_bulge_destroy(void *data)
{
	struct inverse_bulge_data *filter = data;
	if (!filter)
		return;

	obs_enter_graphics();
	gs_effect_destroy(filter->effect);
	obs_leave_graphics();
	bfree(filter);
}

static void inverse_bulge_update(void *data, obs_data_t *settings)
{
	struct inverse_bulge_data *filter = data;
	filter->curve = (float)obs_data_get_double(settings, "curve_strength");
	filter->edge = (float)obs_data_get_double(settings, "edge_compression");
}

static void *inverse_bulge_create(obs_data_t *settings, obs_source_t *source)
{
	struct inverse_bulge_data *filter = bzalloc(sizeof(*filter));
	filter->context = source;

	char *effect_path = obs_module_file("effects/inverse-bulge.effect");
	obs_enter_graphics();
	filter->effect = gs_effect_create_from_file(effect_path, NULL);
	obs_leave_graphics();
	bfree(effect_path);

	if (!filter->effect) {
		inverse_bulge_destroy(filter);
		return NULL;
	}

	filter->curve_param = gs_effect_get_param_by_name(filter->effect, "curve_strength");
	filter->edge_param = gs_effect_get_param_by_name(filter->effect, "edge_compression");
	inverse_bulge_update(filter, settings);
	return filter;
}

static void inverse_bulge_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	struct inverse_bulge_data *filter = data;
	const enum gs_color_space preferred_spaces[] = {
		GS_CS_SRGB,
		GS_CS_SRGB_16F,
		GS_CS_709_EXTENDED,
	};
	const enum gs_color_space source_space = obs_source_get_color_space(
		obs_filter_get_target(filter->context), OBS_COUNTOF(preferred_spaces), preferred_spaces);
	const enum gs_color_format format = gs_get_format_from_space(source_space);
	if (!obs_source_process_filter_begin_with_color_space(filter->context, format, source_space,
							OBS_NO_DIRECT_RENDERING))
		return;

	gs_effect_set_float(filter->curve_param, filter->curve);
	gs_effect_set_float(filter->edge_param, filter->edge);
	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);
}

static enum gs_color_space inverse_bulge_color_space(void *data, size_t count,
						      const enum gs_color_space *preferred_spaces)
{
	UNUSED_PARAMETER(count);
	UNUSED_PARAMETER(preferred_spaces);
	const enum gs_color_space spaces[] = {
		GS_CS_SRGB,
		GS_CS_SRGB_16F,
		GS_CS_709_EXTENDED,
	};
	struct inverse_bulge_data *filter = data;
	return obs_source_get_color_space(obs_filter_get_target(filter->context), OBS_COUNTOF(spaces), spaces);
}

static obs_properties_t *inverse_bulge_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	obs_properties_add_float_slider(props, "curve_strength", obs_module_text("FullViewCurve"), 0.0, 1.0, 0.01);
	obs_properties_add_float_slider(props, "edge_compression", obs_module_text("FullViewEdge"), 0.0, 1.0, 0.01);
	return props;
}

static void inverse_bulge_defaults(obs_data_t *settings)
{
	obs_data_set_default_double(settings, "curve_strength", 0.55);
	obs_data_set_default_double(settings, "edge_compression", 0.45);
}

struct obs_source_info inverse_bulge_filter = {
	.id = "dibu_inverse_bulge_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB,
	.get_name = inverse_bulge_get_name,
	.create = inverse_bulge_create,
	.destroy = inverse_bulge_destroy,
	.update = inverse_bulge_update,
	.get_defaults = inverse_bulge_defaults,
	.get_properties = inverse_bulge_properties,
	.video_render = inverse_bulge_render,
	.video_get_color_space = inverse_bulge_color_space,
};
