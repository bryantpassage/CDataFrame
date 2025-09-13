#ifndef CSV_H
#define CSV_H

#include <cstddef>
#include <string>
#include <vector>
#include <iostream>

// CSV tokens
enum class CSVToken
{
    Comma = ',',
    Quotation = '"'
};

class CSVReader
{
private:
    char* io_buffer;
    std::size_t io_buffer_size;
    bool header;

public:

    // constructors
    CSVReader();                        // default
    CSVReader(const CSVReader&);        // copy
    CSVReader(CSVReader&&) noexcept;    // move
    ~CSVReader();

    CSVReader& operator=(const CSVReader&);     // copy-assignment
    CSVReader& operator=(CSVReader&&) noexcept; // move-assignment

    void read(const char* path, bool header=true);
    void read(std::string& path, bool header=true);

    // member getter functions
    bool hasHeader() const;
    std::size_t size() const;
    const char* getBuffer() const;

    void print();
};

class CSVHeader
{
public:
    std::vector<std::string> values;
    std::size_t size;
};

class CSVRow
{
public:
    std::vector<std::string> values;
    std::size_t size;
};

class CSVData
{
public:
    CSVHeader header;
    std::vector<CSVRow> data;
    std::pair<std::size_t, std::size_t> __shape;
    std::size_t __size;

    CSVToken findNextToken(const std::string&);
    std::vector<std::string> parseStrLine(const std::string&);
    CSVHeader parseHeader(const std::string&);
    CSVRow parseRow(const std::string&);

public:
    // constructors
    CSVData();
    CSVData(const CSVReader&);

    CSVData(const CSVData&);
    CSVData(const CSVData&&);

    CSVData& operator=(const CSVData&);
    CSVData& operator=(const CSVData&&) noexcept;

    std::pair<std::size_t, std::size_t> shape(void) const;

    std::vector<std::size_t> getMaxColStrLen(void) const;
};

std::ostream& operator<<(std::ostream&, const CSVData&);

#endif