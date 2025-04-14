


using namespace std;


// �θ� Ŭ����: Job
class Job {
protected:
    ���� name;
    vec3 cur_loc;
    vec2 cur_rot;
    short hp;
    short Attack;
    short damage_ratio;
    float skill_recovery;
    short skill_cost;
    float range;

public:
    Job(���� name, short hp, short attack_power, short damage_ratio, double skill_recovery, short skill_cost, float range)
        : name(name), hp(hp), Attack(attack_power), damage_ratio(damage_ratio),
        skill_recovery(skill_recovery), skill_cost(skill_cost), range(range) {}

    // ���� �Լ�
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



   vec3 getLoc();				// ���� ��ġ �޾ƿ���	EYE
   vec2 getRot();				// ���� �ٶ󺸴� ���� �޾ƿ���	AT
    void setLoc(vec3& Pos);				// ��ġ �����ϱ�	EYE
    void setRot(vec2& At);				//  ���� �����ϱ�	AT

    virtual void displayInfo() const;
    virtual ~Job() {} // ���� �Ҹ���
};

// ���� Ŭ����
class Warrior : public Job {
public:
    Warrior() : Job(����, 1200, 50, 75, 3.0, 100, 1.6f) {}
};

// ������ Ŭ����
class Mage : public Job {
public:
    Mage() : Job(������, 800, 90, 125, 2.0, 100, 5.0f) {}
};

// ������ Ŭ����
class Cleric : public Job {
public:
    Cleric() : Job(������, 1000, 25, 100, 2.5, 100, 7.0f) {}
};


