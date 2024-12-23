#include "entity/dropped_block.h"

#include "world/tessellator.h"
#include "world/world.h"

#include "block/blocks.h"

#include "voxel.h"

#include <stdlib.h>

#define DROPPED_SIZE 0.1

static void dropped_block_draw(struct entity *entity)
{
#if 0
	Main::getDroppedShader().program->use();
	Main::getTerrain()->bind();
	Vec3 pos = getRealPos();
	Mat4 model(Mat4::translate(Mat4(1), pos));
	model = Mat4::translate(model, Vec3(0, std::sin((nanotime + this->created) / 500000000.) * .1, 0));
	model = Mat4::scale(model, Vec3(.2, .2, .2));
	model = Mat4::rotateY(model, (nanotime + this->created) / 1000000000.);
	model = Mat4::translate(model, Vec3(-.5, -.5, -.5));
	Mat4 mvp = this->world.getPlayer().getViewProjMat() * model;
	Main::getDroppedShader().vLocation.setMat4f(this->world.getPlayer().getViewMat());
	Main::getDroppedShader().mLocation.setMat4f(model);
	Main::getDroppedShader().mvpLocation.setMat4f(mvp);
	Main::getDroppedShader().vertexUVLocation.setVertexBuffer(this->uvBuffer, 2, GL_FLOAT);
	Main::getDroppedShader().vertexColorLocation.setVertexBuffer(this->colorBuffer, 3, GL_FLOAT);
	Main::getDroppedShader().vertexPositionLocation.setVertexBuffer(this->positionBuffer, 3, GL_FLOAT);
	this->indiceBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
	glEnable(GL_CULL_FACE);
	glDrawElements(GL_TRIANGLES, this->indicesNb, GL_UNSIGNED_INT, NULL);
#endif
}

static const struct entity_vtable dropped_block_vtable =
{
	.draw = dropped_block_draw,
};

struct entity *dropped_block_new(struct world *world, struct chunk *chunk, uint8_t type, struct vec3f pos, struct vec3f vel)
{
	struct dropped_block *dropped_block = malloc(sizeof(*dropped_block));
	if (!dropped_block)
		return NULL;
	entity_init(&dropped_block->entity, world, chunk);
	dropped_block->entity.vtable = &dropped_block_vtable;
	dropped_block->number = 1;
	dropped_block->type = type;
#if 0
	Block *block = Blocks::getBlock(type);
	if (!block)
	{
		this->deleted = true;
		return;
	}
	setSize(Vec3(.3, .5, .3));
	setPos(pos);
	dropped_block->posDst = vel;
	ChunkTessellator tessellator;
	float tmp[24];
	for (uint8_t i = 0; i < 24; ++i)
		tmp[i] = voxel_light_value(0xf);
	block->draw(dropped_block->chunk, pos, tessellator, 0xff, tmp); //XXX: not pos, should be 0 to avoir rotation offset
	dropped_block->positionBuffer.setData(GL_ARRAY_BUFFER, tessellator.positions.data(), sizeof(Vec3) * tessellator.positions.size(), GL_STATIC_DRAW);
	dropped_block->indiceBuffer.setData(GL_ELEMENT_ARRAY_BUFFER, tessellator.indices.data(), sizeof(GLuint) * tessellator.indices.size(), GL_STATIC_DRAW);
	dropped_block->colorBuffer.setData(GL_ARRAY_BUFFER, tessellator.colors.data(), sizeof(Vec3) * tessellator.colors.size(), GL_STATIC_DRAW);
	dropped_block->uvBuffer.setData(GL_ARRAY_BUFFER, tessellator.uvs.data(), sizeof(Vec2) * tessellator.uvs.size(), GL_STATIC_DRAW);
	dropped_block->indicesNb = tessellator.indices.size();
#endif
	dropped_block->created = g_voxel->frametime;
	return &dropped_block->entity;
}
