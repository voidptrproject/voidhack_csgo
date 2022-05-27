#pragma once

#include <functional>
#include "interfaces.hpp"

namespace hooks {
	using create_move_listener = bool(*)(float, c_user_cmd*);
	using frame_stage_notify_listener = void(*)(int);
	using cl_move_listener = void(*)(float, bool);

	enum class e_hook_type {
		create_move,
		frame_stage_notify,
		cl_move
	};

	namespace internal {
		void hook(LPVOID address, LPVOID new_function, LPVOID* original);
		void unhook(LPVOID address);
	}

	struct hook_t {
		hook_t() {}
		hook_t(LPVOID new_function) : hook_fn(new_function), original_fn(0) {}
		hook_t(hook_t&) = default;
		hook_t(hook_t&&) = default;
		~hook_t() {}

		LPVOID original_fn;
		LPVOID hook_fn;

		template <typename fnt> inline fnt original() {
			return reinterpret_cast<fnt>(original_fn);
		}

		inline auto hook(uintptr_t address, LPVOID hook_fn) { internal::hook((LPVOID)address, (LPVOID)hook_fn, (LPVOID*)&original_fn); }
		inline void unhook() { internal::unhook((LPVOID)original_fn); }
	};

	void add_listener(e_hook_type hook_t, void* listener);

	void initialize_hooks();
	void shutdown_hooks();
}

namespace utils {
	namespace sendpackets {
		extern bool sendpackets_state;
		inline void apply_sendpackets() {
			/*
				mov bl, 1	; in CL_Move
				Just replace 1 with sendpackets_state :/
				Mb in csgo exists other way to change sendpackets but i`m lazy for this
			*/
			static memory::address_t mov_bl_1_address({ "B3 01 8B 01 8B 40 10" }, memory::engine_module);
			DWORD old;
			VirtualProtect((LPVOID)(mov_bl_1_address.address + 0x1), sizeof(bool), PAGE_EXECUTE_READWRITE, &old);
			memset((LPVOID)(mov_bl_1_address.address + 0x1), sendpackets_state, sizeof(bool));
			VirtualProtect((LPVOID)(mov_bl_1_address.address + 0x1), sizeof(bool), old, 0);
		}
	}
}