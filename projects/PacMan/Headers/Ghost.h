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

START_DECLARE_COREFSMSTATE(Ghost, Die)
UPGRADOR_WITHOUT_METHODS()
END_DECLARE_COREFSMSTATE()
