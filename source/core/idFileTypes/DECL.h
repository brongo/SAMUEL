#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <locale> // std::isalpha

#include "../Utilities.h"

namespace HAYDEN
{
    class DeclSingleLine
    {
        public:
            std::string GetLineVariable() const { return _LineVariable; };
            std::string GetLineValue() const { return _LineValue; };
            void ReadFromStream(std::ifstream& input, std::string line);

        private:
            int _FormatIsGood = 1;
            std::string _LineStart = "";
            std::string _LineVariable = "";
            std::string _LineAssignment = "";
            std::string _LineValue = "";
            std::string _LineTerminator = "";
    };

    class DeclFile
    {
        public: 
            int LineCount = 0;
            std::string GetFileName() const { return _DeclFileName; };
            DeclSingleLine GetLineData(int lineNumber) const { return _LineData[lineNumber]; }
            void SetFileName(std::string fileName) { _DeclFileName = fileName; }
            void SetLineData(DeclSingleLine lineData) { _LineData.push_back(lineData); }

        private:
            std::vector<DeclSingleLine> _LineData;
            std::string _DeclFileName;
    };
}