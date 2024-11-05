#include "../includes/utils.hpp"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <cstdarg>

// ========== ========== String Utils ========== ==========
// trim from end of string (right)
std::string& rtrim(std::string& s, const char* t)
{
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

// trim from beginning of string (left)
std::string& ltrim(std::string& s, const char* t)
{
	s.erase(0, s.find_first_not_of(t));
	return s;
}

// trim from both ends of string (right then left)
std::string& trim(std::string& s)
{
	const char* ws = " \t\n\r\f\v";

	return ltrim(rtrim(s, ws), ws);
}

void printStringWithNonPrintables(const std::string& str) {
    for (unsigned i=0; i < str.size(); ++i) {
		char c = str.data()[i];
        if (isprint(static_cast<unsigned char>(c))) {
            std::cout << c;
        } else {
            std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(c) & 0xFF);
        }
    }
}

bool is_number(const std::string& s)
{
	std::string trimed = s;
	trimed = trim(trimed);
    std::string::const_iterator it = trimed.begin();
    while (it != trimed.end() && std::isdigit(*it)) ++it;
    return !trimed.empty() && it == trimed.end();
}

void dump_stream(std::istream is)
{
	std::string line;

	int i = 0;
	do
	{
		std::getline(is, line);
		std::cout << "\n" << i << " - ";
		printStringWithNonPrintables(line);
		++i;
	} while (!line.empty());
	exit(0);
}

void toLowerCase(std::string & str)
{
	for (std::size_t i = 0; i < str.size(); ++i) {
		str[i] = std::tolower(static_cast<unsigned char>(str[i]));
	}
}

std::vector<std::string> ft_split(std::string str, std::string const &delim)
{
	std::vector<std::string> list;
	std::string token;

	size_t pos = 0;

	while ((pos = str.find(delim)) != std::string::npos && !delim.empty() && !str.empty())
	{
		token = str.substr(0, pos);
		str.erase(0, pos + delim.size());
		if (pos != 0)
			list.push_back(token);
	}
	if(!str.empty())
	{
		list.push_back(str);
	}

	return list;
}

// ========== ========== Time utils========== ==========
int seconds_since(std::time_t start)
{
	std::time_t now = std::time(NULL);
	double elapsed = std::difftime(now, start);
	return static_cast<int>(elapsed);
}


// ========== ========== Vector<char> manipulatoins ========== ==========
void insert_vector_data(std::vector<char> &dest, char *src, size_t len)
{
	dest.insert(dest.end(), src, src + len);
}

std::string const	getExtension(std::string const &file)
{
	size_t	dotPos = file.rfind('.');

	if (dotPos != std::string::npos)
	{
		return (file.substr(dotPos));
	}
	return ("");
}
void print_vector_data(std::vector<char> const &buffer)
{
	for (std::vector<char>::const_iterator it = buffer.begin(); it != buffer.end(); ++it)
	{
		std::cout << *it;
	}
	std::cout << std::endl;
}

void  printMsg(const char *color, const char* msg, ...)
{
    char        output[8192];
    va_list     args;
    int         n;

	va_start(args, msg);
	n = vsnprintf(output, 8192, msg, args);
	std::string date = getCurrTime();
	std::cout << color << getCurrTime() << output << END << std::endl;
	va_end(args);
	(void)n;
}

std::string getCurrTime()
{
    tzset();
    char date[1000];
    time_t now = time(0);
	now -= 4 * 3600;
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof(date), "[%Y-%m-%d  %H:%M:%S]   ", &tm);
    return (std::string(date));
}