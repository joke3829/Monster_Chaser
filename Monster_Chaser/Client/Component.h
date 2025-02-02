////-----------------------------------------------------------------------------
//// File: Component.h
//// ��� ���� ����� �⺻���� ������ ���� ����� �����ϴ� Ŭ����
//// 
//// �߰� ����� ���� ���࿡ ���� Ȯ�� ����
////-----------------------------------------------------------------------------
//#include "stdafx.h"
//
//class Component : public std::enable_shared_from_this<Component>
//{
//public:
//	virtual ~Component() = default;
//
//private:
//	bool bActive = false;
//
//	void (Component::* m_UpdateFunc)() { &Component::FirstUpdate };
//	void UpdateFunc() { (this->*m_UpdateFunc)(); }
//
//public:
//	void SetActive(bool isActive);
//
//private:
//	// ��ü Ȱ��ȭ �� ȣ��
//	virtual void OnEnable()
//	{
//		bActive = true;
//	}
//	// ��ü ��Ȱ��ȭ �� ȣ��
//	virtual void OnDisable()
//	{
//		bActive = false;
//	}
//
//	//�� ������ ȣ��
//	virtual void Update() {};
//	virtual void Animate() {};
//	
//	//�Ҹ� �� ȣ��
//	virtual void Destroy() { SetActive(false); };
//	
//	//���� ���ҽ� ����
//	virtual void Release() {};
//
//private:
//	void FirstUpdate(); //�ʱ� ������Ʈ
//};