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

vec3 Job::getLoc()
{
    return cur_loc;
}

vec2 Job::getRot()
{
    return cur_rot;
}

void Job::setLoc(vec3& Pos)
{
    cur_loc = Pos;
}

void Job::setRot(vec2& At)
{
    cur_rot = At;
}

void Job::displayInfo() const
{
        cout << "직업: " << (name == 전사 ? "전사" : name == 마법사 ? "마법사" : "성직자") << endl;
        cout << "HP: " << hp << endl;
        cout << "공격력: " << Attack << endl;
        cout << "피해 배율: " << damage_ratio << "%" << endl;
        cout << "스킬 코스트 회복량: " << skill_recovery << "/초" << endl;
        cout << "전체 스킬 코스트: " << skill_cost << endl;
        cout << "사거리: " << range << "M" << endl;
        cout << "-----------------------------" << endl;
}
