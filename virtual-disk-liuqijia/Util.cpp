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

		bool isInDoubleQuotes = false;

		for (NULL; it != _cmdLine.end(); ++it)
		{
			if (*it == '\"')
				isInDoubleQuotes = !isInDoubleQuotes;
			if (!isInDoubleQuotes && *it == ' ')
				break;

		}
		if (begin == it)
			continue;
		else
		{
			std::string str(begin, it);
			for (auto it = str.begin(); it != str.end(); NULL)
			{
				if (*it == '\"')
					it = str.erase(it);
				else
					++it;
			}
			cmdLines.push_back(std::move(str));
			if (it == _cmdLine.end()) break;
			else ++it;
		}
	}
	return std::move(cmdLines);
}


bool CheckFileName(const std::string & _str)
{
	for (auto _ele : _str)
	{
		switch (_ele)
		{
		case '\\':
		case '/':
		case ':':
		case '*':
		case '?':
		case '"':
		case '<':
		case '>':
		case '|':
			return false;
		}
	}
	return true;
}