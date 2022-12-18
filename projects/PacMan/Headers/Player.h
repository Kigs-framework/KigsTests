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

	void	startHunting();

protected:
	WRAP_METHODS(UpdateKeyboard)
	void UpdateKeyboard(std::vector<KeyEvent>& keys);

	// direction and time of last keyboard set
	std::pair<double, int>	mKeyDirection;

	void	manageDeathState();
	double  mDeathTime = -1.0;
};

