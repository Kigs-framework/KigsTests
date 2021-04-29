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

	virtual void init(Case* start);

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
	void init(Case* start) override;

};

class AStar : public AI
{
private:

	struct WNode
	{
		Case*		mCase = nullptr;
		float		mWD = -1.0f;
		bool operator<(const WNode& other) const { if (mWD == other.mWD) return mCase < other.mCase; return mWD < other.mWD; }
	};

	std::map<Case*,WNode>		mClosedList;
	std::set<WNode>				mOpenList;

	const float					mDirectionW[4] = { 2.0f,0.1f,2.0f,4.0f };

	Case*						mCurrent=nullptr;

	void						forwardPath(Case* exitcase);

public:

	AStar() : AI()
	{
		
	}

	void init(Case* start) override
	{
		AI::init(start);
		WNode toAdd;
		toAdd.mCase = start;
		toAdd.mWD = 0.0f;
		mClosedList[start] = toAdd;
		mCurrent = start;
	}


	bool	run() override;

};