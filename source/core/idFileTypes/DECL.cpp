#include "DECL.h"

namespace HAYDEN
{
    // Parse a single line of a .decl file that has been read into std::ifstream
    void DeclSingleLine::ReadFromStream(std::ifstream& input, std::string line)
    {
        size_t splitPos = 0;

        // for identifying lines with no alphanumeric characters
        auto it = std::find_if(line.begin(), line.end(), [](char c) 
        {
            return std::isalpha(c, std::locale());
        });

        if (it != line.end())
        {
            splitPos = (line.length() - (line.end() - it));
            _LineStart = line.substr(0, splitPos);
            line = line.substr(splitPos, line.length() - splitPos);

            splitPos = line.find(" = ");
            if (splitPos != -1)
            {
                _LineVariable = line.substr(0, splitPos);
                _LineAssignment = line.substr(splitPos, 3);
                _LineValue = line.substr(splitPos + 3, line.length() - (splitPos + 3) - 1);
                _LineTerminator = line.substr(line.length() - 1, 1);

                // support for unix line endings
                if (_LineTerminator == "\r")
                {
                    _LineValue = line.substr(splitPos + 3, line.length() - (splitPos + 3) - 2);
                    _LineTerminator = line.substr(line.length() - 2, 1);
                }
            }
            else
            {
                // for "editorvars" format
                _LineVariable = line;
                _FormatIsGood = 0;
            }
        }
        else
        {
            // catchall if line contains no alphanumeric values
            _LineStart = line;
        }
    }
}