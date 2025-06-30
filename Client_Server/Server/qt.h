#pragma once
#include <vector>
#include <memory>
#include <DirectXMath.h>
using namespace DirectX;

constexpr int MAX_DEPTH = 4;  // 2^4 x 2^4 = 16x16 분할

struct AABB {
    XMFLOAT3 min;
    XMFLOAT3 max;
};

class QuadTreeNode {
public:
    XMFLOAT2 center; // 중심 좌표 (x, z)
    float halfSize;
    int depth;
    std::vector<AABB> objects;
    std::shared_ptr<QuadTreeNode> children[4];
    bool isLeaf = true;

    QuadTreeNode(const XMFLOAT2& c, float hs, int d);
    void Insert(const AABB& box);
};

class QuadTree {
public:
    std::shared_ptr<QuadTreeNode> root;
    void Build(const std::vector<AABB>& allObjects);
    std::vector<AABB> QueryRange(const XMFLOAT3& point);  // 포인트 주변 오브젝트 탐색
};
