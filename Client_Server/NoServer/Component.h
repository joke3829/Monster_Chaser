////-----------------------------------------------------------------------------
//// File: Component.h
//// 모든 구성 요소의 기본적인 구조와 동작 방식을 정의하는 클래스
//// 
//// 추가 기능은 개발 진행에 따라 확장 예정
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
//	// 객체 활성화 시 호출
//	virtual void OnEnable()
//	{
//		bActive = true;
//	}
//	// 객체 비활성화 시 호출
//	virtual void OnDisable()
//	{
//		bActive = false;
//	}
//
//	//매 프레임 호출
//	virtual void Update() {};
//	virtual void Animate() {};
//	
//	//소멸 시 호출
//	virtual void Destroy() { SetActive(false); };
//	
//	//동적 리소스 해제
//	virtual void Release() {};
//
//private:
//	void FirstUpdate(); //초기 업데이트
//};