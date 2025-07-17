#pragma once
#include "stdafx.h"
#include "protocol.h"
#include <mutex>
#include "GameObject.h"
#include "AnimationManager.h"

class ObjectManager {
public:
    ObjectManager() {}
    ObjectManager(int id);
    virtual ~ObjectManager() = default;

    int getID() const;

    void setMatrix(const XMFLOAT4X4& pos);
    XMFLOAT3 getPosition() const;

    void setRenderingObject(CSkinningObject* obj) { SkinningObject = obj; }
    void setAnimationManager(CAnimationManager* ani) { AnimationManager = ani; }

    CSkinningObject* getRenderingObject() { return SkinningObject; }
    CAnimationManager* getAnimationManager() { return AnimationManager; }

    void setPosition(const XMFLOAT4X4& pos) {
        setMatrix(pos);
        if (SkinningObject) {
            XMFLOAT3 pos3 = { pos._41, pos._42 ,pos._43 };
            SkinningObject->SetPosition(pos3);
        }
    }

    void setVisible(bool visible) {
        /*if (SkinningObject)
            SkinningObject->SetVisible(visible);*/
    }

    void playIdleAnim() {
        /*if (AnimationManager)
            AnimationManager->Play("Idle", true);*/
    }

protected:
    int my_id;
    XMFLOAT4X4 m_Matrix;

    CSkinningObject* SkinningObject = nullptr;
    CAnimationManager* AnimationManager = nullptr;
};

class Player : public ObjectManager {
public:
    Player() {}
    Player(int id) : ObjectManager(id) {}

    bool getReady() const { return readyToStart; }
    void setReady(const bool& ready) { readyToStart = ready; }

    void setCharacterType(const short t) { type = t; }
    short getCharacterType() { return type; }

private:
    bool readyToStart = false;
    int hp = 100;
    short type{ JOB_NOTHING };
};

class Monster : public ObjectManager {
public:
    Monster(int id) : ObjectManager(id) {}

    void setTargetID(int tid) { m_targetID = tid; }
    int getTargetID() const { return m_targetID; }

private:
    int m_targetID = -1;
    int hp = 300;
    short type{ JOB_NOTHING };
};
