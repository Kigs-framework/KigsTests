#pragma once
#include "Case.h"
#include <map>

class AI
{
public:
	AI(Case* start,const v2i& startpos );
	virtual ~AI();

	virtual bool	Update()=0;

protected:

	struct searchCase
	{
		Case*	mCase;
		v2i		mPos;
		int		mIndex;
	};

	std::vector<searchCase>	mPath;

};

class FirstFound : public AI
{
public:
	FirstFound(Case* start, const v2i& startpos) : AI(start,startpos){}
	bool	Update() override;
};

class BestFound : public AI
{
protected:
	std::vector<searchCase>	mFound;
public:
	BestFound(Case* start, const v2i& startpos) : AI(start, startpos) {}
	bool	Update() override;
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

	void setNeighbors(DNode& startNode, Case* current, std::vector<Case*> path);

public:

	Dijkstra(Case* start, const v2i& startpos);
	bool	Update() override;

};