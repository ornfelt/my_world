#ifndef ENTITY_DEF_H
#define ENTITY_DEF_H

#include <stdbool.h>

struct body_part;

struct pigzombie_def
{
	struct body_part *head;
	struct body_part *head_h;
	struct body_part *body;
	struct body_part *arm_l;
	struct body_part *arm_r;
	struct body_part *leg_l;
	struct body_part *leg_r;
};

struct skeleton_def
{
	struct body_part *head;
	struct body_part *body;
	struct body_part *arm_l;
	struct body_part *arm_r;
	struct body_part *leg_l;
	struct body_part *leg_r;
};

struct creeper_def
{
	struct body_part *head;
	struct body_part *body;
	struct body_part *leg_fl;
	struct body_part *leg_fr;
	struct body_part *leg_bl;
	struct body_part *leg_br;
};

struct zombie_def
{
	struct body_part *head;
	struct body_part *body;
	struct body_part *arm_l;
	struct body_part *arm_r;
	struct body_part *leg_l;
	struct body_part *leg_r;
};

struct pigman_def
{
	struct body_part *head;
	struct body_part *body;
	struct body_part *arm_l;
	struct body_part *arm_r;
	struct body_part *leg_l;
	struct body_part *leg_r;
};

struct human_def
{
	struct body_part *head;
	struct body_part *body;
	struct body_part *arm_l;
	struct body_part *arm_r;
	struct body_part *leg_l;
	struct body_part *leg_r;
};

struct sheep_def
{
	struct body_part *head;
	struct body_part *body;
	struct body_part *leg_fl;
	struct body_part *leg_fr;
	struct body_part *leg_bl;
	struct body_part *leg_br;
};

struct slime_def
{
	struct body_part *body;
};

struct pig_def
{
	struct body_part *head;
	struct body_part *body;
	struct body_part *leg_fl;
	struct body_part *leg_fr;
	struct body_part *leg_bl;
	struct body_part *leg_br;
};

struct cow_def
{
	struct body_part *head;
	struct body_part *body;
	struct body_part *horn_l;
	struct body_part *horn_r;
	struct body_part *leg_fl;
	struct body_part *leg_fr;
	struct body_part *leg_bl;
	struct body_part *leg_br;
};

struct entities_def
{
	struct pigzombie_def pigzombie;
	struct skeleton_def skeleton;
	struct creeper_def creeper;
	struct zombie_def zombie;
	struct pigman_def pigman;
	struct human_def human;
	struct sheep_def sheep;
	struct slime_def slime;
	struct pig_def pig;
	struct cow_def cow;
};

struct entities_def *entities_def_new(void);
void entities_def_delete(struct entities_def *entities);

bool pigzombie_def_init(struct pigzombie_def *def);
void pigzombie_def_destroy(struct pigzombie_def *def);
bool skeleton_def_init(struct skeleton_def *def);
void skeleton_def_destroy(struct skeleton_def *def);
bool creeper_def_init(struct creeper_def *def);
void creeper_def_destroy(struct creeper_def *def);
bool zombie_def_init(struct zombie_def *def);
void zombie_def_destroy(struct zombie_def *def);
bool pigman_def_init(struct pigman_def *def);
void pigman_def_destroy(struct pigman_def *def);
bool human_def_init(struct human_def *def);
void human_def_destroy(struct human_def *def);
bool sheep_def_init(struct sheep_def *def);
void sheep_def_destroy(struct sheep_def *def);
bool slime_def_init(struct slime_def *def);
void slime_def_destroy(struct slime_def *def);
bool pig_def_init(struct pig_def *def);
void pig_def_destroy(struct pig_def *def);
bool cow_def_init(struct cow_def *def);
void cow_def_destroy(struct cow_def *def);

#endif
