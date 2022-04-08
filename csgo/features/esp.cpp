#include "features.hpp"

std::mutex render_mutex;

struct esp_box_t {
	ImVec2 min;
	ImVec2 max;

	inline auto width() const { return max.x - min.x; }
	inline auto height() const { return max.y - min.y; }

	inline ImVec2 center() const { return { min.x + width() / 2, min.y + height() / 2 }; }
};

struct render_info_t {
	std::string name;
	esp_box_t box;
};

std::vector<render_info_t> renders;
create_variable(esp_enabled, bool);

inline bool get_entity_box(c_base_entity* ent, render_info_t& render_object) {
	c_vector flb, brt, blb, frt, frb, brb, blt, flt;

	const auto& origin = ent->get_render_origin();
	const auto min = ent->get_collidable_ptr()->mins() + origin;
	const auto max = ent->get_collidable_ptr()->maxs() + origin;

	c_vector points[] = {
		c_vector(min.x, min.y, min.z),
		c_vector(min.x, max.y, min.z),
		c_vector(max.x, max.y, min.z),
		c_vector(max.x, min.y, min.z),
		c_vector(max.x, max.y, max.z),
		c_vector(min.x, max.y, max.z),
		c_vector(min.x, min.y, max.z),
		c_vector(max.x, min.y, max.z)
	};

	if (interfaces::debug_overlay->screen_position(points[3], flb) || interfaces::debug_overlay->screen_position(points[5], brt)
		|| interfaces::debug_overlay->screen_position(points[0], blb) || interfaces::debug_overlay->screen_position(points[4], frt)
		|| interfaces::debug_overlay->screen_position(points[2], frb) || interfaces::debug_overlay->screen_position(points[1], brb)
		|| interfaces::debug_overlay->screen_position(points[6], blt) || interfaces::debug_overlay->screen_position(points[7], flt))
		return false;

	c_vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	auto left = flb.x;
	auto top = flb.y;
	auto right = flb.x;
	auto bottom = flb.y;

	if (left < 0 || top < 0 || right < 0 || bottom < 0)
		return false;

	for (int i = 1; i < 8; i++) {
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}

	render_object.box.min.x = left;
	render_object.box.min.y = top;
	render_object.box.max.x = right;
	render_object.box.max.y = bottom;

	return true;
}

auto update_list() {
	decltype(renders) renders_tmp;
	for (auto i = 0; i < interfaces::entity_list->get_highest_entity_index(); ++i) {
		auto entity = get_entity_by_index(i);
		if (!entity || !entity->is_player() || entity->get_index() == get_local_player()->get_index() || !entity->as_player()->is_alive())
			continue;

		render_info_t renderinfo;

		renderinfo.name = entity->as_player()->get_name();
		get_entity_box(entity, renderinfo);

		renders_tmp.emplace_back(std::move(renderinfo));
	}

	render_mutex.lock();
	renders.clear();
	renders = renders_tmp;
	render_mutex.unlock();
}

void esp_framestage(e_frame_stage stage) {
	if (!esp_enabled || stage != e_frame_stage::frame_start || !interfaces::engine->is_in_game())
		return;

	update_list();
}

void esp_renderf(render::render_data_t& d) {
	if (!esp_enabled)
		return;

	if (!renders.empty() && !interfaces::engine->is_in_game())
		renders.clear();

	render_mutex.lock();
	for (auto& i : renders) {
		d.draw_list->AddRect(i.box.min, i.box.max, colors::red_color);
		d.draw_list->AddText({i.box.center().x - render::calculate_text_size(i.name).x / 2.f, i.box.min.y - render::calculate_text_size(i.name).y}, colors::blue_color, i.name.c_str());
	}
	render_mutex.unlock();
}

features::feature esp_feature([]() {
	settings::register_variables(esp_enabled);

	menu::AddElementToCategory(menu::EMenuCategory_Esp, std::make_shared<menu::ToggleButtonElement>("ESP##ESPACTIVE", esp_enabled.ptr()));

	hooks::add_listener(hooks::e_hook_type::frame_stage_notify, esp_framestage);
	render::add_render_handler(esp_renderf);
});