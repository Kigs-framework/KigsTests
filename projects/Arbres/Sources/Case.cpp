#include "Case.h"

v2i	Case::mTranslatePos[4] = { {0,-1},{1,0},{0,1},{-1,0} };
v3f Case::mColors[4] = { {0.2,0.02,0.02},{0.8,0.8,0.8},{0.0,1.0,0.0},{0.0,0.0,1.0} };

Case::Case()
{

}

bool	Case::addNeighbour(NeighbourPos pos, Case* n)
{
	if (!n)
		return false;

	// first check if neighbour is not already there
	for (auto& existingN : mNeighbors)
	{
		if (existingN.second == pos) 
		{
			return false;
		}
	}

	mNeighbors.push_back({ n,pos });
	return true;
}

// set color according to type
void	Case::resetColor()
{
	mColor = mColors[mType];
}