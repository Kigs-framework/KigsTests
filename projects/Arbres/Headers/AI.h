#pragma once
#include "Case.h"
#include <map>

// base class for AI
class AI
{
public:

	// AI starts at a starting Case
	AI(Case* start);
	virtual ~AI();

	// pure virtual run AI
	virtual bool	run()=0;

protected:

	// current path
	std::vector<Case*>	mPath;
};

class FirstFound : public AI
{
public:
	FirstFound(Case* start) : AI(start) {}
	bool	run() override;
};

class BestFound : public AI
{
protected:
	std::vector<Case*>	mFound;
public:
	BestFound(Case* start) : AI(start) {}
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

	Dijkstra(Case* start);
	bool	run() override;

};