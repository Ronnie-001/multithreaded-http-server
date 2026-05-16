#include "m_strings.h"
#include <algorithm>
#include <iostream>
#include <iterator>

namespace cerberus
{
    namespace string 
    {
        int caseInsensitiveSearch(const std::string& str1, const std::string& str2)
        {
            auto it = std::search(str1.begin(), str1.end(),
                                  str2.begin(), str2.end());
            
            if (it != str1.end()) {
                return std::distance(str1.begin(), it);
            }

            return -1;
        }

        void printRawCharacters(const std::string& str) 
        {
            for (char c : str) {
                switch (c) {
                    case '\n': std::cout << "\\n"; break;
                    case '\t': std::cout << "\\t"; break;
                    case '\r': std::cout << "\\r"; break;
                    case '\\': std::cout << "\\\\"; break;
                    default:   std::cout << c;    break;
                }

            }
        }
    }
}
