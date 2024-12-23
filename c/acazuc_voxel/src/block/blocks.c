#include "block/blocks.h"
#include "block/block.h"

#include "log.h"

#include <string.h>
#include <stdlib.h>

struct blocks_def *blocks_def_new(void)
{
	struct blocks_def *blocks = calloc(sizeof(*blocks), 1);
	if (!blocks)
	{
		LOG_ERROR("blocks allocation failed");
		return NULL;
	}

	/* air */
	struct block_base *air = block_base_new(blocks, 0, "air");
	if (!air)
		goto err;
	air->block.opacity = 1;
	air->block.flags |= BLOCK_FLAG_TRANSPARENT | BLOCK_FLAG_REPLACEABLE;
	air->block.flags &= ~(BLOCK_FLAG_SOLID | BLOCK_FLAG_FOCUSABLE);

	/* stone */
	struct block_base *stone = block_base_new(blocks, 1, "stone");
	if (!stone)
		goto err;
	stone->block.hardness = 1.5;
	stone->block.resistance = 6;
	stone->block.tool = TOOL_PICKAXE;
	block_base_set_tex(stone, 1.0 / 16, 0);

	/* grass_block */
	struct block_base *grass_block = block_grass_new(blocks, 2, "grass_block");
	if (!grass_block)
		goto err;
	grass_block->block.hardness = 0.6;
	grass_block->block.resistance = 0.6;
	grass_block->block.tool = TOOL_SHOVEL;
	block_base_set_tex_top(grass_block, 0, 0);
	block_base_set_tex_side(grass_block, 3.0 / 16, 0);
	block_base_set_tex_bot(grass_block, 2.0 / 16, 0);

	/* dirt */
	struct block_base *dirt = block_base_new(blocks, 3, "dirt");
	if (!dirt)
		goto err;
	dirt->block.hardness = 0.5;
	dirt->block.resistance = 0.5;
	dirt->block.tool = TOOL_SHOVEL;
	block_base_set_tex(dirt, 2.0 / 16, 0);

	/* cobblestone */
	struct block_base *cobblestone = block_base_new(blocks, 4, "cobblestone");
	if (!cobblestone)
		goto err;
	cobblestone->block.hardness = 2;
	cobblestone->block.resistance = 6;
	cobblestone->block.tool = TOOL_PICKAXE;
	block_base_set_tex(cobblestone, 0, 1.0 / 16);

	/* planks */
	struct block_base *planks = block_base_new(blocks, 5, "planks");
	if (!planks)
		goto err;
	planks->block.hardness = 2;
	planks->block.resistance = 3;
	planks->block.tool = TOOL_AXE;
	block_base_set_tex(planks, 4.0 / 16, 0);

	/* sapling */
	struct block_sapling *sapling = block_sapling_new(blocks, 6, "sapling");
	if (!sapling)
		goto err;
	sapling->tex.x = 15.0 / 16;
	sapling->tex.y = 0;

	/* bedrock */
	struct block_base *bedrock = block_base_new(blocks, 7, "bedrock");
	if (!bedrock)
		goto err;
	bedrock->block.hardness = -1;
	bedrock->block.resistance = 3600000;
	block_base_set_tex(bedrock, 1.0 / 16, 1.0 / 16);

	/* flowing_water */
	struct block_liquid *flowing_water = block_water_new(blocks, 8, "flowing_water");
	if (!flowing_water)
		goto err;
	flowing_water->block.hardness = 100;
	flowing_water->block.opacity = 1;
	flowing_water->tex.x = 15.0 / 16;
	flowing_water->tex.y = 13.0 / 16;

	/* water */
	struct block_liquid *water = block_water_new(blocks, 9, "water");
	if (!water)
		goto err;
	water->block.resistance = 100;
	water->block.opacity = 1;
	water->tex.x = 15.0 / 16;
	water->tex.y = 13.0 / 16;

	/* flowing_lava */
	struct block_liquid *flowing_lava = block_lava_new(blocks, 10, "flowing_lava");
	if (!flowing_lava)
		goto err;
	flowing_lava->block.resistance = 100;
	flowing_lava->tex.x = 15.0 / 16;
	flowing_lava->tex.y = 15.0 / 16;

	/* lava */
	struct block_liquid *lava = block_lava_new(blocks, 11, "lava");
	if (!lava)
		goto err;
	lava->block.resistance = 100;
	lava->tex.x = 15.0 / 16;
	lava->tex.y = 15.0 / 16;

	/* sand */
	struct block_base *sand = block_base_new(blocks, 12, "sand");
	if (!sand)
		goto err;
	sand->block.hardness = 0.5;
	sand->block.resistance = 0.5;
	sand->block.tool = TOOL_SHOVEL;
	block_base_set_tex(sand, 2.0 / 16, 1.0 / 16);

	/* gravel */
	struct block_base *gravel = block_base_new(blocks, 13, "gravel");
	if (!gravel)
		goto err;
	gravel->block.hardness = 0.6;
	gravel->block.resistance = 0.6;
	gravel->block.tool = TOOL_SHOVEL;
	block_base_set_tex(gravel, 3.0 / 16, 1.0 / 16);

	/* gold_ore */
	struct block_base *gold_ore = block_base_new(blocks, 14, "gold_ore");
	if (!gold_ore)
		goto err;
	gold_ore->block.hardness = 3;
	gold_ore->block.resistance = 3;
	gold_ore->block.tool = TOOL_PICKAXE;
	block_base_set_tex(gold_ore, 0, 2.0 / 16);

	/* iron_ore */
	struct block_base *iron_ore = block_base_new(blocks, 15, "iron_ore");
	if (!iron_ore)
		goto err;
	iron_ore->block.hardness = 3;
	iron_ore->block.resistance = 3;
	iron_ore->block.tool = TOOL_PICKAXE;
	block_base_set_tex(iron_ore, 1.0 / 16, 2.0 / 16);

	/* coal_ore */
	struct block_base *coal_ore = block_base_new(blocks, 16, "coal_ore");
	if (!coal_ore)
		goto err;
	coal_ore->block.hardness = 3;
	coal_ore->block.resistance = 3;
	coal_ore->block.tool = TOOL_PICKAXE;
	block_base_set_tex(coal_ore, 2.0 / 16, 2.0 / 16);

	/* tree */
	struct block_base *tree = block_base_new(blocks, 17, "tree");
	if (!tree)
		goto err;
	tree->block.hardness = 2;
	block_base_set_tex(tree, 5.0 / 16, 1.0 / 16);
	block_base_set_tex_side(tree, 4.0 / 16, 1.0 / 16);

	/* tree_leaves */
	struct block_base *tree_leaves = block_leaves_new(blocks, 18, "tree_leaves");
	if (!tree_leaves)
		goto err;
	tree_leaves->block.hardness = 0.2;
	tree_leaves->block.resistance = 0.2;
	tree_leaves->block.tool = TOOL_HOE;
	tree_leaves->block.layer = 1;
	tree_leaves->block.flags |= BLOCK_FLAG_TRANSPARENT | BLOCK_FLAG_RENDER_SAME_NEIGHBOR;
	tree_leaves->block.opacity = 1;
	block_base_set_tex(tree_leaves, 4.0 / 16, 3.0 / 16);

	/* sponge */
	struct block_base *sponge = block_base_new(blocks, 19, "sponge");
	if (!sponge)
		goto err;
	sponge->block.hardness = 0.6;
	sponge->block.resistance = 0.6;
	sponge->block.tool = TOOL_HOE;
	block_base_set_tex(sponge, 0, 3.0 / 16);

	/* glass */
	struct block_base *glass = block_base_new(blocks, 20, "glass");
	if (!glass)
		goto err;
	glass->block.hardness = 0.3;
	glass->block.resistance = 0.3;
	glass->block.flags |= BLOCK_FLAG_TRANSPARENT;
	glass->block.opacity = 0;
	block_base_set_tex(glass, 1.0 / 16, 3.0 / 16);

	/* lapis_ore */
	struct block_base *lapis_ore = block_base_new(blocks, 21, "lapis_ore");
	if (!lapis_ore)
		goto err;
	lapis_ore->block.hardness = 3;
	lapis_ore->block.resistance = 3;
	lapis_ore->block.tool = TOOL_PICKAXE;
	block_base_set_tex(lapis_ore, 0, 10.0 / 16);

	/* lapis_block */
	struct block_base *lapis_block = block_base_new(blocks, 22, "lapis_block");
	if (!lapis_block)
		goto err;
	lapis_block->block.hardness = 3;
	lapis_block->block.resistance = 3;
	lapis_block->block.tool = TOOL_PICKAXE;
	block_base_set_tex(lapis_block, 0, 9.0 / 16);

	/* dispenser */
	struct block_base *dispenser = block_base_new(blocks, 23, "dispenser");
	if (!dispenser)
		goto err;
	dispenser->block.hardness = 3.5;
	dispenser->block.resistance = 3.5;
	dispenser->block.tool = TOOL_PICKAXE;
	block_base_set_tex(dispenser, 14.0 / 16, 3.0 / 16);
	block_base_set_tex_side(dispenser, 13.0 / 16, 2.0 / 16);
	block_base_set_tex_front(dispenser, 14.0 / 16, 2.0 / 16);

	/* sandstone */
	struct block_base *sandstone = block_base_new(blocks, 24, "sandstone");
	if (!sandstone)
		goto err;
	sandstone->block.hardness = 0.8;
	sandstone->block.resistance = 0.8;
	sandstone->block.tool = TOOL_PICKAXE;
	block_base_set_tex_top(sandstone, 0, 11.0 / 16);
	block_base_set_tex_side(sandstone, 0, 12.0 / 16);
	block_base_set_tex_bot(sandstone, 0, 13.0 / 16);

	/* noteblock */
	struct block_base *noteblock = block_base_new(blocks, 25, "noteblock");
	if (!noteblock)
		goto err;
	noteblock->block.hardness = 0.8;
	noteblock->block.resistance = 0.8;
	noteblock->block.tool = TOOL_AXE;
	block_base_set_tex(noteblock, 10.0 / 16, 4.0 / 16);

	/* powered_rail */
	struct block_rail *powered_rail = block_rail_new(blocks, 26, "powered_rail");
	if (!powered_rail)
		goto err;
	powered_rail->block.hardness = 0.7;
	powered_rail->block.resistance = 0.7;
	powered_rail->block.tool = TOOL_PICKAXE;
	powered_rail->tex.x = 3.0 / 16;
	powered_rail->tex.y = 10.0 / 16;

	/* detector rail */
	struct block_rail *detector_rail = block_rail_new(blocks, 27, "detector_rail");
	if (!detector_rail)
		goto err;
	detector_rail->block.hardness = 0.7;
	detector_rail->block.resistance = 0.7;
	detector_rail->block.tool = TOOL_PICKAXE;
	detector_rail->tex.x = 3.0 / 16;
	detector_rail->tex.y = 12.0 / 16;

	/* XXX gap */

	/* yellow_flower */
	struct block_sapling *yellow_flower = block_sapling_new(blocks, 37, "yellow_flower");
	if (!yellow_flower)
		goto err;
	yellow_flower->tex.x = 13.0 / 16;
	yellow_flower->tex.y = 0;

	/* red_flower */
	struct block_sapling *red_flower = block_sapling_new(blocks, 38, "red_flower");
	if (!red_flower)
		goto err;
	red_flower->tex.x = 12.0 / 16;
	red_flower->tex.y = 0;

	/* brown_mushroom */
	struct block_sapling *brown_mushroom = block_sapling_new(blocks, 39, "brown_mushroom");
	if (!brown_mushroom)
		goto err;
	brown_mushroom->tex.x = 13.0 / 16;
	brown_mushroom->tex.y = 1.0 / 16;

	/* red_mushroom */
	struct block_sapling *red_mushroom = block_sapling_new(blocks, 40, "red_mushroom");
	if (!red_mushroom)
		goto err;
	red_mushroom->tex.x = 12.0 / 16;
	red_mushroom->tex.y = 1.0 / 16;

	/* gold_block */
	struct block_base *gold_block = block_base_new(blocks, 41, "gold_block");
	if (!gold_block)
		goto err;
	gold_block->block.hardness = 3;
	gold_block->block.resistance = 6;
	gold_block->block.tool = TOOL_PICKAXE;
	block_base_set_tex(gold_block, 7.0 / 16, 1.0 / 16);

	/* iron_block */
	struct block_base *iron_block = block_base_new(blocks, 42, "iron_block");
	if (!iron_block)
		goto err;
	iron_block->block.hardness = 56;
	iron_block->block.resistance = 6;
	iron_block->block.tool = TOOL_PICKAXE;
	block_base_set_tex(iron_block, 6.0 / 16, 1.0 / 16);

	/* double_stone_slab */
	struct block_base *double_stone_slab = block_double_slab_new(blocks, 43, "double_stone_slab");
	if (!double_stone_slab)
		goto err;
	double_stone_slab->block.hardness = 2;
	double_stone_slab->block.resistance = 6;
	double_stone_slab->block.tool = TOOL_PICKAXE;
	block_base_set_tex(double_stone_slab, 5.0 / 16, 0);
	block_base_set_tex_top(double_stone_slab, 6.0 / 16, 0);
	block_base_set_tex_bot(double_stone_slab, 6.0 / 16, 0);

	/* stone_slab */
	struct block_base *stone_slab = block_slab_new(blocks, 44, "stone_slab");
	if (!stone_slab)
		goto err;
	double_stone_slab->block.hardness = 2;
	double_stone_slab->block.resistance = 6;
	double_stone_slab->block.tool = TOOL_PICKAXE;
	stone_slab->block.flags |= BLOCK_FLAG_TRANSPARENT;
	stone_slab->block.opacity = 0;
	block_base_set_tex(stone_slab, 5.0 / 16, 0);
	block_base_set_tex_top(stone_slab, 6.0 / 16, 0);
	block_base_set_tex_bot(stone_slab, 6.0 / 16, 0);

	/* bricks */
	struct block_base *bricks = block_base_new(blocks, 45, "bricks");
	if (!bricks)
		goto err;
	bricks->block.hardness = 2;
	bricks->block.resistance = 6;
	bricks->block.tool = TOOL_PICKAXE;
	block_base_set_tex(bricks, 7.0 / 16, 0);

	/* tnt */
	struct block_base *tnt = block_base_new(blocks, 46, "tnt");
	if (!tnt)
		goto err;
	tnt->block.resistance = 25;
	block_base_set_tex(tnt, 8.0 / 16, 0);
	block_base_set_tex_top(tnt, 9.0 / 16, 0);
	block_base_set_tex_bot(tnt, 10.0 / 16, 0);

	/* bookshelf */
	struct block_base *bookshelf = block_base_new(blocks, 47, "bookshelf");
	if (!bookshelf)
		goto err;
	bookshelf->block.hardness = 1.5;
	bookshelf->block.resistance = 1.5;
	bookshelf->block.tool = TOOL_AXE;
	block_base_set_tex(bookshelf, 3.0 / 16, 2.0 / 16);

	/* mossy_cobblestone */
	struct block_base *mossy_cobblestone = block_base_new(blocks, 48, "mossy_cobblestone");
	if (!mossy_cobblestone)
		goto err;
	mossy_cobblestone->block.hardness = 2;
	mossy_cobblestone->block.resistance = 6;
	mossy_cobblestone->block.tool = TOOL_PICKAXE;
	block_base_set_tex(mossy_cobblestone, 4.0 / 16, 2.0 / 16);

	/* obsidian */
	struct block_base *obsidian = block_base_new(blocks, 49, "obsidian");
	if (!obsidian)
		goto err;
	obsidian->block.hardness = 50;
	obsidian->block.resistance = 1200;
	obsidian->block.tool = TOOL_PICKAXE;
	block_base_set_tex(obsidian, 5.0 / 16, 2.0 / 16);

	/* torch */
	struct block_torch *torch = block_torch_new(blocks, 50, "torch");
	if (!torch)
		goto err;
	torch->block.opacity = 0;
	torch->block.flags |= BLOCK_FLAG_TRANSPARENT | BLOCK_FLAG_REPLACEABLE;
	torch->tex.x = 0;
	torch->tex.y = 5.0 / 16;

	/* XXX 51 */

	/* mob_spawner */
	struct block_base *mob_spawner = block_base_new(blocks, 52, "mob_spawner");
	if (!mob_spawner)
		goto err;
	mob_spawner->block.hardness = 5;
	mob_spawner->block.resistance = 5;
	mob_spawner->block.tool = TOOL_PICKAXE;
	mob_spawner->block.flags |= BLOCK_FLAG_TRANSPARENT;
	mob_spawner->block.opacity = 1;
	block_base_set_tex(mob_spawner, 1.0 / 16, 4.0 / 16);

	/* XXX 53 */

	/* chest */
	struct block_base *chest = block_base_new(blocks, 54, "chest");
	if (!chest)
		goto err;
	chest->block.hardness = 2.5;
	chest->block.resistance = 2.5;
	chest->block.tool = TOOL_AXE;
	block_base_set_tex(chest, 10.0 / 16, 1.0 / 16);
	block_base_set_tex_front(chest, 11.0 / 16, 1.0 / 16);
	block_base_set_tex_top(chest, 9.0 / 16, 1.0 / 16);
	block_base_set_tex_bot(chest, 9.0 / 16, 1.0 / 16);

	/* redstone */
	struct block_redstone *redstone = block_redstone_new(blocks, 55, "redstone");
	if (!redstone)
		goto err;
	redstone->block.flags |= BLOCK_FLAG_TRANSPARENT;
	redstone->block.opacity = 0;
	redstone->tex.x = 4.0 / 16;
	redstone->tex.y = 10.0 / 16;

	/* diamond_ore */
	struct block_base *diamond_ore = block_base_new(blocks, 56, "diamond_ore");
	if (!diamond_ore)
		goto err;
	diamond_ore->block.hardness = 3;
	diamond_ore->block.resistance = 3;
	diamond_ore->block.tool = TOOL_PICKAXE;
	block_base_set_tex(diamond_ore, 2.0 / 16, 3.0 / 16);

	/* diamond_block */
	struct block_base *diamond_block = block_base_new(blocks, 57, "diamond_block");
	if (!diamond_block)
		goto err;
	diamond_block->block.hardness = 5;
	diamond_block->block.resistance = 6;
	diamond_block->block.tool = TOOL_PICKAXE;
	block_base_set_tex(diamond_block, 8.0 / 16, 1.0 / 16);

	/* crafting_table */
	struct block_base *crafting_table = block_crafting_table_new(blocks, 58, "crafting_table");
	if (!crafting_table)
		goto err;
	crafting_table->block.hardness = 2.5;
	crafting_table->block.resistance = 2.5;
	crafting_table->block.tool = TOOL_AXE;
	block_base_set_tex_side(crafting_table, 11.0 / 16, 3.0 / 16);
	block_base_set_tex_front(crafting_table, 12.0 / 16, 3.0 / 16);
	block_base_set_tex_back(crafting_table, 12.0 / 16, 3.0 / 16);
	block_base_set_tex_top(crafting_table, 11.0 / 16, 2.0 / 16);
	block_base_set_tex_bot(crafting_table, 4.0 / 16, 0);

	/* XXX gap */

	/* rail */
	struct block_rail *rail = block_rail_new(blocks, 66, "rail");
	if (!rail)
		goto err;
	rail->block.hardness = 0.7;
	rail->block.resistance = 0.7;
	rail->block.tool = TOOL_PICKAXE;
	rail->tex.x = 0;
	rail->tex.y = 8.0 / 16;

	/* XXX more */

	return blocks;

err:
	blocks_def_delete(blocks);
	return NULL;
}

void blocks_def_delete(struct blocks_def *blocks)
{
	if (!blocks)
		return;
	for (size_t i = 0; i < 4096; ++i)
		block_def_delete(blocks->blocks[i]);
	free(blocks);
}
