#include "Board.h"
#include "JSonFileParser.h"

Board::Board(const std::string& filename)
{
	JSonFileParser L_JsonParser;
	CoreItemSP initP = L_JsonParser.Get_JsonDictionary(filename);

	mBoardSize = (v2f)initP["Size"];

	if ((mBoardSize.y > 0) && (mBoardSize.x > 0))
	{
		CoreItemSP cases = initP["Cases"];
		size_t caseIndex = 0;
		mCases.resize(mBoardSize.y);
		for (int i = 0; i < mBoardSize.y; i++)
		{
			mCases[i].resize(mBoardSize.x);
			for (int j = 0; j < mBoardSize.x; j++)
			{
				mCases[i][j].setInitType((int)cases[caseIndex]);
				caseIndex++;
			}
		}
	}
}