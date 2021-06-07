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

	void Update(const Timer& timer, void* addParam) override;

protected:
	WRAP_METHODS(UpdateKeyboard)
	void UpdateKeyboard(kstl::vector<KeyEvent>& keys);

	// direction and time of last keyboard set
	std::pair<double, int>	mKeyDirection;

};

