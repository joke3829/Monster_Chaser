////-----------------------------------------------------------------------------
//// File: Component.h
//// Not Used
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
//	virtual void OnEnable()
//	{
//		bActive = true;
//	}
//	virtual void OnDisable()
//	{
//		bActive = false;
//	}
//
//	virtual void Update() {};
//	virtual void Animate() {};
//	
//	virtual void Destroy() { SetActive(false); };
//	
//	virtual void Release() {};
//
//private:
//	void FirstUpdate();
//};