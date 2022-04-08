#pragma once

#include "interface_define.hpp"

#include "../misc/color.hpp"
#include "../math/math.hpp"

class i_debug_overlay
{
public:
	bool screen_position(c_vector const& in, c_vector& out) {
		static memory::symbol_t<bool(__stdcall*)(const c_vector* in, c_vector* out)> function(memory::address_t(
			{"55 8B EC 8B 55 0C 8B 4D 08 E8 ? ? ? ? 5D C2 08 00 CC CC CC CC CC CC CC CC CC CC CC CC CC CC 55 8B EC 8B 4D 10"}, memory::engine_module));
		return function.ptr(&in, &out);
	}
	
};

INITIALIZE_INTERFACE(debug_overlay, i_debug_overlay, memory::engine_module, "VDebugOverlay004");