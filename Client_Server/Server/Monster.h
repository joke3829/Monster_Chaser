#pragma once

class Monster
{
private:
	int hp;
	int mp;
	int attack;
	char Mon_type;


public:
	void set_hp(const int& damage);
	void set_mp(const int& use_mp);
	void set_attack(const int& increase);

	int get_hp() { return hp; }
	int get_mp() { return mp; }
	int get_attack() { return attack; }
	int get_Mon_type() {return Mon_type;}


};

