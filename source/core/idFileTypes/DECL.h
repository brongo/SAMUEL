#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <locale> // std::isalpha

#include "../Utilities.h"
#include "../ResourceManager.h"

namespace HAYDEN {
    class DeclFile {
    public:

        DeclFile(const ResourceManager &resourceManager, const std::string &resourcePath);

        void parse();

        [[nodiscard]] bool loaded() const { return m_loaded; }

        [[nodiscard]] const std::vector<char> &data() const { return m_data; }

        [[nodiscard]] const jsonxx::Object& json() const { return m_parsed; }

    private:

        bool m_loaded;
        std::vector<char> m_data;
        jsonxx::Object m_parsed;
    };
}