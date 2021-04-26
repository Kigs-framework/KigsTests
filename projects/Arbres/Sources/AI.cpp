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
	if (current->getNeighbors().size() == 2) // corridor => go to next case
	{
		if (current->isVisit()) // already visited 
		{
			return;
		}
		current->setVisit(true);
		for (auto& n : current->getNeighbors())
		{
			if (n.first != path.back()) // this is not the one I come from ?
			{
				path.push_back(current);
				return setNeighbors(startNode, n.first, path); // recurse
			}
		}
		// both nodes are already done ?
		return;
	}

	if ((current->getNeighbors().size() == 1) && (current->getType() == Case::CaseType::Slab)) // cul de sac
	{
		current->setVisit(true);
		return;
	}

	// if code reach this, then the current case is a crossroad
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

	// so we have found an "endnode" and we have a link between startnode and endnode
	DNode& endNode = mNodes[current];
	
	startNode.mLinks.push_back({ path,&endNode });
	
	if (!endNode.mCase->isVisit())
	{
		endNode.mCase->setVisit(true);
		std::vector<Case*> newpath;
		newpath.push_back(current);

		// recurse
		for (auto& n : current->getNeighbors())
		{
			setNeighbors(endNode, n.first, newpath);
		}
	}
}

// 
void Dijkstra::setBackLinks()
{
	for (auto& n : mNodes)
	{
		for (auto& l : n.second.mLinks)
		{
			auto& endn = mNodes[l.second->mCase];
			endn.mLinks.push_back({ l.first,&n.second });
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
	start->setVisit(true);
	for (auto& n : start->getNeighbors())
	{
		setNeighbors(mNodes[start], n.first, newpath);
	}
	setBackLinks();
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
		int bestw = -1;
		std::pair<std::vector<Case*>, DNode*> bestnode;

		for (const auto& l : currentN.mLinks)
		{
			if ((l.second->mW < bestw) || (bestw == -1))
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

bool	AStar::run()
{
	// mecanism to wait for next step from application
	while (Arbres::isLocked())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}
	Arbres::lock();

	// check if current Case is Exit
	Case* current = mCurrent;
	if (current)
	{
		current->setVisit(true);
		if (current->getType() == Case::CaseType::Exit)
		{
			forwardPath(current);
			return true; // OK we have a good path
		}

		// weight all neighbors

		WNode& currentNode = mClosedList[current];

		for (auto& c : current->getNeighbors())
		{
			if (c.first->isVisit()) // case already in closed set
			{
				continue;
			}

			// set this neighbor as visited and add it to the open list
			c.first->setVisit(true);

			WNode toAdd;
			toAdd.mCase = c.first;
			toAdd.mWD = currentNode.mWD + mDirectionW[c.second];

			mOpenList.insert(toAdd);
		}
	}
	WNode bestNodeInOpenList = *(mOpenList.begin());
	mOpenList.erase(mOpenList.begin());
	mClosedList[bestNodeInOpenList.mCase] = bestNodeInOpenList;
	mCurrent = bestNodeInOpenList.mCase;
	if (run())
	{
		return true; // if all pathes were tested and best was found, return here
	}
	mCurrent = nullptr;

	return false;
}

void AStar::forwardPath(Case* currentcase)
{
	// clear all visited
	for (auto c : mClosedList)
	{
		c.first->setVisit(false);
	}
	for (auto c : mOpenList)
	{
		c.mCase->setVisit(false);
	}

	// then follow the path back from the exit to the start going to the smaller link

	while (currentcase->getType() != Case::CaseType::Start)
	{

		float bestw = -1.0f;
		Case* bestFound = nullptr;
		for (const auto& n : currentcase->getNeighbors())
		{
			if (mClosedList.find(n.first) != mClosedList.end())
			{
				WNode& currentN = mClosedList[n.first];

				if ((currentN.mWD < bestw) || (bestw < 0.0f))
				{
					bestw = currentN.mWD;
					bestFound = currentN.mCase;
				}
			}			
		}

		bestFound->setVisit(true);
		currentcase = bestFound;

	}
}