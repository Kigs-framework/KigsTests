#pragma once
#include "Case.h"
#include <map>
#include <set>

// base class for AI
class AI
{
public:

	// AI starts at a starting Case
	AI()
	{
		mPath.clear();
	}
	virtual ~AI();

	virtual void init(Case* start,v2i startPos,Case* end,v2i endPos);

	// pure virtual run AI
	virtual bool	run()=0;

protected:

	// current path
	std::vector<Case*>	mPath;
};

class FirstFound : public AI
{
public:
	FirstFound() : AI() {}
	bool	run() override;
};

class BestFound : public AI
{
protected:
	std::vector<Case*>	mFound;
public:
	BestFound() : AI() {}
	bool	run() override;
};


class Dijkstra : public AI
{
private:
	struct DNode
	{
		Case*												mCase;
		std::vector<std::pair<std::vector<Case*>, DNode*>>	mLinks;
		int													mW = -1;
	};

	std::map<Case*,DNode>					mNodes;

	void setNeighbors(DNode& startNode, Case* current, std::vector<Case*> path);

	void forwardPath(Case* exitcase);

	void setBackLinks();
public:

	Dijkstra() : AI() {}
	bool	run() override;
	void init(Case* start, v2i startPos, Case* end, v2i endPos) override;

};

class AStar : public AI
{
private:

	struct WNode
	{
		Case*		mCase = nullptr;
		float		mWD = -1.0f;
		v2i			mPos;
		bool operator<(const WNode& other) const { if (mWD == other.mWD) return mCase < other.mCase; return mWD < other.mWD; }
	};

	std::map<Case*,WNode>		mClosedList;
	std::set<WNode>				mOpenList;
	std::map<Case*,const WNode*>		mOpenListMap;

	Case*						mCurrent=nullptr;

	void						forwardPath(Case* exitcase);

	Case*						mEnd=nullptr;
	v2i							mStartPos;
	v2i							mEndPos;

	const v2i					mDeltapos[4] = { {0,-1} , {1,0} , {0,1} , {-1,0}  };

	float						mTotalDist = 0.0f;
public:

	AStar() : AI()
	{
		
	}

	void init(Case* start, v2i startPos, Case* end, v2i endPos) override
	{
		AI::init(start, startPos, end, endPos);

		mEnd = end;

		mStartPos = startPos;
		mEndPos = endPos;

		v2f deltapos(mEndPos);
		deltapos -= mStartPos;
		mTotalDist = length(deltapos);

		WNode toAdd;
		toAdd.mCase = start;
		toAdd.mPos = startPos;
		toAdd.mWD = 0.0f;
		mClosedList[start] = toAdd;
		mCurrent = start;
	}


	bool	run() override;

};