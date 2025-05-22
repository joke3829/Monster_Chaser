//-----------------------------------------------------------------------------
// File: MeshRender.h
// Not Used
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
	Use example
	
	Object* a;
	auto me = std::make_shared<Mesh>();
	auto meshRender = a->AddComponent<MeshRender>();
	meshRender->SetMesh(me);

	auto trans = a->AddComponent<Transform>();

*/