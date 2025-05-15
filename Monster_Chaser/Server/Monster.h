#pragma once
#include "stdafx.h"

enum MonType : char
{
	stage1_1 = 0,
	stage1_2,
	stage1_3,
	stage1_boss,
	stage2_1,
	stage2_2,
	stage2_3,
	stage2_boss,
	stage3_boss
};
class Monster
{
private:
	int hp;
	int mp;
	int attack;
	MonType type;


public:
	void set_hp(const int& damage);
	void set_mp(const int& use_mp);
	void set_attack(const int& increase);

	int get_hp() { return hp; }
	int get_mp() { return mp; }
	int get_attack() { return attack; }
	int get_Mon_type() { return type; }


};

