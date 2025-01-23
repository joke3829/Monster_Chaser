#include "MeshRender.h"
#include "Transform.h"

class Object
{
public:
	Object() {};
	virtual ~Object() = default;
private:
	//컴포넌트
	std::vector<std::shared_ptr<Component>> m_Component;
public:
	//컴포넌트 추가
	template<typename T>
	std::shared_ptr<T>AddComponent() {
		std::shared_ptr<T> comp = std::make_shared<T>;
		m_Component.push_back(comp);
		return comp;
	};

};