


using namespace std;


// 부모 클래스: Job
class Job {
protected:
    직업 name;
    vec3 cur_loc;
    vec2 cur_rot;
    short hp;
    short Attack;
    short damage_ratio;
    float skill_recovery;
    short skill_cost;
    float range;

public:
    Job(직업 name, short hp, short attack_power, short damage_ratio, double skill_recovery, short skill_cost, float range)
        : name(name), hp(hp), Attack(attack_power), damage_ratio(damage_ratio),
        skill_recovery(skill_recovery), skill_cost(skill_cost), range(range) {}

    // 세터 함수
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



   vec3 getLoc();				// 현재 위치 받아오기	EYE
   vec2 getRot();				// 현재 바라보는 방향 받아오기	AT
    void setLoc(vec3& Pos);				// 위치 설정하기	EYE
    void setRot(vec2& At);				//  방향 설정하기	AT

    virtual void displayInfo() const;
    virtual ~Job() {} // 가상 소멸자
};

// 전사 클래스
class Warrior : public Job {
public:
    Warrior() : Job(전사, 1200, 50, 75, 3.0, 100, 1.6f) {}
};

// 마법사 클래스
class Mage : public Job {
public:
    Mage() : Job(마법사, 800, 90, 125, 2.0, 100, 5.0f) {}
};

// 성직자 클래스
class Cleric : public Job {
public:
    Cleric() : Job(성직자, 1000, 25, 100, 2.5, 100, 7.0f) {}
};


