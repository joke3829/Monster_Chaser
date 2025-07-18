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



//// QuadTree.h
//#pragma once
//#include <vector>
//#include <memory>
//#include <DirectXMath.h>
//
//using namespace DirectX;
//
//constexpr int MAX_DEPTH = 4;
//
//struct AABB {
//    XMFLOAT3 min;
//    XMFLOAT3 max;
//
//    bool intersects(const AABB& other) const {
//        return !(max.x < other.min.x || min.x > other.max.x ||
//            max.z < other.min.z || min.z > other.max.z);
//    }
//};
//
//struct GameObject {
//    int id;
//    XMFLOAT3 pos;
//    float width, depth;
//
//    AABB getAABB() const {
//        return AABB{
//            { pos.x - width * 0.5f, pos.y, pos.z - depth * 0.5f },
//            { pos.x + width * 0.5f, pos.y, pos.z + depth * 0.5f }
//        };
//    }
//};
//
//class QuadTreeNode {
//public:
//    QuadTreeNode(XMFLOAT2 c, float hs, int d);
//    void insert(const GameObject& obj);
//    void query(const AABB& range, std::vector<const GameObject*>& out);
//
//    XMFLOAT2 center;
//    float halfSize;
//    int depth;
//    std::vector<GameObject> objects;
//    std::shared_ptr<QuadTreeNode> children[4];
//    bool isLeaf = true;
//};
//
//class QuadTree {
//public:
//    void build(const std::vector<GameObject>& allObjects);
//    void query(const AABB& range, std::vector<const GameObject*>& out) const;
//
//    std::shared_ptr<QuadTreeNode> root;
//};
