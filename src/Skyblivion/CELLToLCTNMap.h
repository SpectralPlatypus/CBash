#pragma once
#include <map>
#include <string>

namespace Skyblivion
{
	class CELLToLCTNMap
	{
	public:
		CELLToLCTNMap();
		std::pair<int, std::string> getLCTN(int cellFormID);

	private:
		std::map<int, std::pair<int, std::string>> map;
	};
}