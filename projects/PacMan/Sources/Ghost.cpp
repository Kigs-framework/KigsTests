#include "Ghost.h"
#include "Core.h"

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
		CMSP fsm = KigsCore::GetInstanceOf("fsm", "CoreFSM");
		addItem(fsm);
	}
}
