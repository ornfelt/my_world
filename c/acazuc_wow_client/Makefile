include config

NAME = wow

WITH_ZLIB = yes
WITH_LIBPNG = yes
WITH_FREETYPE = yes
WITH_LIBXML2 = yes
WITH_LIBWOW = yes
WITH_JKS = yes
WITH_GFX = yes
WITH_LUA = yes
WITH_PORTAUDIO = yes
WITH_LIBSAMPLERATE = yes
WITH_JKSSL = yes

CPPFLAGS+= -DRENDER_FRAMES_COUNT=3

ifeq ($(WITH_PERFORMANCE), YES)
CPPFLAGS+= -DWITH_PERFORMANCE
endif

ifeq ($(WITH_MEMORY), YES)
CPPFLAGS+= -DWITH_MEMORY
endif

ifeq ($(WITH_OPTIMIZATIONS), YES)
CPPFLAGS+= -DDBG_NO_FILE
endif

SRCS_NAME = cache.c \
            camera.c \
            char_input.c \
            cvars.c \
            dbc.c \
            graphics.c \
            lagometer.c \
            loader.c \
            log.c \
            memory.c \
            performance.c \
            shaders.c \
            simplex.c \
            wdb.c \
            wow.c \
            gx/blp.c \
            gx/frame.c \
            gx/m2.c \
            gx/m2_particles.c \
            gx/m2_ribbons.c \
            gx/mclq.c \
            gx/mcnk.c \
            gx/skybox.c \
            gx/text.c \
            gx/wdl.c \
            gx/wmo.c \
            gx/wmo_group.c \
            gx/wmo_mliq.c \
            ppe/bloom_render_pass.c \
            ppe/filter_render_pass.c \
            ppe/render_pass.c \
            ppe/render_target.c \
            ppe/ssao_render_pass.c \
            obj/container.c \
            obj/corpse.c \
            obj/creature.c \
            obj/dynobj.c \
            obj/fields.c \
            obj/gameobj.c \
            obj/item.c \
            obj/object.c \
            obj/player.c \
            obj/unit.c \
            obj/worldobj.c \
            itf/enum.c \
            itf/addon.c \
            itf/interface.c \
            ui/anchor.c \
            ui/backdrop.c \
            ui/button.c \
            ui/check_button.c \
            ui/color.c \
            ui/color_select.c \
            ui/cooldown.c \
            ui/dimension.c \
            ui/dress_up_model.c \
            ui/edit_box.c \
            ui/frame.c \
            ui/font.c \
            ui/font_instance.c \
            ui/font_string.c \
            ui/game_tooltip.c \
            ui/gradient.c \
            ui/inset.c \
            ui/layered_region.c \
            ui/message_frame.c \
            ui/minimap.c \
            ui/model.c \
            ui/model_ffx.c \
            ui/movie_frame.c \
            ui/object.c \
            ui/player_model.c \
            ui/region.c \
            ui/texture.c \
            ui/scroll_frame.c \
            ui/scrolling_message_frame.c \
            ui/shadow.c \
            ui/simple_html.c \
            ui/slider.c \
            ui/status_bar.c \
            ui/tabard_model.c \
            ui/taxi_route_frame.c \
            ui/tex_coords.c \
            ui/value.c \
            ui/world_frame.c \
            xml/anchors.c \
            xml/attributes.c \
            xml/backdrop.c \
            xml/button.c \
            xml/check_button.c \
            xml/color.c \
            xml/color_select.c \
            xml/cooldown.c \
            xml/dimension.c \
            xml/dress_up_model.c \
            xml/edit_box.c \
            xml/element.c \
            xml/font.c \
            xml/font_string.c \
            xml/frame.c \
            xml/frames.c \
            xml/game_tooltip.c \
            xml/gradient.c \
            xml/include.c \
            xml/inset.c \
            xml/layers.c \
            xml/layout_frame.c \
            xml/message_frame.c \
            xml/minimap.c \
            xml/model.c \
            xml/model_ffx.c \
            xml/movie_frame.c \
            xml/player_model.c \
            xml/resize_bounds.c \
            xml/script.c \
            xml/scripts.c \
            xml/scroll_child.c \
            xml/scroll_frame.c \
            xml/scrolling_message_frame.c \
            xml/simple_html.c \
            xml/slider.c \
            xml/shadow.c \
            xml/status_bar.c \
            xml/tabard_model.c \
            xml/taxi_route_frame.c \
            xml/tex_coords.c \
            xml/texture.c \
            xml/ui.c \
            xml/value.c \
            xml/world_frame.c \
            lua/addon.c \
            lua/arena.c \
            lua/auction.c \
            lua/audio.c \
            lua/battle_ground.c \
            lua/bindings.c \
            lua/chat.c \
            lua/combat_log.c \
            lua/craft.c \
            lua/glue.c \
            lua/guild.c \
            lua/inbox.c \
            lua/input.c \
            lua/kb.c \
            lua/lua_script.c \
            lua/lfg.c \
            lua/loot.c \
            lua/macro.c \
            lua/misc.c \
            lua/objects.c \
            lua/party.c \
            lua/pet.c \
            lua/pvp.c \
            lua/quest.c \
            lua/secure.c \
            lua/std.c \
            lua/social.c \
            lua/skill.c \
            lua/spell.c \
            lua/system.c \
            lua/taxi.c \
            lua/talent.c \
            lua/trade.c \
            lua/unit.c \
            lua/voice.c \
            lua/world_map.c \
            net/auth_socket.c \
            net/buffer.c \
            net/network.c \
            net/packet.c \
            net/packet_handler.c \
            net/socket.c \
            net/world_socket.c \
            net/pkt/auth.c \
            net/pkt/cache.c \
            net/pkt/group.c \
            net/pkt/misc.c \
            net/pkt/movement.c \
            net/pkt/object.c \
            net/pkt/social.c \
            font/atlas.c \
            font/font.c \
            font/model.c \
            game/group.c \
            game/guild.c \
            game/social.c \
            snd/filter.c \
            snd/mp3.c \
            snd/resample.c \
            snd/snd.c \
            snd/stream.c \
            snd/wav.c \
            map/map.c \
            map/tile.c \
            phys/physics.c \

ifeq ($(WITH_DEBUG_RENDERING), YES)

CPPFLAGS+= -DWITH_DEBUG_RENDERING

SRCS_NAME+= gx/aabb.c \
            gx/collisions.c \
            gx/m2_bones.c \
            gx/m2_collisions.c \
            gx/m2_lights.c \
            gx/taxi.c \
            gx/wmo_collisions.c \
            gx/wmo_lights.c \
            gx/wmo_portals.c \

endif

include lib/jkl/jkm.mk

$(NAME): shaders

clean: shaders_clean

shaders:
	@$(MAKE) -C shaders WITH_GL3=YES WITH_GL4=YES WITH_GLES3=YES WITH_D3D9=NO WITH_D3D11=YES WITH_VK=NO

shaders_clean:
	@$(MAKE) -C shaders clean

.PHONY: shaders_clean shaders
