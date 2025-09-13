#include "csv.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>

CSVReader::CSVReader():
    io_buffer(nullptr), io_buffer_size(0)
{

}

CSVReader::CSVReader(const CSVReader& rhs)
{
    if (this != &rhs)
    {
        this->io_buffer = new char[rhs.io_buffer_size];
        std::memcpy((void*)this->io_buffer, (void*)rhs.io_buffer, rhs.io_buffer_size * sizeof(char));
        this->io_buffer_size = rhs.io_buffer_size;
    }
}

CSVReader::CSVReader(CSVReader&& rhs) noexcept
{
    if (this != &rhs)
    {
        this->io_buffer = rhs.io_buffer;
        this->io_buffer_size = rhs.io_buffer_size;
        rhs.io_buffer = nullptr;
        rhs.io_buffer_size = 0;
    }
}

CSVReader::~CSVReader()
{
    if (this->io_buffer != nullptr)
    {
        delete[] this->io_buffer;
    }
    this->io_buffer_size = 0;
}

CSVReader& CSVReader::operator=(const CSVReader& rhs)
{
    if (this != &rhs)
    {
        delete[] this->io_buffer;
        this->io_buffer = new char[rhs.io_buffer_size];
        std::memcpy((void *)this->io_buffer, (void *)rhs.io_buffer, rhs.io_buffer_size * sizeof(char));
        this->io_buffer_size = rhs.io_buffer_size;
    }

    return *this;
}

CSVReader& CSVReader::operator=(CSVReader&& rhs) noexcept
{
    if (this != &rhs)
    {
        delete[] this->io_buffer;
        this->io_buffer = rhs.io_buffer;
        this->io_buffer_size = rhs.io_buffer_size;
        rhs.io_buffer = nullptr;
        rhs.io_buffer_size = 0;
    }

    return *this;
}

void CSVReader::read(const char* path, bool header)
{
    // open file and read total bytes
    std::ifstream csvfile(path, std::ifstream::binary);
    if (csvfile)
    {
        csvfile.seekg(0, csvfile.end);
        std::size_t length = csvfile.tellg();
        csvfile.seekg(0, csvfile.beg);

        // allocate memory
        this->io_buffer = new char[length];

        // read data
        csvfile.read(this->io_buffer, length);
        this->io_buffer_size = length;

        csvfile.close();
    }
}

void CSVReader::read(std::string& path, bool header)
{
    CSVReader::read(path.c_str());
}

bool CSVReader::hasHeader() const { return this->header; }

std::size_t CSVReader::size() const { return this->io_buffer_size; }

const char* CSVReader::getBuffer() const { return this->io_buffer; }

void CSVReader::print()
{
    // add null terminating character at end of io_buffer
    char* cstring_buffer = new char[this->io_buffer_size + 1];
    std::memcpy((void*)cstring_buffer, (void*)this->io_buffer, this->io_buffer_size);
    cstring_buffer[this->io_buffer_size] = '\0';

    std::cout << cstring_buffer << std::endl;

    delete[] cstring_buffer;
}

CSVToken CSVData::findNextToken(const std::string& str)
{
    if (str.find((char)CSVToken::Comma) == std::string::npos && str.find((char)CSVToken::Quotation) == std::string::npos)
        return CSVToken::Comma;
    return str.find((char) CSVToken::Comma) < str.find((char) CSVToken::Quotation) ? CSVToken::Comma : CSVToken::Quotation;
}

std::vector<std::string> CSVData::parseStrLine(const std::string& line)
{
    std::vector<std::string> row;
    std::stringstream sLine(line);
    do
    {
        // skip if empty line
        if (line.empty()) continue;

        std::string cell;
        std::string remainingSubstr = sLine.str().substr(sLine.tellg());
        std::size_t firstQuote;    
        std::size_t secondQuote;
        std::size_t validComma;

        CSVToken nextToken = findNextToken(remainingSubstr);
        
        switch (nextToken)
        {
            case CSVToken::Quotation:
                // remainingSubstr = sLine.str().substr(sLine.tellg());

                firstQuote = remainingSubstr.find('"');

                // get escaped quotes
                secondQuote = firstQuote;
                while (remainingSubstr.find("\"\"", secondQuote + 1) == remainingSubstr.find('"', secondQuote + 1))
                    secondQuote = remainingSubstr.find('"', secondQuote + 1) + 1;
                secondQuote = remainingSubstr.find('"', secondQuote + 1);
                
                validComma = remainingSubstr.find(',', secondQuote);
                cell = remainingSubstr.substr(0, validComma);
                sLine.ignore(validComma+1);
                break;
            case CSVToken::Comma:
                std::getline(sLine, cell, ',');
                break;
            default:
                break;
        }
        row.emplace_back(std::move(cell));
    } while (!sLine.eof());

    return row;
    
}

CSVHeader CSVData::parseHeader(const std::string& sHeader)
{
    std::vector<std::string> vHeader = parseStrLine(sHeader);
    return CSVHeader{ vHeader, vHeader.size() };
}

CSVRow CSVData::parseRow(const std::string& sRow)
{
    std::vector<std::string> vRow = parseStrLine(sRow);
    return CSVRow{ vRow, vRow.size() };
}

CSVData::CSVData(const CSVReader& in_reader)
{
    std::string io_buffer_to_string(in_reader.getBuffer(), in_reader.size());
    std::stringstream bufferstream(io_buffer_to_string);

    // obtain header row
    if (in_reader.hasHeader())
    {
        std::string line;
        std::getline(bufferstream, line);
        this->header = parseHeader(line);
    }

    for (std::string sRowLine; std::getline(bufferstream, sRowLine);)
    {
        this->data.push_back(parseRow(sRowLine));
    }

    // validate CSV data here
    // FIXME

    std::size_t csv_width = 0;
    if (in_reader.hasHeader())
        csv_width = this->header.size;
    else
        csv_width = this->data.at(0).size;

    this->__shape = std::pair<std::size_t, std::size_t>(this->data.size(), csv_width);
    this->__size = this->__shape.first * this->__shape.second;
}

std::pair<std::size_t, std::size_t> CSVData::shape() const
{
    return this->__shape;
}

std::vector<std::size_t> CSVData::getMaxColStrLen() const
{
    std::vector<std::size_t> maxw_arr(this->__shape.second, 0);
    int i = 0;
    for (std::vector<std::string>::const_iterator it=this->header.values.begin(); it != this->header.values.end(); it++, i++)
        maxw_arr[i] = std::max(maxw_arr[i], it->size());
    
    for (std::vector<CSVRow>::const_iterator it = this->data.begin(); it != this->data.end(); it++)
    {
        int j = 0;
        for (std::vector<std::string>::const_iterator jt = it->values.begin(); jt != it->values.end(); jt++, j++)
            maxw_arr[j] = std::max(maxw_arr[j], jt->size());
    }

    return maxw_arr;
}

std::ostream& operator<<(std::ostream& lhs, const CSVData& rhs)
{
    std::stringstream outstream;
    std::vector<std::size_t> max_col_width = rhs.getMaxColStrLen();

    const unsigned int EXTRA_WIDTH = 2;

    // header
    for (std::size_t i = 0; i < rhs.shape().second; i++)
    {
        outstream << std::setw(max_col_width[i] + EXTRA_WIDTH) << std::setiosflags(std::ios_base::right) << rhs.header.values[i];
    }
    outstream << std::endl;

    // data
    for (std::size_t i = 0; i < rhs.shape().first; i++)
    {
        for (std::size_t j = 0; j < rhs.shape().second; j++)
            outstream << std::setw(max_col_width[j] + EXTRA_WIDTH) << std::setiosflags(std::ios_base::right) << rhs.data[i].values[j];
        
        outstream << std::endl;
    }

    lhs << outstream.str();

    return lhs;
}