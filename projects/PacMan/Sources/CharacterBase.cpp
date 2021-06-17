#include "CharacterBase.h"
#include "Board.h"
#include "Timer.h"

IMPLEMENT_CLASS_INFO(CharacterBase)

CharacterBase::CharacterBase(const kstl::string& name, CLASS_NAME_TREE_ARG) : CoreModifiable(name, PASS_CLASS_NAME_TREE_ARG)
{

}

CharacterBase::~CharacterBase()
{
	if (mBoard && mGraphicRepresentation)
	{
		mBoard->getGraphicInterface()->removeItem(mGraphicRepresentation);
		mBoard = nullptr;
		mGraphicRepresentation = nullptr;
	}
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

void	CharacterBase::initAtPos(const v2i& pos)
{
	setCurrentPos(pos);
	mDirection = -1;
	mDestPos = pos;
}

// return true if is at dest
bool	CharacterBase::moveToDest(const Timer& timer,v2f& newpos)
{
	double dt = timer.GetDt(this);
	if (dt > 0.1)
	{
		dt = 0.1;
	}
	v2f dtmove(movesVector[mDirection].x, movesVector[mDirection].y);
	newpos += dtmove * dt * mSpeed*mSpeedCoef;

	// dest reached ?
	v2f dpos = (newpos + dtmove * dt * mSpeed * mSpeedCoef) - mCurrentPos;
	v2f dDest = v2f(mDestPos.x, mDestPos.y) - mCurrentPos;

	if (Dot(dDest, dpos) < 0.0f)
	{
		return true;
	}
	return false;
}