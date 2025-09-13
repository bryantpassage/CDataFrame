#include <iostream>
#include "csv.h"


int main()
{
    CSVReader reader;
    reader.read("testfiles/addresses.csv");
    
    // reader.print();

    CSVData myData = reader;

    std::cout << myData << std::endl;
    
    std::cout << "Hello" << std::endl;
}