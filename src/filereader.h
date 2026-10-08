#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>

using namespace std;

class Filereader
{
private:
    ifstream file; // here ifstream of fstream is creating an object "file"
    unordered_map<string, map<int, int>> wordfrequency;

public:
    bool openfile(string filename);
    bool readfile(string filename, string &line);
    bool buildindex(string filename);
    void searchword(string word);
    bool isallowedpunct(char x);
};