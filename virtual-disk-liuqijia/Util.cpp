#include "Util.h"

bool IsMatch(Comment(without * and ? ) const std::string & _str1, Comment(with * and ? ) const std::string & _str2)
{
	auto it1 = _str1.begin(), it2 = _str2.begin();
	for (NULL; it1 != _str1.end() && it2 != _str2.end(); NULL)
	{
		if (*it1 == *it2 || *it2 == '?')
		{
			++it1, ++it2;
			continue;
		}

		if (*it2 == '*')
		{
			if (++it2 == _str2.end()) return true;

			std::string restString2(it2, _str2.end());
			for (auto tempit1 = it1; tempit1 != _str1.end(); ++tempit1)
			{
				if (IsMatch(std::string(tempit1, _str1.end()), restString2)) return true;
			}
			return false;
		}
		return false;
	}

	bool flag = it2 != _str2.end();

	while (it2 != _str2.end() && *it2 == '*') ++it2;

	return (flag || it1 == _str1.end()) && it2 == _str2.end();
}
