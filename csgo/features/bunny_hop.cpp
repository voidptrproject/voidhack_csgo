#include "features.hpp"

create_variable(bunny_hop, bool);

bool bunny_hop_create_move(float f, c_user_cmd* cmd) {
	if (bunny_hop) {
		auto local_player = get_local_player();
		static bool should_fake = false;
		if (static bool last_jumped = false; !last_jumped && should_fake) {
			should_fake = false;
			cmd->buttons |= IN_JUMP;
		} else if (cmd->buttons & IN_JUMP) {
			if (local_player->flags() & (1 << 0)) {
				last_jumped = true;
				should_fake = true;
			} else {
				cmd->buttons &= ~IN_JUMP;
				last_jumped = false;
			}
		} else {
			last_jumped = false;
			should_fake = false;
		}
	}

	return false;
}

features::feature bunny_hop_feature([]() {
	settings::register_variables(bunny_hop);

	menu::AddElementToCategory(menu::EMenuCategory_Misc, std::make_shared<menu::ToggleButtonElement>("BunnyHop", bunny_hop.ptr()));

	hooks::add_listener(hooks::e_hook_type::create_move, bunny_hop_create_move);
});