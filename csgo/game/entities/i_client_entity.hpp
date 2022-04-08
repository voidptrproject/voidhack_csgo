#pragma once

#include "../math/math.hpp"
#include <d3d9.h>

#include <stdint.h>

#include "../misc/c_client_class.hpp"

class c_base_handle;
class i_handle_entity;
class i_client_unknown;

class i_handle_entity {
public:
	virtual							~i_handle_entity() = default;
	virtual void					set_ref_handle(const c_base_handle& handle) = 0;
	virtual const c_base_handle& get_handle() const = 0;
};

class i_client_thinkable {
public:
	virtual i_client_unknown* get_client_unknown() = 0;
	virtual void					client_think() = 0;
	virtual void* get_think_handle() = 0;
	virtual void					set_think_handle(void* think) = 0;
	virtual void					release() = 0;
};

class i_client_networkable {
public:
	virtual i_client_unknown* get_client_unknown() = 0;
	virtual void				release() = 0;
	virtual c_client_class* get_client_class() = 0;
	virtual void				notify_should_transmit(int state) = 0;
	virtual void				on_pre_data_changed(int update_type) = 0;
	virtual void				on_data_changed(int update_type) = 0;
	virtual void				pre_data_update(int update_type) = 0;
	virtual void				post_data_update(int update_type) = 0;
	virtual void					__unkn(void) = 0;
	virtual bool				is_dormant() = 0;
	virtual int					get_index() const = 0;
	virtual void				receive_message(int class_id, void* msg) = 0;		//bf_read
	virtual void* get_data_table_base_ptr() = 0;
	virtual void				set_destroyed_on_recreate_entities() = 0;
};

struct shadow_type_t;
class ipvs_notify;
struct client_shadow_handle_t;
struct client_render_handle_t;

class i_client_renderable {
public:
	virtual i_client_unknown* get_i_client_unknown() = 0;
	virtual c_vector const& get_render_origin(void) = 0;
	virtual q_angle const& get_render_angles(void) = 0;
	virtual bool                       should_draw(void) = 0;
	virtual int                        get_render_flags(void) = 0; // ERENDERFLAGS_xxx
	virtual void                       unused(void) const {}
	virtual void*      get_shadow_handle() const = 0;
	virtual void* render_handle() = 0;
	virtual const model_t* get_model() const = 0;
	virtual int                        draw_model(int flags, const int /*RenderableInstance_t*/& instance) = 0;
	virtual int                        get_body() = 0;
	virtual void                       get_color_modulation(float* color) = 0;
	virtual bool                       lod_test() = 0;
	virtual bool					   setup_bones(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) = 0;
	virtual void                       setup_weights(const matrix3x4_t* pBoneToWorld, int nFlexWeightCount, float* pFlexWeights, float* pFlexDelayedWeights) = 0;
	virtual void                       do_animation_events(void) = 0;
	virtual void*      get_pvs_notify_interface() = 0;
	virtual void                       get_render_bounds(c_vector& mins, c_vector& maxs) = 0;
	virtual void                       get_render_bounds_worldspace(c_vector& mins, c_vector& maxs) = 0;
	virtual void                       get_shadow_render_bounds(c_vector& mins, c_vector& maxs, int /*ShadowType_t*/ shadowType) = 0;
	virtual bool                       should_receive_projected_textures(int flags) = 0;
	virtual bool                       get_shadow_cast_distance(float* pDist, int /*ShadowType_t*/ shadowType) const = 0;
	virtual bool                       get_shadow_cast_direction(c_vector* pDirection, int /*ShadowType_t*/ shadowType) const = 0;
	virtual bool                       is_shadow_dirty() = 0;
	virtual void                       mark_shadow_dirty(bool bDirty) = 0;
	virtual i_client_renderable* get_shadow_parent() = 0;
	virtual i_client_renderable** first_shadow_child() = 0;
	virtual i_client_renderable* next_shadow_peer() = 0;
	virtual int /*ShadowType_t*/       shadow_cast_type() = 0;
	virtual void                       create_model_instance() = 0;
	virtual void*      get_model_instance() = 0;
	virtual const matrix3x4_t& renderable_to_world_transform() = 0;
	virtual int                        lookup_attachment(const char* pAttachmentName) = 0;
	virtual   bool                     get_attachment(int number, c_vector& origin, q_angle& angles) = 0;
	virtual bool                       get_attachment(int number, matrix3x4_t& matrix) = 0;
	virtual float* get_render_clip_plane(void) = 0;
	virtual int                        get_skin() = 0;
	virtual void                       on_threaded_draw_setup() = 0;
	virtual bool                       uses_flex_delayed_weights() = 0;
	virtual void                       record_tool_message() = 0;
	virtual bool                       should_draw_for_split_screen_user(int nSlot) = 0;
	virtual uint8_t                    override_alpha_modulation(uint8_t nAlpha) = 0;
	virtual uint8_t                    override_shadow_alpha_modulation(uint8_t nAlpha) = 0;
};

class c_base_entity;
class i_client_entity;
class i_client_alpha_property;

class i_client_unknown : public i_handle_entity {
public:
	virtual void* get_collideable() = 0;	//i_collideable
	virtual i_client_networkable* get_client_networkable() = 0;
	virtual i_client_renderable* get_client_renderable() = 0;
	virtual i_client_entity* get_client_entity() = 0;
	virtual c_base_entity* get_base_entity() = 0;
	virtual i_client_thinkable* get_client_thinkable() = 0;
	virtual i_client_alpha_property* get_client_alpha_property() = 0;
};

class i_client_entity : public i_client_unknown, public i_client_renderable, public i_client_networkable, public i_client_thinkable {
public:
	virtual void release() = 0;
};