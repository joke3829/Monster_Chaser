


using namespace std;
enum character {
    warrior = 0,
    mage,
    supplier
};

// Base: Job
class Job {
protected:
    character name;
    short hp;
    short Attack;
    short damage_ratio;
    float skill_recovery;
    short skill_cost;
    float range;

public:
  
    // setter
    void set_attack(short& a);
    void set_hp(short& h);
    void set_damage_ratio(short& dr);
    void set_skill_recovery(float& sr);
    void set_skill_cost(short& sc);
    void set_range(float& r);


    short get_hp() { return hp; }
    short get_attack() { return Attack; }
    short get_damage_ratio() { return damage_ratio; }
    float get_skill_recovery() { return skill_recovery; }
    short get_skill_cost() { return skill_cost; }
    float get_range() { return range; }



  
};




