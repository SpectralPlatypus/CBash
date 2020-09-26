#pragma once
#include <map>

namespace Skyblivion
{
	class DIALToGLOBMap
	{
	public:
		DIALToGLOBMap();
		int getGLOBFormID(int dialFormID);

	private:
		std::map<int, int> map;
	};
}