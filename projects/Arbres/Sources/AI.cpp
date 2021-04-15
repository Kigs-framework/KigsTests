#include "AI.h"
#include "Arbres.h""

AI::AI(Case* start)
{
	// just add start Case to the path
	mPath.clear();
	mPath.push_back(start);
}

AI::~AI()
{
	mPath.clear();
}

bool	FirstFound::run()
{
	// mecanism to wait for next step from application
	while (Arbres::isLocked())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}
	Arbres::lock();

	// check if current Case is Exit
	Case* current = mPath.back();
	if (current->getType() == Case::CaseType::Exit) 
	{
		return true; // OK we have a good path
	}

	// visit all neighbors
	for (auto& c : current->getNeighbors())
	{		
		if (c.first->isVisit()) // case already visited
		{
			continue;
		}

		// set this neighbor as visited and add it to path
		c.first->setVisit(true);
		mPath.push_back(c.first);
		// then recurse
		if (run()) // return only if exit was found
		{
			return true; 
		}
	}

	// all neigbors were recursively visited but none was the exit so pop this case (go back)
	current->setVisit(false);
	mPath.pop_back();
	return false;
}


bool	BestFound::run()
{
	// mecanism to wait for next step from application
	while (Arbres::isLocked())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}
	Arbres::lock();

	// test if current case is Exit
	Case* current = mPath.back();
	if (current->getType() == Case::CaseType::Exit) // exit found
	{
		// check if this is the best (or first) found path
		if ((mPath.size() <= mFound.size()) || (!mFound.size()))
		{
			mFound = mPath;
		}
	}

	// visit all neighbors
	for (auto& c : current->getNeighbors())
	{
		if (c.first->isVisit()) // case already visited
		{
			continue;
		}

		// set this case as visited and push it on path
		c.first->setVisit(true);
		mPath.push_back(c.first);

		// recurse
		if (run())
		{
			return true; // if all pathes were tested and best was found, return here
		}
	}

	// all neighbors were recursively visited, go back
	current->setVisit(false);
	mPath.pop_back();

	// check if all pathes were visited
	if (mPath.size() == 0)
	{
		// mark best found as visited for main application 
		for (auto& c : mFound)
		{
			c->setVisit(true);
		}
		return true;
	}
	return false;
}

// mark crossroads and construct links between crossroads
void Dijkstra::setNeighbors(DNode& startNode,Case* current, std::vector<Case*> path)
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
				return setNeighbors(startNode, n.first, path);
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
		std::vector<Case*> newpath;
		newpath.push_back(current);
		for (auto& n : current->getNeighbors())
		{
			setNeighbors(endNode, n.first, newpath);
		}
	}
}

Dijkstra::Dijkstra(Case* start) : AI(start)
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
		setNeighbors(mNodes[start], n.first, newpath);
	}

	for (auto& n : mNodes)
	{
		n.first->setColor({ 0.2,1.0,0.5 });
	}
}

void Dijkstra::forwardPath(Case* currentcase)
{
	// clear path
	for (auto c : mPath)
	{
		c->setVisit(false);
	}
	// then follow the path back from the exit to the start going to the smaller link

	while (currentcase->getType() != Case::CaseType::Start)
	{
		DNode& currentN = mNodes[currentcase];
		int bestw = 0;
		std::pair<std::vector<Case*>, DNode*> bestnode;

		for (const auto& l : currentN.mLinks)
		{
			if ((l.second->mW < bestw) || (bestw ==0))
			{
				bestw = l.second->mW;
				bestnode = l;
			}
		}

		for (auto c : bestnode.first)
		{
			c->setVisit(true);
		}

		currentcase = bestnode.second->mCase;

	}
}

bool	Dijkstra::run()
{
	// mecanism to wait for next step from application
	while (Arbres::isLocked())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}
	Arbres::lock();

	Case* current = mPath.back();

	if (current->getType() == Case::CaseType::Exit) // found
	{
		forwardPath(current);
		return true;
	}
	
	DNode& currentN = mNodes[current];
	current->setVisit(true);

	// update linked nodes
	for (auto l : currentN.mLinks)
	{
		int neww = currentN.mW + l.first.size();
		if ((l.second->mW < 0) || ((!l.second->mCase->isVisit())&&(l.second->mW> neww)))
		{
			l.second->mW = neww;
			l.second->mCase->setText(std::to_string(l.second->mW));
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
		mPath.push_back(goodOne);
		if (run())
		{
			return true;
		}
	}
	
	return false;
}