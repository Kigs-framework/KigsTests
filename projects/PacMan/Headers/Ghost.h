#pragma once

#include "CoreModifiable.h"
#include "CoreFSMState.h"


class Ghost : public CoreModifiable
{
public:
	DECLARE_CLASS_INFO(Ghost, CoreModifiable, PacMan);
	DECLARE_CONSTRUCTOR(Ghost);

	void	InitModifiable() override;
	
protected:

	
};


START_DECLARE_COREFSMSTATE(Ghost, Appear)
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
UPGRADOR_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(Ghost, Hunted)
UPGRADOR_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()

START_DECLARE_COREFSMSTATE(Ghost, Die)
UPGRADOR_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()
