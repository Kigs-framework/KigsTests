#include "AI.h"
#include "Arbres.h"

using namespace Kigs;

void AI::init(Case* start, v2i startPos, Case* end, v2i endPos)
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
	Arbres::waitMainThread();

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
	Arbres::waitMainThread();

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

void Dijkstra::init(Case* start, v2i startPos, Case* end, v2i endPos)
{
	AI::init(start,  startPos,  end,  endPos);
	// init by filling the whole tree
	start->setVisit(true);

	DNode toAdd;
	toAdd.mW = 0;
	toAdd.mCase = start;
	toAdd.mLinks.clear();
	mNodes[start] = toAdd;
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
	Arbres::waitMainThread();

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
	Arbres::waitMainThread();

	// check if current Case is Exit
	Case* current = mCurrent;

	std::set<WNode>::const_iterator itBest= mOpenList.end();

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
		// prepare compute move cost
		v2f deltapos(mEndPos);
		deltapos -= currentNode.mPos;
		float dist = Norm(deltapos);
		deltapos *= 1.0f / dist;

		// compute cost for all 4 directions

		std::vector<float> costs = {-1.0f,-1.0f,-1.0f,-1.0f};
		
		for (size_t costIndex = 0; costIndex < 4; costIndex++)
		{
			v2f fdeltamove(mDeltapos[costIndex]);

			float dotprod = Dot(fdeltamove, deltapos);

			costs[costIndex] = (2.0 - dotprod);
			costs[costIndex] *= costs[costIndex];
			costs[costIndex] *= (dist / mTotalDist);
			costs[costIndex] *= costs[costIndex];
		}
		std::vector<float> bestcosts = costs;
		std::sort(bestcosts.begin(), bestcosts.end());

		int currentBestCost = 0;
		for (auto& c : current->getNeighbors())
		{
			if (mClosedList.find(c.first) != mClosedList.end()) // case already in closed set
			{
				if (costs[c.second] == bestcosts[currentBestCost])
					currentBestCost++;
				continue;
			}

			// set this neighbor as visited and add it to the open list
			if (!c.first->isVisit()) // already in open list
			{
				c.first->setVisit(true);

				WNode toAdd;
				toAdd.mCase = c.first;
				toAdd.mPos = currentNode.mPos + mDeltapos[c.second];

				toAdd.mWD = currentNode.mWD + costs[c.second];

				auto inserted = mOpenList.insert(toAdd);
				mOpenListMap[c.first] = &(*(inserted.first));

				if (costs[c.second] == bestcosts[currentBestCost])
					itBest = inserted.first;
			}
			else // already visited, update if needed
			{
				float newW = currentNode.mWD + costs[c.second];

				const WNode* currentWNode = mOpenListMap[c.first];
				
				if (currentWNode->mWD > newW) 
				{
					WNode toAdd(*currentWNode);
					toAdd.mWD = newW;

					mOpenList.erase(mOpenList.find(*currentWNode));

					auto inserted = mOpenList.insert(toAdd);
					currentWNode= &(*(inserted.first));
					mOpenListMap[c.first] = currentWNode;
				}

				if (costs[c.second] == bestcosts[currentBestCost])
					itBest = mOpenList.find(*currentWNode);
			}
		
		}

	}

	if (itBest == mOpenList.end()) // if best way is not possible, then choose best node in open list
	{
		itBest = mOpenList.begin();
	}

	WNode bestNodeInOpenList = *(itBest);
	mOpenList.erase(itBest);
	mOpenListMap.erase(mOpenListMap.find(bestNodeInOpenList.mCase));
	mClosedList[bestNodeInOpenList.mCase] = bestNodeInOpenList;

	bestNodeInOpenList.mCase->setText("c");

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