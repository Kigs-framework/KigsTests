#include "Ghost.h"
#include "Core.h"
#include "CoreFSM.h"

IMPLEMENT_CLASS_INFO(Ghost)

Ghost::Ghost(const kstl::string& name, CLASS_NAME_TREE_ARG) : CoreModifiable(name, PASS_CLASS_NAME_TREE_ARG)
{

}

void	Ghost::InitModifiable()
{
	ParentClassType::InitModifiable();
	if (IsInit())
	{
		// add FSM
		SP<CoreFSM> fsm = KigsCore::GetInstanceOf("fsm", "CoreFSM");
		addItem(fsm);
		// Appear state
		fsm->addState("Appear", new CoreFSMStateClass(Ghost, Appear)());
		SP<CoreFSMTransition> wait = KigsCore::GetInstanceOf("wait", "CoreFSMDelayTransition");
		wait->setState("FreeMove");
		wait->Init();
		fsm->getState("Appear")->AddTransition(wait);

		// FreeMove state
		fsm->addState("FreeMove", new CoreFSMStateClass(Ghost, FreeMove)());
		SP<CoreFSMTransition> pacmanHunting = KigsCore::GetInstanceOf("pacmanHunting", "CoreFSMOnEventTransition");
		pacmanHunting->setValue("EventName", "PacManHunting");
		pacmanHunting->setState("Hunted");
		pacmanHunting->Init();
		fsm->getState("FreeMove")->AddTransition(pacmanHunting);
		SP<CoreFSMTransition> hunt = KigsCore::GetInstanceOf("hunt", "CoreFSMOnMethodTransition");
		hunt->setValue("MethodName", "seePacMan");
		hunt->setState("Hunting");
		hunt->Init();
		fsm->getState("FreeMove")->AddTransition(hunt);

		// Hunting state
		fsm->addState("Hunting", new CoreFSMStateClass(Ghost, Hunting)());
		fsm->getState("Hunting")->AddTransition(pacmanHunting);
		SP<CoreFSMTransition> pacmanNotVisible = KigsCore::GetInstanceOf("pacmanNotVisible", "CoreFSMOnValueTransition");
		pacmanNotVisible->setValue("ValueName", "PacManNotVisible");
		pacmanNotVisible->setState("FreeMove");
		pacmanNotVisible->Init();
		fsm->getState("Hunting")->AddTransition(pacmanNotVisible);

		// Hunted state
		fsm->addState("Hunted", new CoreFSMStateClass(Ghost, Hunted)());
		SP<CoreFSMTransition> die = KigsCore::GetInstanceOf("die", "CoreFSMOnMethodTransition");
		die->setValue("MethodName", "touchPacMan");
		die->setState("Die");
		fsm->getState("Hunted")->AddTransition(die);
		SP<CoreFSMTransition> pacmanHuntingEnd = KigsCore::GetInstanceOf("pacmanHuntingEnd", "CoreFSMOnEventTransition");
		pacmanHuntingEnd->setValue("EventName", "PacManHuntingEnd");
		pacmanHuntingEnd->setState("FreeMove");
		pacmanHuntingEnd->Init();
		fsm->getState("Hunted")->AddTransition(pacmanHuntingEnd);


		// Die state
		fsm->addState("Die", new CoreFSMStateClass(Ghost, Die)());

		fsm->setStartState("Appear");
		fsm->Init();
	}
}


// ShowPopUp slot
DEFINE_UPGRADOR_METHOD(CoreFSMStateClass(Ghost, FreeMove), seePacMan)
{
	
	return false;
}

// update ( hide popup if it was open for too long ) 
DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(Ghost, FreeMove))
{
	//kdouble t = timer.GetTime();
	
}

// create and init Upgrador if needed and add dynamic attributes
void	CoreFSMStateClass(Ghost, FreeMove)::Init(CoreModifiable* toUpgrade)
{

}
// destroy UpgradorData and remove dynamic attributes 
void	CoreFSMStateClass(Ghost, FreeMove)::Destroy(CoreModifiable* toDowngrade, bool toDowngradeDeleted)
{

}