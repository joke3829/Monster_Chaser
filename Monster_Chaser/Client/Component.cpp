#include "Component.h"

void Component::SetActive(bool isActive)
{
	if (isActive)
	{
		OnEnable();
	}
	else {
		if (!bActive) {
			return;
		}

		OnDisable();
	}
}

void Component::FirstUpdate()
{
	if (!bActive) {
		return;
	}

	SetActive(true);

	m_UpdateFunc = &Component::Update;
	Update();
}
