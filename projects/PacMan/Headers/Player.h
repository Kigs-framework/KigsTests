#pragma once

#include "CoreModifiable.h"
#include "CharacterBase.h"
#include "KeyboardDevice.h"

class Player : public CharacterBase
{
public:
	DECLARE_CLASS_INFO(Player, CharacterBase, PacMan);
	DECLARE_CONSTRUCTOR(Player);

	void	InitModifiable() override;

protected:
	WRAP_METHODS(UpdateKeyboard)
	void UpdateKeyboard(kstl::vector<KeyEvent>& keys);
};

