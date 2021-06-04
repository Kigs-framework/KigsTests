#include "Ghost.h"
#include "Core.h"
#include "CoreFSM.h"
#include "Board.h"
#include "Timer.h"

IMPLEMENT_CLASS_INFO(Ghost)

Ghost::Ghost(const kstl::string& name, CLASS_NAME_TREE_ARG) : CharacterBase(name, PASS_CLASS_NAME_TREE_ARG)
{

}

void	Ghost::InitModifiable()
{
	ParentClassType::InitModifiable();
	if (IsInit())
	{
		// graphic representation first

		std::string ghostName="Pacman.json:";
		ghostName += (std::string)mName;

		mGraphicRepresentation = KigsCore::GetInstanceOf("ghost", "UIImage");
		mGraphicRepresentation->setValue("TextureName", ghostName);
		mGraphicRepresentation->setValue("Anchor", v2f(0.5f, 0.5f));
		mGraphicRepresentation->setValue("Priority", 15);
		mBoard->getGraphicInterface()->addItem(mGraphicRepresentation);
		mGraphicRepresentation->Init();

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


// create and init Upgrador if needed and add dynamic attributes
void	CoreFSMStateClass(Ghost, Appear)::Init(CoreModifiable* toUpgrade)
{
	Ghost* gh = static_cast<Ghost*>(toUpgrade);
	Board* b = gh->getBoard();
	// set ghost pos
	if (b)
	{
		v2i pos=b->getAppearPosForGhost();
		gh->setCurrentPos(v2f( pos.x,pos.y ));
	}
}
// destroy UpgradorData and remove dynamic attributes 
void	CoreFSMStateClass(Ghost, Appear)::Destroy(CoreModifiable* toDowngrade, bool toDowngradeDeleted)
{

}


// ShowPopUp slot
DEFINE_UPGRADOR_METHOD(CoreFSMStateClass(Ghost, FreeMove), seePacMan)
{
	
	return false;
}

// update ( hide popup if it was open for too long ) 
DEFINE_UPGRADOR_UPDATE(CoreFSMStateClass(Ghost, FreeMove))
{
	v2f newpos = mCurrentPos;
	v2f rpos = getRoundPos();
	if (mDirection >= 0)
	{
		double dt = timer.GetDt(this);
		if (dt > 0.1)
		{
			dt = 0.1;
		}
		v2f dtmove(movesVector[mDirection].x, movesVector[mDirection].y);
		newpos += dtmove * dt*mSpeed;

		// too far ? 
		v2f dpos = newpos - mCurrentPos;
		v2f dDest = v2f(mDestPos.x, mDestPos.y) - mCurrentPos;

		if (Dot(dDest, dpos) < 0) 
		{
			// check if need to change direction
			std::vector<bool> availableCases = mBoard->getAvailableDirection(mDestPos);
			if ((availableCases[mDirection]) && (!mBoard->checkForGhostOnCase(mDestPos,this)) && (!mBoard->checkForGhostOnCase(rpos, this)))
			{
				mCurrentPos = mDestPos;
				mDestPos = rpos;
				mDestPos+=movesVector[mDirection];
			}
			else
			{
				mCurrentPos = mDestPos;
				newpos = mCurrentPos;
				rpos = getRoundPos();
				mDirection = -1;
			}
		}

		setCurrentPos(newpos);
	}

	if (mDirection == -1)
	{
		std::vector<bool> availableCases = mBoard->getAvailableDirection(rpos);
		std::vector<std::pair<int, v2i>> reallyAvailable;
		// check if other ghost is on available case
		for (int direction = 0; direction < 4; direction++)
		{
			if (availableCases[direction])
			{
				v2i dircase = rpos;
				dircase += movesVector[direction];

				if (mBoard->checkForGhostOnCase(dircase))
				{
					availableCases[direction] = false;
				}
				else
				{
					reallyAvailable.push_back({ direction,dircase });
				}
			}
		}

		if (reallyAvailable.size())
		{
			auto choose = reallyAvailable[rand() % reallyAvailable.size()];
			mDirection = choose.first;
			mDestPos = choose.second;
		}
	}
}

// create and init Upgrador if needed and add dynamic attributes
void	CoreFSMStateClass(Ghost, FreeMove)::Init(CoreModifiable* toUpgrade)
{
	
}
// destroy UpgradorData and remove dynamic attributes 
void	CoreFSMStateClass(Ghost, FreeMove)::Destroy(CoreModifiable* toDowngrade, bool toDowngradeDeleted)
{

}