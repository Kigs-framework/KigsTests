#include "CharacterBase.h"
#include "Board.h"

IMPLEMENT_CLASS_INFO(CharacterBase)

CharacterBase::CharacterBase(const kstl::string& name, CLASS_NAME_TREE_ARG) : CoreModifiable(name, PASS_CLASS_NAME_TREE_ARG)
{

}

void	CharacterBase::setCurrentPos(const v2f& pos)
{
	mCurrentPos = pos;
	if (mGraphicRepresentation)
	{
		v2f dock = mBoard->convertBoardPosToDock(mCurrentPos);
		mGraphicRepresentation->setValue("Dock", dock);
	}
}