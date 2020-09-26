#pragma once
#include <map>

namespace Skyblivion
{
	class CELLToLCTNMap
	{
	public:
		CELLToLCTNMap();
		int getLCTNFormID(int cellFormID);

	private:
		std::map<int, int> map;
	};
}