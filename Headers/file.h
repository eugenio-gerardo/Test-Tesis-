#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
using namespace std;
class file {
private string separator filename;
private char separator;
public file(string & name, char & separator);
public vector readfile();
}


#endif // FILE_H_INCLUDED
