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
        cout << "����: " << (name == ���� ? "����" : name == ������ ? "������" : "������") << endl;
        cout << "HP: " << hp << endl;
        cout << "���ݷ�: " << Attack << endl;
        cout << "���� ����: " << damage_ratio << "%" << endl;
        cout << "��ų �ڽ�Ʈ ȸ����: " << skill_recovery << "/��" << endl;
        cout << "��ü ��ų �ڽ�Ʈ: " << skill_cost << endl;
        cout << "��Ÿ�: " << range << "M" << endl;
        cout << "-----------------------------" << endl;
}
