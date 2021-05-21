#pragma once
#include "Case.h"
#include <vector>
#include <string>
#include "CoreModifiable.h"

class Board
{
protected:

	std::vector<std::vector<Case>>	mCases;
	v2f								mBoardSize;

public:

	Board(const std::string& filename);
};
