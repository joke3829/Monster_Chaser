#include "stdafx.h"
#include "Monster.h"

void Monster::set_hp(const int& damage)
{
	{
		if (hp - damage > 0)
			hp -= damage;
		else
			hp = 0;


	}
}

void Monster::set_mp(const int& use_mp)
{
	{
		if (mp - use_mp > 0)
			mp -= use_mp;
		else
			mp = 0;


	}
}

void Monster::set_attack(const int& increase)
{
	attack += increase;
}
