#pragma once

#include "interface_define.hpp"

class i_surface {
public:
	void lock_cursor() {
		using fn = void(__thiscall*)(void*);
		return (*(fn**)this)[67](this);
	}
	
	void unlock_cursor() {
		using fn = void(__thiscall*)(void*);
		return (*(fn**)this)[66](this);
	}

	bool is_cursor_visible() {
		using fn = bool(__thiscall*)(void*);
		return (*(fn**)this)[58](this);
	}
};

INITIALIZE_INTERFACE(surface, i_surface, memory::matsurface_module, "VGUI_Surface031");