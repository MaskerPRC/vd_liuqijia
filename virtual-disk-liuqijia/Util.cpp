#include "Util.h"
#include <vector>
#include <string>

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

std::vector<std::string> SplitCmdLine(std::string _cmdLine)
{
	std::vector<std::string> cmdLines;
	bool isInDoubleQuotes = false;
	bool IsMatching = false;

	for (auto it = _cmdLine.begin(); it != _cmdLine.end(); NULL)
	{
		for (NULL; it != _cmdLine.end() && *it == L' '; ++it);
		if (it == _cmdLine.end()) break;

		auto begin = it;

		if (*begin == L'\"')
		{
			for (it = ++begin; it != _cmdLine.end() && *it != L'\"'; ++it);
			if (begin == it)
				continue;
			else
			{
				cmdLines.push_back(std::string(begin, it));
				if (it == _cmdLine.end()) break;
				if (*it == L'\"') ++it;
			}
		}
		else
		{
			for (NULL; it != _cmdLine.end() && *it != L' '; ++it);
			if (begin == it)
				continue;
			else
				cmdLines.push_back(std::string(begin, it));
		}
	}

	return std::move(cmdLines);
}
