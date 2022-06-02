#include "hooks.hpp"

#include <imgui_impl_win32.h>

#include <kiero.h>
#include <MinHook.h>

#include "memory.hpp"
#include "render.hpp"
#include "globals.hpp"
#include "input.hpp"

#include <d3d9.h>
#include <Windows.h>
#include <intrin.h>

#include <thread>
#include <functional>
#include <algorithm>
#include <assert.h>
#include <map>

#include "interfaces.hpp"
#include "features/features.hpp"
#include "menu/menu.hpp"

using namespace hooks;

static auto warning = memory::symbol_t<void(__cdecl*)(char const*, ...)>::get_symbol(memory::tier0_module, "Warning").ptr;

bool utils::sendpackets::sendpackets_state = true;

hook_t cl_move_hook;
hook_t end_scene_hook;
hook_t create_move_hook;
hook_t frame_stage_notify_hook;

inline auto& get_listeners() {
	static std::map<hooks::e_hook_type, std::vector<void*>> hook_listeners;
	return hook_listeners;
}

void __fastcall cl_move_hk(float a, bool b) {
	for (auto& i : get_listeners()[hooks::e_hook_type::cl_move])
		((hooks::cl_move_listener)i)(a, b);
	utils::sendpackets::apply_sendpackets();
	return cl_move_hook.original<void(__stdcall*)(float, bool)>()(a, b);
}

long __stdcall end_scane_hooked_fn(IDirect3DDevice9* device) {
	render::internal::render_hook(device, (uintptr_t)_ReturnAddress());
	return end_scene_hook.original<long(__stdcall*)(IDirect3DDevice9*)>()(device);
}

bool __fastcall create_move_hk(void* self, void* edx, float frame_time, c_user_cmd* cmd) {
	if (!cmd || cmd->command_number == 0 || !interfaces::engine->is_in_game())
		return create_move_hook.original<bool(__thiscall*)(void*, float, c_user_cmd*)>()(self, frame_time, cmd);

	if (menu::MenuOpen()) {
		cmd->buttons &= ~IN_ATTACK;
		cmd->buttons &= ~IN_ATTACK2;
		cmd->buttons &= ~IN_ATTACK3;
	}

	bool _return = false;
	for (auto& i : get_listeners()[hooks::e_hook_type::create_move])
		if (i)
			if (((hooks::create_move_listener)i)(frame_time, cmd) && !_return)
				_return = true;

	create_move_hook.original<bool(__thiscall*)(void*, float, c_user_cmd*)>()(self, frame_time, cmd);

	return _return;
}

WNDPROC original_wndproc = nullptr; extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT STDMETHODCALLTYPE hooked_wndproc(HWND window, UINT message_type, WPARAM w_param, LPARAM l_param) {
	if (message_type == WM_KEYDOWN || message_type == WM_SYSKEYDOWN)
		input::internal::notify_handlers(w_param, input::EKeyState::Pressed);
	else if (message_type == WM_KEYUP || message_type == WM_SYSKEYUP)
		input::internal::notify_handlers(w_param, input::EKeyState::Released);

	if (message_type == WM_XBUTTONDOWN || message_type == WM_XBUTTONDBLCLK || message_type == WM_LBUTTONDOWN ||
		message_type == WM_LBUTTONDBLCLK || message_type == WM_RBUTTONDOWN ||
		message_type == WM_RBUTTONDBLCLK || message_type == WM_MBUTTONDOWN || message_type == WM_MBUTTONDBLCLK) {

		int button = 0;
		if (message_type == WM_LBUTTONDOWN || message_type == WM_LBUTTONDBLCLK) button = 1;
		if (message_type == WM_RBUTTONDOWN || message_type == WM_RBUTTONDBLCLK) button = 2;
		if (message_type == WM_MBUTTONDOWN || message_type == WM_MBUTTONDBLCLK) button = 4;
		if (message_type == WM_XBUTTONDOWN || message_type == WM_XBUTTONDBLCLK)
			button = (HIWORD(w_param) == XBUTTON1) ? 5 : 6;
		input::internal::notify_handlers(button, input::EKeyState::Pressed);
	} else if (message_type == WM_XBUTTONUP || message_type == WM_LBUTTONUP || message_type == WM_RBUTTONUP || message_type == WM_MBUTTONUP) {
		int button = 0;
		if (message_type == WM_LBUTTONUP) button = 1;
		if (message_type == WM_RBUTTONUP) button = 2;
		if (message_type == WM_MBUTTONUP) button = 4;
		if (message_type == WM_XBUTTONUP)
			button = (HIWORD(w_param) == XBUTTON1) ? 5 : 6;
		input::internal::notify_handlers(button, input::EKeyState::Released);
	}

	if (ImGui_ImplWin32_WndProcHandler(window, message_type, w_param, l_param) && menu::MenuOpen() && !interfaces::engine->is_in_game()) {
		return TRUE;
	}

	if (menu::MenuOpen() && (message_type == WM_XBUTTONDOWN || message_type == WM_XBUTTONDBLCLK || message_type == WM_LBUTTONDOWN ||
		message_type == WM_LBUTTONDBLCLK || message_type == WM_RBUTTONDOWN ||
		message_type == WM_RBUTTONDBLCLK || message_type == WM_MBUTTONDOWN || message_type == WM_MBUTTONDBLCLK)) {
		return TRUE;
	}
	if (menu::MenuOpen() && (message_type == WM_XBUTTONUP || message_type == WM_LBUTTONUP || message_type == WM_RBUTTONUP || message_type == WM_MBUTTONUP)) {
		return TRUE;
	}

	return CallWindowProc(original_wndproc, window, message_type, w_param, l_param);
}

void __fastcall frame_stage_hk(chl_client* client, void* edx, int frame_stage) {
	for (auto& i : get_listeners()[hooks::e_hook_type::frame_stage_notify])
		if (i) ((hooks::frame_stage_notify_listener)i)(frame_stage); 
	return frame_stage_notify_hook.original<void(__thiscall*)(chl_client*, int)>()(client, frame_stage);
}

struct basic_hook_data {
	LPVOID address; LPVOID new_function; LPVOID* original;
};

auto& hooks_to_initialization() {
	static std::vector<basic_hook_data> hti;
	return hti;
}

void hooks::initialize_hooks() {
	#ifdef _DEBUG
	assert(MH_Initialize() == MH_OK);
	assert(kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success);
	#else
	MH_Initialize();
	kiero::init(kiero::RenderType::D3D9);
	#endif

	std::string modules[]{ "client.dll", "engine.dll", "studiorender.dll", "materialsystem.dll"};
	std::array<byte, 6> patch({ 0xB0, 0x01, 0xC2, 0x04, 0x00, 0x90 });
	for (auto& base : modules) {
		memory::module_t module(base);
		if (module.get_handle() == INVALID_HANDLE_VALUE)
			continue;

		memory::address_t address({ "55 8B EC 56 8B F1 33 C0 57 8B 7D 08" }, module);
		if (!(void*)address.address)
			continue;

		std::cout << "Patching: " << "0x" << std::hex << address.address << " in module: " << base << std::endl;

		DWORD prot;
		VirtualProtect((LPVOID)address.address, patch.size(), PAGE_EXECUTE_READWRITE, &prot);
		memcpy((void*)address.address, patch.data(), patch.size());
		VirtualProtect((LPVOID)address.address, patch.size(), prot, 0);
	}

	original_wndproc = (WNDPROC)SetWindowLongPtr(FindWindow("Valve001", 0), GWLP_WNDPROC, (LONG_PTR)hooked_wndproc);
	end_scene_hook.hook(kiero::getMethodsTable()[42], end_scane_hooked_fn);
	create_move_hook.hook(memory::address_t({ "55 8B EC 56 8D 75 04 8B 0E E8 ? ? ? ? 8B 0E" }, memory::client_module), create_move_hk);
	frame_stage_notify_hook.hook(memory::address_t({ "55 8B EC 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0 A2 ? ? ? ?" }, memory::client_module), frame_stage_hk);
	//cl_move_hook.hook(memory::address_t({ "E8 ? ? ? ? FF 15 ? ? ? ? F2 0F 10 05 ? ? ? ? DC 25 ? ? ? ? DD 5D F0" }, memory::engine_module).absolute(0x1, 0x5), cl_move_hk);

	for (auto& i : hooks_to_initialization())
		internal::hook(i.address, i.new_function, i.original);
	hooks_to_initialization().clear();
}

void hooks::shutdown_hooks() {
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninitialize();
	SetWindowLongPtr(FindWindowW(0, L"Garry's Mod (x64)"), GWLP_WNDPROC, (LONG_PTR)original_wndproc);
}

void hooks::add_listener(e_hook_type hook, void* listener) {
	get_listeners()[hook].push_back(listener);
}

void hooks::internal::hook(LPVOID address, LPVOID new_function, LPVOID* original) {
	if (MH_CreateHook((LPVOID)address, (LPVOID)new_function, original) != MH_ERROR_NOT_INITIALIZED) {
		MH_EnableHook((LPVOID)address);
	} else {
		basic_hook_data tmp{};
		tmp.address = address;
		tmp.new_function = new_function;
		tmp.original = original;
		hooks_to_initialization().emplace_back(std::move(tmp));
	}
}

void hooks::internal::unhook(LPVOID address) {
	MH_DisableHook((LPVOID)address); 
	MH_RemoveHook((LPVOID)address);
}