#pragma once

#include "CoreModifiable.h"
#include "CoreFSMState.h"
#include "CharacterBase.h"

class Board;

class Ghost : public CharacterBase
{
public:
	DECLARE_CLASS_INFO(Ghost, CharacterBase, PacMan);
	DECLARE_CONSTRUCTOR(Ghost);

	void	InitModifiable() override;

	template<typename T>
	friend class Upgrador;

protected:

	maString mName = BASE_ATTRIBUTE(Name, "");
};


START_DECLARE_COREFSMSTATE(Ghost, Appear)
// create and init Upgrador if needed and add dynamic attributes
virtual void	Init(CoreModifiable* toUpgrade) override;
// destroy UpgradorData and remove dynamic attributes 
virtual void	Destroy(CoreModifiable* toDowngrade, bool toDowngradeDeleted) override;
UPGRADOR_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(Ghost, FreeMove)
// create and init Upgrador if needed and add dynamic attributes
virtual void	Init(CoreModifiable* toUpgrade) override;
// destroy UpgradorData and remove dynamic attributes 
virtual void	Destroy(CoreModifiable* toDowngrade, bool toDowngradeDeleted) override;
UPGRADOR_METHODS(seePacMan)
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(Ghost, Hunting)
virtual void	Init(CoreModifiable* toUpgrade) override;
v2i	mPacmanSeenPos;
UPGRADOR_METHODS(seePacMan)
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(Ghost, Hunted)
UPGRADOR_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(Ghost, Die)
UPGRADOR_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()
