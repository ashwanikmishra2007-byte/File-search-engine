#include <iostream>      // input/output
#include <fstream>       // file handling
#include <map> // hash map
#include <string>        // string
#include <map>
#include <cctype>
#include "filereader.h"
using namespace std;

bool Filereader::readfile(string filename, string &line)
{
    if (!file.is_open())
    {
        file.open(filename);
    }

    if (!file.is_open())
    {
        return false;
    }

    if (getline(file, line))
    {
        return true;
    }
    return false;
}

bool Filereader::isallowedpunct(char x)
{ // Helper function to identify allowed punctuations
    return x == '+' || x == '#' || x == '.' || x == '_';
}

bool Filereader::buildindex(string filename)
{
    string line;
    string word;
    if (!file.is_open())
    {
        file.open(filename);
    }

    if (!file.is_open())
    {
        cout << "Could not open file" << endl;
        return false;
    }

    int linenum = 1; // to track line number

    while (readfile(filename, line))
    {

        for (auto x : line)
        {
            if (!isspace(x) && (!ispunct(x) || isallowedpunct(x)))
            { // checking for punctuations and spaces
                word = word + x;
            }
            else
            {
                if (word != "")
                {
                    wordfrequency[word][linenum]++;
                    word = "";
                }
            }
        }
        if (word != "")
        { // reads last word of line
            wordfrequency[word][linenum]++;
            word = "";
        }

        linenum = linenum + 1;
    }
    return true;
}

void Filereader::searchword(string word)
{

    auto it = wordfrequency.find(word); //.find(word) returns an iterator pointing to the map element whose key is word, if that key exists.

    if (it != wordfrequency.end())
    {
        cout << "Word had been found ";

        for (auto x : it->second)
        {
            cout << "Line " << x.first
                 << ": " << x.second << " times" << endl;
        }
    }
    else
    {
        cout << "Word had not been found";
        cout << endl;
        return;
    }
}

int main()
{
    Filereader reader;
    string name = "sample1.txt";
    int choice;
    string line;
    string word;
    do
    {
        cout << "1 for finding a word in a file 2 for exit";
        cin >> choice;
        cout << endl;
        switch (choice)
        {
        case 1:
            if (!reader.buildindex(name))
            {
                cout << "Some error in reading the file";
                cout << endl;
            }
            else
            {
                cout << "Enter the word to be found ";
                cin >> word;
                cout << endl;
                reader.searchword(word);
            }
            break;
        }
    } while (choice != 2);
}
