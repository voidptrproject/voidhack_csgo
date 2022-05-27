#include "features.hpp"

void cl_move_test_(float, bool) {
	if (interfaces::engine->is_in_game()) {
		static int choked_packets = 0;
		if (choked_packets > 14) {
			utils::sendpackets::sendpackets_state = true;
			choked_packets = 0;
		} else {
			utils::sendpackets::sendpackets_state = false;
			choked_packets++;
		}
	}
}

features::feature test_feature([]() {
	hooks::add_listener(hooks::e_hook_type::cl_move, cl_move_test_);
});