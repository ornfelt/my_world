include config

NAME = voxel

WITH_ZLIB = yes
WITH_LIBPNG = yes
WITH_GLFW = yes
WITH_JKS = yes
WITH_GFX = yes
WITH_LIBNBT = yes

ifeq ($(WITH_OPTIMIZATIONS), YES)
CPPFLAGS+= -DDBG_NO_FILE
endif

SRCS_NAME = voxel.c \
            log.c \
            shaders.c \
            textures.c \
            graphics.c \
            lagometer.c \
            noise/simplex.c \
            noise/simplex_octave.c \
            noise/worley.c \
            player/inventory.c \
            player/player.c \
            player/raycast.c \
            block/base.c \
            block/block.c \
            block/blocks.c \
            block/crafting_table.c \
            block/grass.c \
            block/leaves.c \
            block/liquid.c \
            block/rail.c \
            block/redstone.c \
            block/slab.c \
            block/sapling.c \
            block/torch.c \
            entity/body_part.c \
            entity/human.c \
            entity/creeper.c \
            entity/zombie.c \
            entity/pig.c \
            entity/pigman.c \
            entity/pigzombie.c \
            entity/sheep.c \
            entity/skeleton.c \
            entity/slime.c \
            entity/cow.c \
            entity/entity.c \
            entity/entities.c \
            entity/dropped_block.c \
            entity/def.c \
            world/world.c \
            world/block.c \
            world/chunk.c \
            world/region.c \
            world/storage.c \
            world/level.c \
            world/clouds.c \
            world/skybox.c \
            biome/biome.c \
            biome/biomes.c \
            generator/tree.c \
            generator/minable.c \
            generator/generator.c \
            particle/particle.c \
            particle/particles.c \
            world_screen/world_screen.c \
            world_screen/gui.c \
            title_screen/title_screen.c \
            gui/bar.c \
            gui/button.c \
            gui/crafting.c \
            gui/cross.c \
            gui/gui.c \
            gui/hearts.c \
            gui/inventory.c \
            gui/item_slot.c \
            gui/label.c \
            gui/multiplayer.c \
            gui/multiplayer_entry.c \
            gui/sprite.c \
            gui/singleplayer.c \
            gui/singleplayer_entry.c \
            gui/slider.c \
            gui/title.c \
            gui/options.c \
            gui/options/chat.c \
            gui/options/controls.c \
            gui/options/skin.c \
            gui/options/sound.c \
            gui/options/video.c \

include lib/jkl/jkm.mk
