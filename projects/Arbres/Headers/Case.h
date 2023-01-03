#pragma once
#include <vector>
#include <string>
#include "TecLibs/Tec3D.h"

namespace Kigs
{
	class Arbres;
}

class Case
{

public:

	static v2i	mTranslatePos[4];
	static v3f	mColors[4];

	enum NeighbourPos
	{
		UP = 0,
		RIGHT = 1,
		DOWN = 2,
		LEFT = 3
	};
	
	enum CaseType
	{
		Slab = 0,
		Wall = 1,
		Start = 2,
		Exit = 3
	};

	Case();
	~Case() {};

	bool	addNeighbour(NeighbourPos pos, Case* n);

	void	setType(CaseType t)
	{
		mType = t;
		resetColor();
	}
	CaseType	getType()
	{
		return mType;
	}

	bool isVisit()
	{
		return mVisit;
	}

	void setVisit(bool visit)
	{
		mVisit=visit;
	}

	friend class Kigs::Arbres;

	const std::vector<std::pair<Case*, NeighbourPos>>& getNeighbors()
	{
		return mNeighbors;
	}

	// set color according to type
	void	resetColor();

	void	setColor(const v3f& c)
	{
		mColor = c;
	}

	const v3f&	getColor()
	{
		return mColor;
	}

	void	setText(const std::string& t)
	{
		mText = t;
	}

	const std::string& getText()
	{
		return mText;
	}

protected:
	CaseType				mType = Slab;
	bool					mVisit = false;
	std::vector<std::pair<Case*, NeighbourPos>>		mNeighbors;
	v3f						mColor;
	std::string				mText="";
};