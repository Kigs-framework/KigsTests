#include "AI.h"

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

bool	FirstFound::Update()
{
	searchCase& current = mPath.back();

	if (current.mCase->getType() == Case::CaseType::Exit) // already found
	{
		return true;
	}

	current.mIndex++;
	while (current.mIndex < current.mCase->getNeighbors().size())
	{
		searchCase	toAdd;

		v2i pos = current.mPos + Case::mTranslatePos[(int)current.mCase->getNeighbors()[current.mIndex].second];

		toAdd.mCase = current.mCase->getNeighbors()[current.mIndex].first;
		if (toAdd.mCase->isVisit()) // case already visited
		{
			current.mIndex++;
			continue;
		}
		toAdd.mIndex = -1;
		toAdd.mPos = pos;
		toAdd.mCase->setVisit(true);

		mPath.push_back(toAdd);
		return false;
	}
	
	current.mCase->setVisit(false);
	mPath.pop_back();
	
	return false;
}

bool	BestFound::Update()
{
	if ((mPath.size() == 0) && (mFound.size()))
	{
		return true;
	}
	searchCase& current = mPath.back();

	if (current.mCase->getType() == Case::CaseType::Exit) // found
	{
		if ((mPath.size() <= mFound.size()) || (!mFound.size()))
		{
			mFound = mPath;
		}
	}

	current.mIndex++;
	while(current.mIndex < current.mCase->getNeighbors().size())
	{
		
		searchCase	toAdd;

		v2i pos = current.mPos + Case::mTranslatePos[(int)current.mCase->getNeighbors()[current.mIndex].second];

		toAdd.mCase = current.mCase->getNeighbors()[current.mIndex].first;
		if (toAdd.mCase->isVisit()) // case already visited
		{
			current.mIndex++;
			continue;
		}
		toAdd.mIndex = -1;
		toAdd.mPos = pos;
		toAdd.mCase->setVisit(true);

		mPath.push_back(toAdd);
		return false;
	}

	
	current.mCase->setVisit(false);
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


void Dijkstra::setNeighbors(DNode& startNode,Case* current, std::vector<Case*> path)
{

	if (current->getNeighbors().size() == 2) // go to next one
	{
		current->setVisit(true);
		for (auto& n : current->getNeighbors())
		{
			if (!n.first->isVisit())
			{
				path.push_back(current);
				return setNeighbors(startNode, n.first, path);
			}
		}
		// both nodes are already done ?
		return;
	}

	if ((current->getNeighbors().size() == 1) && (current->getType() != Case::CaseType::Exit)) // cul de sac
	{
		std::vector<Case*> empty;
		startNode.mLinks.push_back({ empty ,nullptr });
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

	startNode.mLinks.push_back({ path,&mNodes[current] });
	mNodes[current].mLinks.push_back({ path,&startNode });

	if (startNode.mLinks.size() == current->getNeighbors().size())
	{
		current->setVisit(true);
	}

	std::vector<Case*> newpath;
	newpath.push_back(current);
	for (auto& n : current->getNeighbors())
	{
		if (!n.first->isVisit())
		{
			setNeighbors(mNodes[current], n.first, newpath);
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
		setNeighbors(mNodes[start], n.first, newpath);
	}

	for (auto& n : mNodes)
	{
		n.first->setColor({ 0.2,1.0,0.5 });
	}
}

bool	Dijkstra::Update()
{

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

	return false;
}