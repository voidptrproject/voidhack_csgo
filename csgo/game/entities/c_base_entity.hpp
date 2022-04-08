#pragma once

#include "../misc/netvars.hpp"
#include "../../interfaces.hpp"

#include "i_client_entity.hpp"

#include <map>

#define CHECK_THIS if (!this) return

class c_collidable
{
public:
	virtual void unknown_0() = 0;
	virtual c_vector& mins() = 0;
	virtual c_vector& maxs() = 0;
};

class c_base_player;

class c_base_entity : public i_client_entity
{
public:
	netvar("DT_BaseEntity", "m_vecOrigin", get_origin, c_vector);
	netvar("DT_BaseEntity", "m_nModelIndex", get_model_index, int);
	netvar("DT_BaseEntity", "m_iTeamNum", get_team_num, int);
	netvar("DT_BasePlayer", "m_vecViewOffset[0]", get_view_offset, c_vector);
	netvar("DT_BaseEntity", "m_flAnimTime", get_anim_time, float);
	netvar("DT_BaseEntity", "m_flSimulationTime", get_simulation_time, float);
	netvar("DT_BaseEntity", "m_angRotation", get_rotation, c_vector);
	netvar("DT_BaseEntity", "m_hOwnerEntity", get_owner_entity_handle, uintptr_t);
	netvar("DT_BaseEntity", "m_iMaxHealth", get_max_health, int);

	c_vector get_eye_pos() { return this->get_origin() + this->get_view_offset(); }

	c_collidable* get_collidable_ptr() {
		using original_fn = c_collidable * (__thiscall*)(void*);
		return (*(original_fn**)this)[3](this);
	}

	bool is_player() {
		return get_client_class()->class_id == class_ids::c_csplayer;
	}

	c_vector get_bone(int bone) {
		matrix3x4_t bone_matrix[128];
		if (!setup_bones(bone_matrix, 128, 0x00000100, interfaces::engine->get_last_time_stamp()))
			return {};
		auto hitbox = bone_matrix[bone];
		return { hitbox[0][3], hitbox[1][3], hitbox[2][3] };
	}

	inline c_base_player* as_player() const {
		return (c_base_player*)this;
	}

	inline c_base_entity* as_entity() const {
		return (c_base_entity*)this;
	}

	__forceinline bool equal(c_base_entity* other) const {
		return other->get_index() == this->get_index();
	}
};

__forceinline c_base_entity* get_entity_by_index(const int i)
{
	return static_cast<c_base_entity*>(interfaces::entity_list->get_client_entity(i));
}