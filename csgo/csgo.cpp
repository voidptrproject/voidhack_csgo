#include <thread>

#include "memory.hpp"

#include "hooks.hpp"
#include "menu/menu.hpp"

#include "game/misc/netvars.hpp"

#ifdef _DEBUG
void command_executor() {
	while (true) {
		std::string line;
		std::getline(std::cin, line);

		std::transform(line.begin(), line.end(), line.begin(), [](char c) { return std::tolower(c); });
		if (line.contains("dump netvars")) {
			std::ofstream out(env::get_data_path() / "netvardump.txt");
			netvar_manager::dump_netvars(out);
		}
	}
}
#endif

void entry_point() {
	#ifdef _DEBUG
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);
	freopen("CONIN$", "rb", stdin);

	std::thread(command_executor).detach();
	#endif

	hooks::initialize_hooks();
	menu::InitializeMenu();
}



BOOL APIENTRY DllMain(HINSTANCE dll_instance, DWORD reason, LPVOID reversed) {
	memory::dllinstance = dll_instance;
	DisableThreadLibraryCalls(dll_instance);
	if (reason == DLL_PROCESS_ATTACH) {
		std::thread(entry_point).detach();
	}
	return TRUE;
}

