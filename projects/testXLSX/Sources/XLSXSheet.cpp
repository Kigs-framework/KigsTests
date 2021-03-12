#include "XLSXSheet.h"
#include "XLSXDocument.h"

void XLSXSheet::initFromXML(XMLBase* xml)
{
	mBaseXML = xml;
	mRows.clear();

	XMLNodeBase* worksheet = xml->getRoot();
	if (worksheet)
	{
		XMLNodeBase* dimension = worksheet->getChildElement("dimension");
		if (dimension)
		{
			auto ref = dimension->getAttribute("ref");
			std::string dimstr = ref->getString();	
			setRange(dimstr);
		}
		// now get values
		XMLNodeBase* sheetData = worksheet->getChildElement("sheetData");
		if (sheetData)
		{
			for (u32 i = 0; i < sheetData->getChildCount(); i++)
			{
				XMLNodeBase* e = sheetData->getChildElement(i);
				if (e->getName() == "row")
				{
					for (u32 ci = 0; ci < e->getChildCount(); ci++)
					{
						XMLNodeBase* ce = e->getChildElement(ci);
						if (ce && ce->getChildCount())
						{
							XMLNodeBase* val = ce->getChildElement(0);
							if (val->getChildCount())
							{
								val = val->getChildElement(0);
							}
							auto cellname = ce->getAttribute("r");
							if (cellname)
							{
								std::string strcellname = cellname->getString();

								auto cell = (*this)[strcellname];

								auto type = ce->getAttribute("t");

								if (type && (type->getString() == "s"))
								{
									*cell = mSharedStrings->getString(val->getInt()); //CoreItemSP::getCoreValue(mSharedStrings->getString(val->getInt()));
								}
								else
								{
									*cell = val->getInt(); //CoreItemSP::getCoreValue();
								}

							}
						}
					}
				}
				
			}
		}
	}
}

void XLSXSheet::updateXML(XMLBase* xml)
{
	XMLNodeBase* worksheet = xml->getRoot();
	if (worksheet)
	{
		XMLNodeBase* dimension = worksheet->getChildElement("dimension");
		if (dimension)
		{
			XMLAttribute* ref = static_cast<XMLAttribute*>(dimension->getAttribute("ref"));
			std::string dimstr = "A1:" + getCellName(mRows[0].size() - 1, mRows.size() - 1);
			ref->setString(dimstr);
		}
		XMLNodeBase* sheetData = worksheet->getChildElement("sheetData");
		if (sheetData)
		{
			/*for (u32 i = 0; i < sheetData->getChildCount(); i++)
			{
				XMLNodeBase* e = sheetData->getChildElement(i);
				if (e->getName() == "row")
				{
					for (u32 ci = 0; ci < e->getChildCount(); ci++)
					{
						XMLNodeBase* ce = e->getChildElement(ci);
						if (ce && ce->getChildCount())
						{
							XMLNodeBase* val = ce->getChildElement(0);
							if (val->getChildCount())
							{
								val = val->getChildElement(0);
							}
							auto cellname = ce->getAttribute("r");
							if (cellname)
							{
								std::string strcellname = cellname->getString();

								auto cell = (*this)[strcellname];

								auto type = ce->getAttribute("t");

								if (type && (type->getString() == "s"))
								{
									*cell = mSharedStrings->getString(val->getInt()); //CoreItemSP::getCoreValue(mSharedStrings->getString(val->getInt()));
								}
								else
								{
									*cell = val->getInt(); //CoreItemSP::getCoreValue();
								}

							}
						}
					}
				}

			}*/
		}
	}
}


void	XLSXSheet::setRange(const std::string& range)
{
	auto rangesize = XLSXSheet::getRange(range);
	mRows.resize(rangesize.second[1] + 1);
	int rindex = 0;
	for (auto& r : mRows)
	{
		r.resize(rangesize.second[0] + 1);
		r.mIndex = rindex;
		r.mSheet = this;

		int cindex = 0;
		for (auto& c : r)
		{
			c.mName.setIndex({ cindex,rindex });
			++cindex;
		}
		++rindex;
	}
}



// index starting at 0
std::string	XLSXSheet::getCellName(u32 col, u32 row)
{
	std::string name="";

	do
	{
		std::string newletter;
		newletter += ('A' + (col % 26));

		name = newletter + name;
		col /= 26;
	} 	while (col);

	row++;
	name+=std::to_string(row);

	return name;
}

// index starting from 0
s32	XLSXSheet::getColIndex(const std::string& colname)
{
	if (colname.length() == 0)
	{
		return -1;
	}
	s32 index=0;
	u32 pos = 0;
	while (pos < colname.size())
	{
		index = index * 26 + (colname[pos] - 'A');
		pos++;
	}
	return index;
}

// index starting from 0
s32	XLSXSheet::getRowIndex(const std::string& rowname)
{
	s32 index = -1;
	if (rowname.length())
	{
		index = std::stoi(rowname);
		index--;
	}
	return index;
}

std::pair<v2i, v2i>	XLSXSheet::getRange(const std::string& rangename)
{
	std::pair<v2i, v2i> result;
	v2i min(0,0);
	v2i max(0,0);
	size_t found = rangename.find(":"); // is this a range
	if (found != std::string::npos)
	{
		min=XLSXSheet::getCellPos(rangename.substr(0, found));
		max=XLSXSheet::getCellPos(rangename.substr(found+1));
	}
	else
	{
		max = min = XLSXSheet::getCellPos(rangename);
	}
	result.first = min;
	result.second = max;
	return result;
}

v2i	XLSXSheet::getCellPos(const std::string& cellname)
{
	v2i result;
	result[0] = getColIndex(getColName(cellname));
	result[1] = getRowIndex(getRowName(cellname));
	return result;
}

std::string XLSXSheet::getColName(const std::string& cellname)
{
	int pos = 0;
	std::string colName = "";
	while (pos < cellname.length())
	{
		if ((cellname[pos] < 'A') || (cellname[pos] > 'Z'))
		{
			break;
		}
		colName += cellname[pos];
		pos++;
	}
	return colName;
}

std::string	XLSXSheet::getRowName(const std::string& cellname)
{
	int pos = 0;
	std::string rowName = "";
	while (pos < cellname.length())
	{
		if ((cellname[pos] >= 'A') && (cellname[pos] <= 'Z'))
		{
			pos++;
			continue;
		}
		if ((cellname[pos] >= '0') && (cellname[pos] <= '9'))
		{
			rowName += cellname[pos];
		}
		else // malformed
		{
			rowName = "";
			break;
		}
		pos++;
	}
	return rowName;
}


