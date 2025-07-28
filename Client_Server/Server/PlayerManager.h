#pragma once
#include "Player.h"

class PlayerManager {
public:
    // �÷��̾� �߰� �� ����
    void AddPlayer(int id, std::shared_ptr<Player> player);
    void RemovePlayer(int id);

    // ������ ����
    void ApplyDamage(int id, int dmg);

    // ��ġ ����
    void SetPosition(int id, const XMFLOAT4X4& pos);
    void SetBoganPostion(int id, const XMFLOAT4X4& pos);
    std::shared_ptr<Player> GetPlayer(int id) const;

private:
    std::unordered_map<int, std::shared_ptr<Player>> players;
    mutable  std::mutex managerMutex; // ��ü player map ��ȣ const�ɹ� �Լ������� ���ؽ�ó�� ��� ���� 
};

