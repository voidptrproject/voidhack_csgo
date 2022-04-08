#include "c_base_entity.hpp"

#include "../interfaces/playerinfo.hpp"
#include "../../globals.hpp"

class c_base_player : public c_base_entity {
public:
	netvar("DT_CSPlayer", "m_fFlags", flags, int);
	netvar("DT_CSPlayer", "m_iHealth", health, int);

	bool is_alive() {
		return health() > 0;
	}

	std::string get_name() {
		static memory::symbol_t<const wchar_t* (__thiscall*)(void* self, int n, wchar_t* dst, int a4, int a5)> GetPlayerName(
			memory::address_t({ "E8 ? ? ? ? 66 83 3E 00" }, memory::client_module).absolute(0x1, 0x5));

		wchar_t widename[128];
		GetPlayerName.ptr(*interfaces::playerinfo.ptr, get_index(), widename, sizeof(widename), 4);

		return env::wstring_to_string(widename);
	}
};

inline c_base_player* get_local_player() {
	return (c_base_player*)interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player());
}