#include "stdafx.h"
#include "Character.h"

void Job::set_attack(short& a)
{
    Attack += a;
}

void Job::set_hp(short& h)
{
    hp += h;
}

void Job::set_damage_ratio(short& dr)
{
    damage_ratio += dr;
}

void Job::set_skill_cost(short& sc)
{
    skill_cost += sc;
}

void Job::set_range(float& r)
{
    range += r;
}

void Job::set_skill_recovery(float& sr)
{
    skill_recovery += sr;
}



