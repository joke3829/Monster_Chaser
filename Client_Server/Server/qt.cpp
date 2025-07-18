#include "qt.h"

bool Intersect(const AABB& aabb, const XMFLOAT2& center, float halfSize) {
    float nodeMinX = center.x - halfSize;
    float nodeMaxX = center.x + halfSize;
    float nodeMinZ = center.y - halfSize;
    float nodeMaxZ = center.y + halfSize;

    return !(aabb.max.x < nodeMinX || aabb.min.x > nodeMaxX ||
        aabb.max.z < nodeMinZ || aabb.min.z > nodeMaxZ);
}

QuadTreeNode::QuadTreeNode(const XMFLOAT2& c, float hs, int d) : center(c), halfSize(hs), depth(d) {}

void QuadTreeNode::Insert(const AABB& box) {
    if (!Intersect(box, center, halfSize)) return;

    if (depth == MAX_DEPTH) {
        objects.push_back(box);
        return;
    }

    if (isLeaf) {
        float offset = halfSize / 2.0f;
        for (int i = 0; i < 4; ++i) {
            float dx = (i % 2 == 0 ? -1 : 1) * offset;
            float dz = (i < 2 ? -1 : 1) * offset;
            XMFLOAT2 newCenter = { center.x + dx, center.y + dz };
            children[i] = std::make_shared<QuadTreeNode>(newCenter, offset, depth + 1);
        }
        isLeaf = false;
    }

    for (auto& child : children)
        child->Insert(box);
}

void QuadTree::Build(const std::vector<AABB>& allObjects) {
    root = std::make_shared<QuadTreeNode>(XMFLOAT2(0.0f, 0.0f), 1024.0f, 0);  // 중심이 (0,0), 전체 크기 2048
    for (const auto& obj : allObjects)
        root->Insert(obj);
}

std::vector<AABB> QuadTree::QueryRange(const XMFLOAT3& point) {
    std::vector<AABB> result;
    std::vector<std::shared_ptr<QuadTreeNode>> stack = { root };

    while (!stack.empty()) {
        auto node = stack.back();
        stack.pop_back();

        if (!Intersect({ point, point }, node->center, node->halfSize)) continue;

        for (auto& obj : node->objects)
            result.push_back(obj);

        if (!node->isLeaf)
            for (auto& child : node->children)
                stack.push_back(child);
    }
   
    return result;
}


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
