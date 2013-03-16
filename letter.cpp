#include "letter.hpp"

const char letter::emptychain_buff = 0;
letter* letter::nochoice = reinterpret_cast<letter*>(const_cast<char*>(&letter::emptychain_buff));
