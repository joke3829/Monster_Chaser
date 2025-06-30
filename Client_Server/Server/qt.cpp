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
