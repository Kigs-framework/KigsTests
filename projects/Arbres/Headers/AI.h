#pragma once
#include "Case.h"
#include <map>

class AI
{
public:
	AI(Case* start,const v2i& startpos );
	virtual ~AI();

	virtual bool	run()=0;

protected:

	struct searchCase
	{
		Case*	mCase;
		v2i		mPos;
		int		mIndex;
	};

	std::vector<searchCase>	mPath;

};

class recursiveFirstFound : public AI
{
public:
	recursiveFirstFound(Case* start, const v2i& startpos) : AI(start, startpos) {}
	bool	run() override;
};

class recursiveBestFound : public AI
{
protected:
	std::vector<searchCase>	mFound;
public:
	recursiveBestFound(Case* start, const v2i& startpos) : AI(start, startpos) {}
	bool	run() override;
};


class Dijkstra : public AI
{
	
	struct DNode
	{
		Case*												mCase;
		std::vector<std::pair<std::vector<Case*>, DNode*>>	mLinks;
		int													mW = -1;
	};

	std::map<Case*,DNode>					mNodes;

	void setNeighbors(DNode& startNode, Case* current, std::vector<Case*> path,int deep);

public:

	Dijkstra(Case* start, const v2i& startpos);
	bool	run() override;

};