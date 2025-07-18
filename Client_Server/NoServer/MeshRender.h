//-----------------------------------------------------------------------------
// File: MeshRender.h
// 객체가 사용하는 모든 그래픽 리소스를 관리하고 렌더링을 처리하는 클래스
// 
// 추가적인 기능은 개발 진행에 따라 확장 예정
//-----------------------------------------------------------------------------
#include "Component.h"
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"

class MeshRender : public Component
{
public:
	virtual ~MeshRender() = default;

	template<class T>
	void SetMesh(std::shared_ptr<T>& mesh) { m_Mesh = mesh; }
	template<class T>
	void SetShader(std::shared_ptr<T>& shader) { m_Shader = shader; }
	template<class T>
	void SetMaterial(std::shared_ptr<T>& material) { m_Material = material; }

	std::shared_ptr<Mesh> GetMesh() { return m_Mesh; }
	std::shared_ptr<Shader> GetShader() { return m_Shader; }
	std::shared_ptr<Material> GetMaterial() { return m_Material; }
private:
	std::shared_ptr<Mesh> m_Mesh;
	std::shared_ptr<Shader> m_Shader;
	std::shared_ptr<Material> m_Material;
};

/*
	이렇게 사용을 할 거 같다
	
	Object* a;
	auto me = std::make_shared<Mesh>();
	auto meshRender = a->AddComponent<MeshRender>();
	meshRender->SetMesh(me);

	auto trans = a->AddComponent<Transform>();

*/