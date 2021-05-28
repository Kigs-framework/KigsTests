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



		fsm->addState(CoreFSMStateClassName(Ghost, Appear), new CoreFSMStateClass(Ghost, Appear)());
	}
}
