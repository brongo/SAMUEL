#pragma once
#include <vector>
#include <string>

#include "ResourceFileReader.h"

namespace HAYDEN
{
    // This file has been made mostly obsolete. 
    // Currently just stores these structs which are used for globally-loaded resources.  
    struct RESOURCES_ARCHIVE
    {
        std::string ResourceName;
        fs::path ResourcePath;
        std::vector<ResourceEntry> Entries;
    };

    struct GLOBAL_RESOURCES
    {
        std::vector<RESOURCES_ARCHIVE> Files;
    };
}