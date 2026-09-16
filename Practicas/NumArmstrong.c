#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>

class Kata {
public:
  static int DuplicateCount(std::string str) {
    std::vector<char> repChars;
    
    // Convertir string a minúsculas
    std::string lowerStr = str;
    for (char &c : lowerStr) {
        c = std::tolower(c);
    }
    
    std::vector<char> list(lowerStr.begin(), lowerStr.end());
    
    for(int i = 0; i < list.size(); i++) {
      if(duplicated(list[i], list) && list[i] != ' ') {
        char h = list[i];
        for(int j = 0; j < list.size(); j++) {
          if(list[j] == h) {
            list[j] = ' ';
          }
        }
        repChars.push_back(h);
      }
    }
    return repChars.size();
  }
  
  static bool duplicated(char charct, std::vector<char>& list) {
    charct = std::tolower(charct);
    int count = 0;
    
    for(int i = 0; i < list.size(); i++) {
      if(std::isalpha(list[i])) {
        if(charct == std::tolower(list[i])) {
          count = count + 1;
        }
      } else if(charct == list[i]) {
          count = count + 1;
      }
    }
    if(count >= 2) {
      return true;
    }
    return false;
  }
};



