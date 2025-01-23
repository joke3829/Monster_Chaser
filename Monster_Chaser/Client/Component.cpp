#include "Component.h"

void Component::SetActive(bool isActive)
{
	if (isActive)
	{
		OnEnable();
	}
	else {
		if (!m_bActive) {
			return;
		}

		OnDisable();
	}
}

void Component::FirstUpdate()
{
	if (!m_bActive) {
		return;
	}

	SetActive(true);

	m_UpdateFunc = &Component::Update;
	Update();
}
