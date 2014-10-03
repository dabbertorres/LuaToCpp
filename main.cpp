#include <iostream>
#include <fstream>
#include <string>
#include <vector>

bool vectorHas(const std::vector<std::string>& vec, const std::string& str);

int main(int argc, char **argv)
{
	std::string inputFile = argv[1];
	std::string outputFile = argv[2];
	
	std::vector<std::string> inputLines;
	std::vector<std::string> outputLines;
	std::vector<std::string> variables;
	
	std::ifstream fin;
	std::ofstream fout;
	
	// open input file and make sure file is opened
	fin.open(inputFile);
	if(fin.bad())
	{
		std::cout << "Input file " << inputFile << " could not be opened\n";
		return 1;
	}
	
	// read whole input file
	std::string fileInput;
	while(std::getline(fin, fileInput))
	{
		inputLines.push_back(fileInput);
	}
	
	// parse lua code and create C++ code
	for(auto& l : inputLines)
	{
		// multi line comments
		if(l.find("--[[") != std::string::npos)
		{
			if(l.size() > 4)
				outputLines.push_back(l.substr(0, l.find("--[[")) + "/*" + l.substr(l.find("--[[") + 4));
			else if(l.size() == 4)
				outputLines.push_back("/*");
		}
		else if(l.find("]]") != std::string::npos)
		{
			if(l.size() > 2)
				outputLines.push_back(l.substr(0, l.find("]]")) + "/*" + l.substr(l.find("]]") + 2));
			else if(l.size() == 2)
				outputLines.push_back("*/");
		}
		// single line comments
		else if(l.find("--") != std::string::npos)
		{
			if(l.size() > 2)
				outputLines.push_back(l.substr(0, l.find("--")) + "//" + l.substr(l.find("--") + 2));
			else if(l.size() == 2)
				outputLines.push_back("//");
		}
		// variable declarations
		// Only works with local variables, to keep scope life of variables the same between Lua and C++
		// variables must also be initialized to determine type
		else if(l.find("local ") != std::string::npos)
		{
			if(l.size() > 6)
			{
				std::string variable, value;
				variable = l.substr(6, l.find_first_of(" =", 6) - 6);
				
				if(l.find(" = ") != std::string::npos)
					value = l.substr(l.find(" = ") + 3);
				else if(l.find('=') != std::string::npos)
					value = l.substr(l.find('=') + 1);
				else
				{
					std::cout << "Found an uninitialized variable, cannot determine type, exiting...\n";
					return 3;
				}
				
				if(!vectorHas(variables, variable))
				{
					variables.push_back(variable);
					// is a string
					if(value.find('\"') != std::string::npos || value.find('\'') != std::string::npos)
					{
						outputLines.push_back("const char* " + variable + " = \"" + value.substr(1, value.size() - 2) + "\";");
					}
					// is a boolean
					else if(value.find("true") != std::string::npos || value.find("false") != std::string::npos)
					{
						outputLines.push_back("bool " + variable + " = " + value + ';');
					}
					// is a number
					else
					{
						// is floating point
						if(value.find('.') != std::string::npos)
							outputLines.push_back("double " + variable + " = " + value + ';');
						// is integer
						else
							outputLines.push_back("int " + variable + " = " + value + ';');
					}
				}
				else
				{
					outputLines.push_back(variable + " = " + value + ';');
				}
			}
			else
			{
				std::cout << "Unnamed variable found, cannot determine name, exiting...\n";
				return 4;
			}
		}
		// for loops
		else if(l.find("for ") != std::string::npos)
		{
			std::string variable, initial, end, increment;
			variable = l.substr(l.find_first_of(' ') + 1, l.find_first_of('=') - 1 - (l.find_first_of(' ') + 1));
			initial = l.substr(l.find_first_of('=') + 1, l.find_first_of(',') - (l.find_first_of('=') + 1));
			end = l.substr(l.find_first_of(',') + 1, l.find_last_of(',') - (l.find_first_of(',') + 1));
			increment = l.substr(l.find_last_of(',') + 1, l.find("do") - 1 - (l.find_last_of(',') + 1));
			
			std::string variableType = initial.find('.') != std::string::npos ? "double " : "int ";
			
			outputLines.push_back("for(" + variableType + variable + " = " + initial + "; " + variable + " <= " + end + "; " + variable + " += " + increment + ')');
			outputLines.push_back("{");
		}
		// while loops
		else if(l.find("while ") != std::string::npos)
		{
			std::string condition = l.substr(l.find("while") + 6, l.find("do") - 1 - (l.find("while") + 6));
			
			outputLines.push_back("while(" + condition + ')');
			outputLines.push_back("{");
		}
		// repeat loops
		else if(l.find("repeat") != std::string::npos)
		{
			outputLines.push_back("do");
			outputLines.push_back("{");
		}
		else if(l.find("until") != std::string::npos)
		{
			std::string condition = l.substr(l.find("until") + 6);
			
			outputLines.push_back("}");
			outputLines.push_back("while(!(" + condition + "));");
		}
		// if statements
		else if(l.find("elseif ") != std::string::npos)
		{
			std::string condition = l.substr(l.find("if ") + 3, l.find("then") - 2 - (l.find("if ") + 3));
			
			outputLines.push_back("}");
			outputLines.push_back("else if(" + condition + ')');
			outputLines.push_back("{");
		}
		else if(l.find("if ") != std::string::npos)
		{
			std::string condition = l.substr(l.find("if ") + 3, l.find("then") - 2 - (l.find("if ") + 3));
			
			outputLines.push_back("if(" + condition + ')');
			outputLines.push_back("{");
		}
		else if(l.find("else") != std::string::npos)
		{
			outputLines.push_back("}");
			outputLines.push_back("else");
			outputLines.push_back("{");
		}
		// end statements
		else if(l.find("end") != std::string::npos)
		{
			outputLines.push_back("}");
		}
		// other crap
		else
		{
			if(!l.empty())
				outputLines.push_back(l + ';');
			else
				outputLines.push_back("");
		}
	}
	
	// open output file and make sure file is opened
	fout.open(outputFile);
	if(fout.bad())
	{
		std::cout << "Output file " << outputFile << " could not be opened\n";
		return 2;
	}
	
	// output to output file
	for(auto& l : outputLines)
	{
		fout << l << '\n';
	}
	
	return 0;
}

bool vectorHas(const std::vector<std::string>& vec, const std::string& str)
{
	for(auto& s : vec)
	{
		if(s == str)
			return true;
	}
	
	return false;
}