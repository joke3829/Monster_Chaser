#include "PlayableCharacter.h"

CPlayableCharacter::CPlayableCharacter(CSkinningObject* object, CAnimationManager* aManager)
	: m_Object(object)
{
	m_AManager = dynamic_cast<CPlayableCharacterAnimationManager*>(aManager);
}

// =======================================================================================

void CPlayerMage::Skill1()
{
	XMFLOAT3 cameraDir = m_pCamera->getDir();
	XMFLOAT3 characterDir = cameraDir;
	characterDir.y = 0.0f; // delete y value
	m_AManager->ChangeAnimation(ANI_SKILL1, true);
	m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_AManager->UpdateAniPosition(0.0f, m_Object);
	m_bLockAnimation1 = true;
}

void CPlayerMage::Skill2()
{
	XMFLOAT3 cameraDir = m_pCamera->getDir();
	XMFLOAT3 characterDir = cameraDir;
	characterDir.y = 0.0f; // delete y value
	m_AManager->ChangeAnimation(ANI_SKILL2, true);
	m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_AManager->UpdateAniPosition(0.0f, m_Object);
	m_bLockAnimation1 = true;
}

void CPlayerMage::Skill3()
{
	XMFLOAT3 cameraDir = m_pCamera->getDir();
	XMFLOAT3 characterDir = cameraDir;
	characterDir.y = 0.0f; // delete y value
	m_AManager->OnKey3Input();
	m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_AManager->UpdateAniPosition(0.0f, m_Object);
	m_bLockAnimation = true;
}

CPlayerMage::CPlayerMage(CSkinningObject* object, CAnimationManager* aManager)
	: CPlayableCharacter(object, aManager)
{
	m_HP = 800.0f;
	m_MP = 100.0f;
}

void CPlayerMage::MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	if (!mouseIsInitialize) {
		ShowCursor(FALSE);  // hide cursor
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		POINT center;
		center.x = (clientRect.right - clientRect.left) / 2;
		center.y = (clientRect.bottom - clientRect.top) / 2;
		oldCursor = center;
		ClientToScreen(hWnd, &oldCursor);
		SetCursorPos(oldCursor.x, oldCursor.y);
		mouseIsInitialize = true;
	}
	POINT currentPos;
	GetCursorPos(&currentPos);
	XMFLOAT3 cameraDir = m_pCamera->getDir();
	switch (nMessage) {
	case WM_LBUTTONDOWN:
	{
		if (!m_bLockAnimation && !m_bLockAnimation1) {
			XMFLOAT3 characterDir = cameraDir;
			characterDir.y = 0.0f; // delete y value
			m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
			m_AManager->UpdateAniPosition(0.0f, m_Object);
			m_AManager->OnAttackInput();
			/*bullet.setPosition(m_pResourceManager->getSkinningObjectList()[0]->getPosition());
			bullet.setMoveDirection(characterDir);
			bullet.setActive(true);
			bullet.setTime(0.0f);*/
			m_bDoingCombo = true;
		}
		break;
	}
	case WM_MOUSEMOVE:
	{
		// mouse moving amount
		float deltaX = static_cast<float>(currentPos.x - oldCursor.x);
		float deltaY = static_cast<float>(currentPos.y - oldCursor.y);

		if (deltaX != 0.0f || deltaY != 0.0f) {
			if (m_pCamera->getThirdPersonState()) {
				m_pCamera->Rotate(deltaX * 1.5f, -deltaY * 1.5f);
				CGameObject* frame = m_AManager->getFrame()[0];
				if (!m_bLockAnimation && !m_bLockAnimation1 && !m_bDoingCombo && !m_AManager->IsInCombo() && !m_AManager->IsAnimationFinished()) {
					m_Object->Rotation(XMFLOAT3(0.0f, deltaX * 0.5f, 0.0f), *frame);
					XMFLOAT3 characterDir = cameraDir;
					characterDir.y = 0.0f; // delete y value
					m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
				}
			}
			else
				m_pCamera->Rotate(deltaX * 1.5f, deltaY * 1.5f);

			SetCursorPos(oldCursor.x, oldCursor.y);
			break;
		}
	}
	}
}

void CPlayerMage::ProcessInput(UCHAR* keyBuffer, float fElapsedTime)
{
	XMFLOAT3 cameraDir = m_pCamera->getDir();
	XMFLOAT3 characterDir = cameraDir;
	characterDir.y = 0.0f; // delete y value
	m_bMoving = false;
	XMFLOAT3 normalizedCharacterDir = characterDir;
	XMStoreFloat3(&normalizedCharacterDir, XMVector3Normalize(XMLoadFloat3(&normalizedCharacterDir)));
	XMFLOAT3 moveDir{};

	if (m_bLockAnimation && !m_AManager->IsInCombo()) {
		m_bLockAnimation = false;
	}

	if (m_bLockAnimation || m_bLockAnimation1 || m_bDoingCombo) {
		memset(m_PrevKeyBuffer, 0, sizeof(m_PrevKeyBuffer));
		return;
	}

	// Handle single and combined key presses
	if (keyBuffer['W'] & 0x80 && keyBuffer['A'] & 0x80) {
		moveDir = XMFLOAT3(normalizedCharacterDir.x - normalizedCharacterDir.z, 0.0f, normalizedCharacterDir.z + normalizedCharacterDir.x);
		m_bMoving = true;
	}
	else if (keyBuffer['W'] & 0x80 && keyBuffer['D'] & 0x80) {
		moveDir = XMFLOAT3(normalizedCharacterDir.x + normalizedCharacterDir.z, 0.0f, normalizedCharacterDir.z - normalizedCharacterDir.x);
		m_bMoving = true;
	}
	else if (keyBuffer['S'] & 0x80 && keyBuffer['A'] & 0x80) {
		moveDir = XMFLOAT3(-normalizedCharacterDir.x - normalizedCharacterDir.z, 0.0f, -normalizedCharacterDir.z + normalizedCharacterDir.x);
		m_bMoving = true;
	}
	else if (keyBuffer['S'] & 0x80 && keyBuffer['D'] & 0x80) {
		moveDir = XMFLOAT3(-normalizedCharacterDir.x + normalizedCharacterDir.z, 0.0f, -normalizedCharacterDir.z - normalizedCharacterDir.x);
		m_bMoving = true;
	}
	else if (keyBuffer['W'] & 0x80) {
		moveDir = normalizedCharacterDir;
		m_bMoving = true;
	}
	else if (keyBuffer['S'] & 0x80) {
		moveDir = XMFLOAT3(-normalizedCharacterDir.x, 0.0f, -normalizedCharacterDir.z);
		m_bMoving = true;
	}
	else if (keyBuffer['A'] & 0x80) {
		moveDir = XMFLOAT3(-normalizedCharacterDir.z, 0.0f, normalizedCharacterDir.x);
		m_bMoving = true;
	}
	else if (keyBuffer['D'] & 0x80) {
		moveDir = XMFLOAT3(normalizedCharacterDir.z, 0.0f, -normalizedCharacterDir.x);
		m_bMoving = true;
	}

	if (m_bMoving) {
		XMStoreFloat3(&moveDir, XMVector3Normalize(XMLoadFloat3(&moveDir)));
		m_Object->SetMoveDirection(moveDir);
	}

    if ((m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_UP, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_UP, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_DOWN, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_DOWN, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_FORWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_FORWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_FORWARD, true);
            }
        }
        }
    else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_FORWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_FORWARD, true);
            }
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_BACKWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_BACKWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_BACKWARD, true);
            }
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_BACKWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_BACKWARD, true);
            }
        }
    }
    else if ((keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_LEFT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_LEFT, true);
            }
        }
    }
    else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['A'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['A'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_LEFT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_LEFT, true);
            }
        }
    }
    else if ((keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        if (!(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_RIGHT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_RIGHT, true);
            }
        }
    }
    else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['D'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        if (!(m_PrevKeyBuffer['D'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_RIGHT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_RIGHT, true);
            }
        }
    }
    else if ((m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if (!(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (m_AManager->IsAnimationNearEnd()) {
            m_AManager->ChangeAnimation(ANI_IDLE, false);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = false;
        }
    }

	if (!m_bLockAnimation && !m_bLockAnimation1 && !m_bDoingCombo) {
		if ((keyBuffer['J'] & 0x80) && !(m_PrevKeyBuffer['J'] & 0x80)) {
			m_AManager->ChangeAnimation(ANI_HIT, true);
			m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
			m_AManager->UpdateAniPosition(0.0f, m_Object);
			m_bLockAnimation1 = true;
		}
		if ((keyBuffer['K'] & 0x80) && !(m_PrevKeyBuffer['K'] & 0x80)) {
			m_AManager->ChangeAnimation(ANI_HIT_DEATH, true);
			m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
			m_AManager->UpdateAniPosition(0.0f, m_Object);
			m_bLockAnimation1 = true;
		}
		if ((keyBuffer[VK_SPACE] & 0x80) && !(m_PrevKeyBuffer[VK_SPACE] & 0x80)) {
			XMFLOAT3 dodgeDir = characterDir;

			if (keyBuffer['W'] & 0x80 && keyBuffer['A'] & 0x80) {
				dodgeDir = XMFLOAT3(characterDir.x - characterDir.z, 0.0f, characterDir.z + characterDir.x);
			}
			else if (keyBuffer['W'] & 0x80 && keyBuffer['D'] & 0x80) {
				dodgeDir = XMFLOAT3(characterDir.x + characterDir.z, 0.0f, characterDir.z - characterDir.x);
			}
			else if (keyBuffer['S'] & 0x80 && keyBuffer['A'] & 0x80) {
				dodgeDir = XMFLOAT3(-characterDir.x - characterDir.z, 0.0f, -characterDir.z + characterDir.x);
			}
			else if (keyBuffer['S'] & 0x80 && keyBuffer['D'] & 0x80) {
				dodgeDir = XMFLOAT3(-characterDir.x + characterDir.z, 0.0f, -characterDir.z - characterDir.x);
			}
			else if (keyBuffer['W'] & 0x80) {
				dodgeDir = characterDir;
			}
			else if (keyBuffer['S'] & 0x80) {
				dodgeDir = XMFLOAT3(-characterDir.x, 0.0f, -characterDir.z);
			}
			else if (keyBuffer['A'] & 0x80) {
				dodgeDir = XMFLOAT3(-characterDir.z, 0.0f, characterDir.x);
			}
			else if (keyBuffer['D'] & 0x80) {
				dodgeDir = XMFLOAT3(characterDir.z, 0.0f, -characterDir.x);
			}

			XMStoreFloat3(&dodgeDir, XMVector3Normalize(XMLoadFloat3(&dodgeDir)));

			m_AManager->ChangeAnimation(ANI_DODGE, true);
			m_Object->SetLookDirection(dodgeDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
			m_AManager->UpdateAniPosition(0.0f, m_Object);
			m_bLockAnimation1 = true;
		}
		if ((keyBuffer['L'] & 0x80) && !(m_PrevKeyBuffer['L'] & 0x80)) {
			m_AManager->ChangeAnimation(ANI_BIGHIT, true);
			m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
			m_AManager->UpdateAniPosition(0.0f, m_Object);
			m_bLockAnimation1 = true;
		}
		if ((keyBuffer['U'] & 0x80) && !(m_PrevKeyBuffer['U'] & 0x80)) {
			m_AManager->ChangeAnimation(ANI_BIGHIT_DEATH, true);
			m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
			m_AManager->UpdateAniPosition(0.0f, m_Object);
			m_bLockAnimation1 = true;
		}
		if ((keyBuffer['1'] & 0x80) && !(m_PrevKeyBuffer['1'] & 0x80)) {
			Skill1();
		}
		if ((keyBuffer['2'] & 0x80) && !(m_PrevKeyBuffer['2'] & 0x80)) {
			Skill2();
		}
		if ((keyBuffer['3'] & 0x80) && !(m_PrevKeyBuffer['3'] & 0x80)) {
			Skill3();
		}
	}
	memcpy(m_PrevKeyBuffer, keyBuffer, sizeof(m_PrevKeyBuffer));
}

void CPlayerMage::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_AManager->UpdateCombo(fElapsedTime);
	if (!m_AManager->IsInCombo() && m_AManager->IsAnimationFinished()) {
		m_AManager->ChangeAnimation(ANI_IDLE, false);
		test = true;
		m_bLockAnimation1 = false;
		m_bLockAnimation = false;
		m_bDoingCombo = false;
	}
	if (m_AManager->IsComboInterrupted()) {
		test = true;
		m_AManager->ClearComboInterrupted();
		m_bLockAnimation1 = false;
		m_bLockAnimation = false;
		m_bDoingCombo = false;
	}

	if (test) {
		m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
	}
}

// =======================================================================================

void CPlayerWarrior::Skill1()
{
}

void CPlayerWarrior::Skill2()
{
}

void CPlayerWarrior::Skill3()
{
}

CPlayerWarrior::CPlayerWarrior(CSkinningObject* object, CAnimationManager* aManager)
    : CPlayableCharacter(object, aManager)
{
    m_HP = 1200.0f;
    m_MP = 100.0f;
}

void CPlayerWarrior::MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
    if (!mouseIsInitialize) {
        ShowCursor(FALSE);  // hide cursor
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        POINT center;
        center.x = (clientRect.right - clientRect.left) / 2;
        center.y = (clientRect.bottom - clientRect.top) / 2;
        oldCursor = center;
        ClientToScreen(hWnd, &oldCursor);
        SetCursorPos(oldCursor.x, oldCursor.y);
        mouseIsInitialize = true;
    }
    POINT currentPos;
    GetCursorPos(&currentPos);
    XMFLOAT3 cameraDir = m_pCamera->getDir();
    switch (nMessage) {
    case WM_LBUTTONDOWN:
    {
        if (!m_bLockAnimation && !m_bLockAnimation1) {
            XMFLOAT3 characterDir = cameraDir;
            characterDir.y = 0.0f; // delete y value
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_AManager->OnAttackInput();
            m_bDoingCombo = true;
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        // mouse moving amount
        float deltaX = static_cast<float>(currentPos.x - oldCursor.x);
        float deltaY = static_cast<float>(currentPos.y - oldCursor.y);

        if (deltaX != 0.0f || deltaY != 0.0f) {
            if (m_pCamera->getThirdPersonState()) {
                m_pCamera->Rotate(deltaX * 1.5f, -deltaY * 1.5f);
                CGameObject* frame = m_AManager->getFrame()[0];
                if (!m_bLockAnimation && !m_bLockAnimation1 && !m_bDoingCombo && !m_AManager->IsInCombo() && !m_AManager->IsAnimationFinished()) {
                    m_Object->Rotation(XMFLOAT3(0.0f, deltaX * 0.5f, 0.0f), *frame);
                    XMFLOAT3 characterDir = cameraDir;
                    characterDir.y = 0.0f; // delete y value
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                }
            }
            else
                m_pCamera->Rotate(deltaX * 1.5f, deltaY * 1.5f);

            SetCursorPos(oldCursor.x, oldCursor.y);
            break;
        }
    }
    }
}

void CPlayerWarrior::ProcessInput(UCHAR* keyBuffer, float fElapsedTime)
{
    XMFLOAT3 cameraDir = m_pCamera->getDir();
    XMFLOAT3 characterDir = cameraDir;
    characterDir.y = 0.0f; // delete y value
    m_bMoving = false;
    XMFLOAT3 normalizedCharacterDir = characterDir;
    XMStoreFloat3(&normalizedCharacterDir, XMVector3Normalize(XMLoadFloat3(&normalizedCharacterDir)));
    XMFLOAT3 moveDir{};

    if (m_bLockAnimation && !m_AManager->IsInCombo()) {
        m_bLockAnimation = false;
    }

    if (m_bLockAnimation || m_bLockAnimation1 || m_bDoingCombo) {
        memset(m_PrevKeyBuffer, 0, sizeof(m_PrevKeyBuffer));
        return;
    }

    // Handle single and combined key presses
    if (keyBuffer['W'] & 0x80 && keyBuffer['A'] & 0x80) {
        moveDir = XMFLOAT3(normalizedCharacterDir.x - normalizedCharacterDir.z, 0.0f, normalizedCharacterDir.z + normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['W'] & 0x80 && keyBuffer['D'] & 0x80) {
        moveDir = XMFLOAT3(normalizedCharacterDir.x + normalizedCharacterDir.z, 0.0f, normalizedCharacterDir.z - normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['S'] & 0x80 && keyBuffer['A'] & 0x80) {
        moveDir = XMFLOAT3(-normalizedCharacterDir.x - normalizedCharacterDir.z, 0.0f, -normalizedCharacterDir.z + normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['S'] & 0x80 && keyBuffer['D'] & 0x80) {
        moveDir = XMFLOAT3(-normalizedCharacterDir.x + normalizedCharacterDir.z, 0.0f, -normalizedCharacterDir.z - normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['W'] & 0x80) {
        moveDir = normalizedCharacterDir;
        m_bMoving = true;
    }
    else if (keyBuffer['S'] & 0x80) {
        moveDir = XMFLOAT3(-normalizedCharacterDir.x, 0.0f, -normalizedCharacterDir.z);
        m_bMoving = true;
    }
    else if (keyBuffer['A'] & 0x80) {
        moveDir = XMFLOAT3(-normalizedCharacterDir.z, 0.0f, normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['D'] & 0x80) {
        moveDir = XMFLOAT3(normalizedCharacterDir.z, 0.0f, -normalizedCharacterDir.x);
        m_bMoving = true;
    }

    if (m_bMoving) {
        XMStoreFloat3(&moveDir, XMVector3Normalize(XMLoadFloat3(&moveDir)));
        m_Object->SetMoveDirection(moveDir);
    }

    if ((m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_UP, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_UP, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_DOWN, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_DOWN, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_FORWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_FORWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_FORWARD, true);
            }
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_FORWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_FORWARD, true);
            }
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_BACKWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_BACKWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_BACKWARD, true);
            }
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_BACKWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_BACKWARD, true);
            }
        }
    }
    else if ((keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_LEFT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_LEFT, true);
            }
        }
    }
    else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['A'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['A'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_LEFT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_LEFT, true);
            }
        }
    }
    else if ((keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        if (!(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_RIGHT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_RIGHT, true);
            }
        }
    }
    else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['D'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        if (!(m_PrevKeyBuffer['D'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_RIGHT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_RIGHT, true);
            }
        }
    }
    else if ((m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if (!(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (m_AManager->IsAnimationNearEnd()) {
            m_AManager->ChangeAnimation(ANI_IDLE, false);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = false;
        }
    }

    if (!m_bLockAnimation && !m_bLockAnimation1 && !m_bDoingCombo) {
        if ((keyBuffer['J'] & 0x80) && !(m_PrevKeyBuffer['J'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_HIT, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer['K'] & 0x80) && !(m_PrevKeyBuffer['K'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_HIT_DEATH, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer[VK_SPACE] & 0x80) && !(m_PrevKeyBuffer[VK_SPACE] & 0x80)) {
            XMFLOAT3 dodgeDir = characterDir;

            if (keyBuffer['W'] & 0x80 && keyBuffer['A'] & 0x80) {
                dodgeDir = XMFLOAT3(characterDir.x - characterDir.z, 0.0f, characterDir.z + characterDir.x);
            }
            else if (keyBuffer['W'] & 0x80 && keyBuffer['D'] & 0x80) {
                dodgeDir = XMFLOAT3(characterDir.x + characterDir.z, 0.0f, characterDir.z - characterDir.x);
            }
            else if (keyBuffer['S'] & 0x80 && keyBuffer['A'] & 0x80) {
                dodgeDir = XMFLOAT3(-characterDir.x - characterDir.z, 0.0f, -characterDir.z + characterDir.x);
            }
            else if (keyBuffer['S'] & 0x80 && keyBuffer['D'] & 0x80) {
                dodgeDir = XMFLOAT3(-characterDir.x + characterDir.z, 0.0f, -characterDir.z - characterDir.x);
            }
            else if (keyBuffer['W'] & 0x80) {
                dodgeDir = characterDir;
            }
            else if (keyBuffer['S'] & 0x80) {
                dodgeDir = XMFLOAT3(-characterDir.x, 0.0f, -characterDir.z);
            }
            else if (keyBuffer['A'] & 0x80) {
                dodgeDir = XMFLOAT3(-characterDir.z, 0.0f, characterDir.x);
            }
            else if (keyBuffer['D'] & 0x80) {
                dodgeDir = XMFLOAT3(characterDir.z, 0.0f, -characterDir.x);
            }

            XMStoreFloat3(&dodgeDir, XMVector3Normalize(XMLoadFloat3(&dodgeDir)));

            m_AManager->ChangeAnimation(ANI_DODGE, true);
            m_Object->SetLookDirection(dodgeDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer['L'] & 0x80) && !(m_PrevKeyBuffer['L'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_BIGHIT, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer['U'] & 0x80) && !(m_PrevKeyBuffer['U'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_BIGHIT_DEATH, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer['1'] & 0x80) && !(m_PrevKeyBuffer['1'] & 0x80)) {
            Skill1();
        }
        if ((keyBuffer['2'] & 0x80) && !(m_PrevKeyBuffer['2'] & 0x80)) {
            Skill2();
        }
        if ((keyBuffer['3'] & 0x80) && !(m_PrevKeyBuffer['3'] & 0x80)) {
            Skill3();
        }
    }
    memcpy(m_PrevKeyBuffer, keyBuffer, sizeof(m_PrevKeyBuffer));
}

void CPlayerWarrior::UpdateObject(float fElapsedTime)
{
    bool test = false;
    m_AManager->UpdateCombo(fElapsedTime);
    if (!m_AManager->IsInCombo() && m_AManager->IsAnimationFinished()) {
        m_AManager->ChangeAnimation(ANI_IDLE, false);
        test = true;
        m_bLockAnimation1 = false;
        m_bLockAnimation = false;
        m_bDoingCombo = false;
    }
    if (m_AManager->IsComboInterrupted()) {
        test = true;
        m_AManager->ClearComboInterrupted();
        m_bLockAnimation1 = false;
        m_bLockAnimation = false;
        m_bDoingCombo = false;
    }

    if (test) {
        m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
    }
}

// =======================================================================================

void CPlayerPriest::Skill1()
{
}

void CPlayerPriest::Skill2()
{
}

void CPlayerPriest::Skill3()
{
}

CPlayerPriest::CPlayerPriest(CSkinningObject* object, CAnimationManager* aManager)
    : CPlayableCharacter(object, aManager)
{
    m_HP = 1000.0f;
    m_MP = 100.0f;
}

void CPlayerPriest::MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
    if (!mouseIsInitialize) {
        ShowCursor(FALSE);  // hide cursor
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        POINT center;
        center.x = (clientRect.right - clientRect.left) / 2;
        center.y = (clientRect.bottom - clientRect.top) / 2;
        oldCursor = center;
        ClientToScreen(hWnd, &oldCursor);
        SetCursorPos(oldCursor.x, oldCursor.y);
        mouseIsInitialize = true;
    }
    POINT currentPos;
    GetCursorPos(&currentPos);
    XMFLOAT3 cameraDir = m_pCamera->getDir();
    switch (nMessage) {
    case WM_LBUTTONDOWN:
    {
        if (!m_bLockAnimation && !m_bLockAnimation1) {
            XMFLOAT3 characterDir = cameraDir;
            characterDir.y = 0.0f; // delete y value
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_AManager->OnAttackInput();
            m_bDoingCombo = true;
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        // mouse moving amount
        float deltaX = static_cast<float>(currentPos.x - oldCursor.x);
        float deltaY = static_cast<float>(currentPos.y - oldCursor.y);

        if (deltaX != 0.0f || deltaY != 0.0f) {
            if (m_pCamera->getThirdPersonState()) {
                m_pCamera->Rotate(deltaX * 1.5f, -deltaY * 1.5f);
                CGameObject* frame = m_AManager->getFrame()[0];
                if (!m_bLockAnimation && !m_bLockAnimation1 && !m_bDoingCombo && !m_AManager->IsInCombo() && !m_AManager->IsAnimationFinished()) {
                    m_Object->Rotation(XMFLOAT3(0.0f, deltaX * 0.5f, 0.0f), *frame);
                    XMFLOAT3 characterDir = cameraDir;
                    characterDir.y = 0.0f; // delete y value
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                }
            }
            else
                m_pCamera->Rotate(deltaX * 1.5f, deltaY * 1.5f);

            SetCursorPos(oldCursor.x, oldCursor.y);
            break;
        }
    }
    }
}

void CPlayerPriest::ProcessInput(UCHAR* keyBuffer, float fElapsedTime)
{
    XMFLOAT3 cameraDir = m_pCamera->getDir();
    XMFLOAT3 characterDir = cameraDir;
    characterDir.y = 0.0f; // delete y value
    m_bMoving = false;
    XMFLOAT3 normalizedCharacterDir = characterDir;
    XMStoreFloat3(&normalizedCharacterDir, XMVector3Normalize(XMLoadFloat3(&normalizedCharacterDir)));
    XMFLOAT3 moveDir{};

    if (m_bLockAnimation && !m_AManager->IsInCombo()) {
        m_bLockAnimation = false;
    }

    if (m_bLockAnimation || m_bLockAnimation1 || m_bDoingCombo) {
        memset(m_PrevKeyBuffer, 0, sizeof(m_PrevKeyBuffer));
        return;
    }

    // Handle single and combined key presses
    if (keyBuffer['W'] & 0x80 && keyBuffer['A'] & 0x80) {
        moveDir = XMFLOAT3(normalizedCharacterDir.x - normalizedCharacterDir.z, 0.0f, normalizedCharacterDir.z + normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['W'] & 0x80 && keyBuffer['D'] & 0x80) {
        moveDir = XMFLOAT3(normalizedCharacterDir.x + normalizedCharacterDir.z, 0.0f, normalizedCharacterDir.z - normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['S'] & 0x80 && keyBuffer['A'] & 0x80) {
        moveDir = XMFLOAT3(-normalizedCharacterDir.x - normalizedCharacterDir.z, 0.0f, -normalizedCharacterDir.z + normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['S'] & 0x80 && keyBuffer['D'] & 0x80) {
        moveDir = XMFLOAT3(-normalizedCharacterDir.x + normalizedCharacterDir.z, 0.0f, -normalizedCharacterDir.z - normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['W'] & 0x80) {
        moveDir = normalizedCharacterDir;
        m_bMoving = true;
    }
    else if (keyBuffer['S'] & 0x80) {
        moveDir = XMFLOAT3(-normalizedCharacterDir.x, 0.0f, -normalizedCharacterDir.z);
        m_bMoving = true;
    }
    else if (keyBuffer['A'] & 0x80) {
        moveDir = XMFLOAT3(-normalizedCharacterDir.z, 0.0f, normalizedCharacterDir.x);
        m_bMoving = true;
    }
    else if (keyBuffer['D'] & 0x80) {
        moveDir = XMFLOAT3(normalizedCharacterDir.z, 0.0f, -normalizedCharacterDir.x);
        m_bMoving = true;
    }

    if (m_bMoving) {
        XMStoreFloat3(&moveDir, XMVector3Normalize(XMLoadFloat3(&moveDir)));
        m_Object->SetMoveDirection(moveDir);
    }

    if ((m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_UP, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_UP, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_UP, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_UP, true);
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_DOWN, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_DOWN, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_DOWN, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
        }
        else {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_DOWN, true);
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_RUN_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_FORWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_FORWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_FORWARD, true);
            }
        }
    }
    else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['W'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_FORWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_FORWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_FORWARD, true);
            }
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_BACKWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_BACKWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_BACKWARD, true);
            }
        }
    }
    else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['S'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['S'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_BACKWARD, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_BACKWARD, true);
            }
        }
    }
    else if ((keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_LEFT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_LEFT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_LEFT, true);
            }
        }
    }
    else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['A'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (!(m_PrevKeyBuffer['A'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_LEFT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_LEFT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_LEFT, true);
            }
        }
    }
    else if ((keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        if (!(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_RUN_RIGHT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_RUN_RIGHT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_RUN_RIGHT, true);
            }
        }
    }
    else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
        m_bFirst = true;
    }
    else if ((keyBuffer['D'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
        if (!(m_PrevKeyBuffer['D'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_WALK_RIGHT_START, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = true;
        }
        else {
            if (m_bFirst) {
                if (m_AManager->IsAnimationNearEnd()) {
                    m_AManager->ChangeAnimation(ANI_WALK_RIGHT, true);
                    m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
                    m_AManager->UpdateAniPosition(0.0f, m_Object);
                    m_bFirst = false;
                }
            }
            else {
                m_AManager->ChangeAnimation(ANI_WALK_RIGHT, true);
            }
        }
    }
    else if ((m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_FORWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_BACKWARD_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_LEFT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if ((m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
        m_AManager->ChangeAnimation(ANI_WALK_RIGHT_STOP, false);
        m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
        m_AManager->UpdateAniPosition(0.0f, m_Object);
    }
    else if (!(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
        if (m_AManager->IsAnimationNearEnd()) {
            m_AManager->ChangeAnimation(ANI_IDLE, false);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bFirst = false;
        }
    }

    if (!m_bLockAnimation && !m_bLockAnimation1 && !m_bDoingCombo) {
        if ((keyBuffer['J'] & 0x80) && !(m_PrevKeyBuffer['J'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_HIT, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer['K'] & 0x80) && !(m_PrevKeyBuffer['K'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_HIT_DEATH, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer[VK_SPACE] & 0x80) && !(m_PrevKeyBuffer[VK_SPACE] & 0x80)) {
            XMFLOAT3 dodgeDir = characterDir;

            if (keyBuffer['W'] & 0x80 && keyBuffer['A'] & 0x80) {
                dodgeDir = XMFLOAT3(characterDir.x - characterDir.z, 0.0f, characterDir.z + characterDir.x);
            }
            else if (keyBuffer['W'] & 0x80 && keyBuffer['D'] & 0x80) {
                dodgeDir = XMFLOAT3(characterDir.x + characterDir.z, 0.0f, characterDir.z - characterDir.x);
            }
            else if (keyBuffer['S'] & 0x80 && keyBuffer['A'] & 0x80) {
                dodgeDir = XMFLOAT3(-characterDir.x - characterDir.z, 0.0f, -characterDir.z + characterDir.x);
            }
            else if (keyBuffer['S'] & 0x80 && keyBuffer['D'] & 0x80) {
                dodgeDir = XMFLOAT3(-characterDir.x + characterDir.z, 0.0f, -characterDir.z - characterDir.x);
            }
            else if (keyBuffer['W'] & 0x80) {
                dodgeDir = characterDir;
            }
            else if (keyBuffer['S'] & 0x80) {
                dodgeDir = XMFLOAT3(-characterDir.x, 0.0f, -characterDir.z);
            }
            else if (keyBuffer['A'] & 0x80) {
                dodgeDir = XMFLOAT3(-characterDir.z, 0.0f, characterDir.x);
            }
            else if (keyBuffer['D'] & 0x80) {
                dodgeDir = XMFLOAT3(characterDir.z, 0.0f, -characterDir.x);
            }

            XMStoreFloat3(&dodgeDir, XMVector3Normalize(XMLoadFloat3(&dodgeDir)));

            m_AManager->ChangeAnimation(ANI_DODGE, true);
            m_Object->SetLookDirection(dodgeDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer['L'] & 0x80) && !(m_PrevKeyBuffer['L'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_BIGHIT, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer['U'] & 0x80) && !(m_PrevKeyBuffer['U'] & 0x80)) {
            m_AManager->ChangeAnimation(ANI_BIGHIT_DEATH, true);
            m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f));
            m_AManager->UpdateAniPosition(0.0f, m_Object);
            m_bLockAnimation1 = true;
        }
        if ((keyBuffer['1'] & 0x80) && !(m_PrevKeyBuffer['1'] & 0x80)) {
            Skill1();
        }
        if ((keyBuffer['2'] & 0x80) && !(m_PrevKeyBuffer['2'] & 0x80)) {
            Skill2();
        }
        if ((keyBuffer['3'] & 0x80) && !(m_PrevKeyBuffer['3'] & 0x80)) {
            Skill3();
        }
    }
    memcpy(m_PrevKeyBuffer, keyBuffer, sizeof(m_PrevKeyBuffer));
}

void CPlayerPriest::UpdateObject(float fElapsedTime)
{
    bool test = false;
    m_AManager->UpdateCombo(fElapsedTime);
    if (!m_AManager->IsInCombo() && m_AManager->IsAnimationFinished()) {
        m_AManager->ChangeAnimation(ANI_IDLE, false);
        test = true;
        m_bLockAnimation1 = false;
        m_bLockAnimation = false;
        m_bDoingCombo = false;
    }
    if (m_AManager->IsComboInterrupted()) {
        test = true;
        m_AManager->ClearComboInterrupted();
        m_bLockAnimation1 = false;
        m_bLockAnimation = false;
        m_bDoingCombo = false;
    }

    if (test) {
        m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
    }
}

// =======================================================================================

CPlayer::CPlayer(CPlayableCharacter* playerObject, std::shared_ptr<CCamera>& camera)
	: m_pPlayerObject(playerObject), m_pCamera(camera)
{
	m_pPlayerObject->SetCamera(camera);
}

void CPlayer::MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	m_pPlayerObject->MouseProcess(hWnd, nMessage, wParam, lParam);
}

void CPlayer::ProcessInput(UCHAR* keyBuffer,float fElapsedTime)
{
	m_pPlayerObject->ProcessInput(keyBuffer, fElapsedTime);
}

void CPlayer::HeightCheck(CHeightMapImage* heightmap, float fElapsedTime)
{
	CSkinningObject* p = m_pPlayerObject->getObject();
	XMFLOAT4X4& playerWorld = p->getWorldMatrix();
	XMFLOAT4X4& playerPreWorld = p->getPreWorldMatrix();
	XMFLOAT4X4& objectWorld = p->getObjects()[0]->getWorldMatrix();
	float fy = objectWorld._42 - (30 * fElapsedTime);

	float terrainHeight = heightmap->GetHeightinWorldSpace(objectWorld._41 + 1024.0f, objectWorld._43 + 1024.0f);
	if (objectWorld._43 >= -500.0f) {
		if (terrainHeight < 10.0f) {
			terrainHeight = 10.0f;
		}
	}
	if (fy < terrainHeight)
		playerWorld._42 = terrainHeight;
	else
		playerWorld._42 -= (30 * fElapsedTime);
	p->SetPosition(XMFLOAT3(playerWorld._41, playerWorld._42, playerWorld._43));
	playerPreWorld._42 = playerWorld._42;
}

// =======================================================================================
