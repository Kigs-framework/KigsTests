#include "AI.h"
#include "Arbres.h""

AI::AI(Case* start, const v2i& startpos)
{
	mPath.clear();
	searchCase	toAdd;
	toAdd.mCase = start;
	toAdd.mIndex = -1;
	toAdd.mPos = startpos;

	mPath.push_back(toAdd);
}

AI::~AI()
{
	mPath.clear();
}

bool	recursiveFirstFound::run()
{
	while (Arbres::isLocked())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}
	Arbres::lock();
	Case* current = mPath.back().mCase;
	if (current->getType() == Case::CaseType::Exit) // already found
	{
		return true;
	}

	for (auto& c : current->getNeighbors())
	{		
		if (c.first->isVisit()) // case already visited
		{
			continue;
		}
		searchCase	toAdd;
		toAdd.mCase = c.first;
		toAdd.mCase->setVisit(true);
		mPath.push_back(toAdd);
		if (run())
		{
			return true;
		}
	}

	current->setVisit(false);
	mPath.pop_back();

	return false;
}


bool	recursiveBestFound::run()
{
	while (Arbres::isLocked())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}
	Arbres::lock();

	if ((mPath.size() == 0) && (mFound.size()))
	{
		return true;
	}

	Case* current = mPath.back().mCase;
	if (current->getType() == Case::CaseType::Exit) // already found
	{
		if ((mPath.size() <= mFound.size()) || (!mFound.size()))
		{
			mFound = mPath;
		}
	}

	for (auto& c : current->getNeighbors())
	{
		if (c.first->isVisit()) // case already visited
		{
			continue;
		}
		searchCase	toAdd;
		toAdd.mCase = c.first;
		toAdd.mCase->setVisit(true);
		mPath.push_back(toAdd);
		if (run())
		{
			return true;
		}
	}

	current->setVisit(false);
	mPath.pop_back();
	if (mPath.size() == 0)
	{
		for (auto& c : mFound)
		{
			c.mCase->setVisit(true);
		}
		return true;
	}
	return false;
}


void Dijkstra::setNeighbors(DNode& startNode,Case* current, std::vector<Case*> path, int deep)
{

	if (current->getNeighbors().size() == 2) // go to next one
	{
		if (current->isVisit()) // already visited 
		{
			return;
		}
		current->setVisit(true);
		for (auto& n : current->getNeighbors())
		{
			if (n.first != path.back())
			{
				path.push_back(current);
				return setNeighbors(startNode, n.first, path,deep);
			}
		}
		// both nodes are already done ?
		return;
	}

	if ((current->getNeighbors().size() == 1) && (current->getType() == Case::CaseType::Slab)) 
	{
		current->setVisit(true);
		return;
	}

	// check if node already there, else add it
	auto f=mNodes.find(current);
	if (f == mNodes.end())
	{
		DNode toAdd;
		toAdd.mCase = current;
		toAdd.mLinks.clear();
		mNodes[current]=toAdd;
	}

	path.push_back(current);
	DNode& endNode = mNodes[current];
	bool endalreadyvisited = endNode.mCase->isVisit();

	startNode.mLinks.push_back({ path,&endNode });
	startNode.mCase->setVisit(true);
	endNode.mLinks.push_back({ path,&startNode });
	endNode.mCase->setVisit(true);

	if (!endalreadyvisited)
	{
		if (deep == 5)
		{
			printf("");
		}
		endNode.mCase->setText(std::to_string(deep));
		std::vector<Case*> newpath;
		newpath.push_back(current);
		for (auto& n : current->getNeighbors())
		{
			setNeighbors(endNode, n.first, newpath,deep+1);
		}
	}
}

Dijkstra::Dijkstra(Case* start, const v2i& startpos) : AI(start, startpos)
{
	// init by filling the whole tree
	start->setVisit(true);

	DNode toAdd;
	toAdd.mW = 0;
	toAdd.mCase = start;
	toAdd.mLinks.clear();
	mNodes[start]=toAdd;
	std::vector<Case*> newpath;
	newpath.push_back(start);
	for (auto& n : start->getNeighbors())
	{
		setNeighbors(mNodes[start], n.first, newpath,1);
	}

	for (auto& n : mNodes)
	{
		n.first->setColor({ 0.2,1.0,0.5 });
	}
}

bool	Dijkstra::run()
{
	/*
	searchCase& current = mPath.back();

	if (current.mCase->getType() == Case::CaseType::Exit) // found
	{
		return true;
	}
	
	DNode& currentN = mNodes[current.mCase];

	// weight n
	for (auto l : currentN.mLinks)
	{
		if (l.second->mW < 0)
		{
			l.second->mW = currentN.mW + l.first.size();
			l.second->mCase->setText(std::to_string(l.second->mW));
			return false;
		}
	}

	// go to the smaller unvisited

	Case* goodOne = nullptr;
	int bestW=0;

	for (auto& n : mNodes)
	{
		if ((n.second.mW > 0) && (!n.first->isVisit()))
		{
			if (!goodOne || (bestW > n.second.mW))
			{
				goodOne = n.first;
				bestW = n.second.mW;
			}
		}
	}

	if (goodOne)
	{

		searchCase	toAdd;
		toAdd.mCase = goodOne;
		toAdd.mIndex = -1;
		toAdd.mCase->setVisit(true);

		mPath.push_back(toAdd);
	}
	*/
	return false;
}