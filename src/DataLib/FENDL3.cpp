#include "Fendl3.h"
#include "DataLib/ALARALib_def.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

// Data structure to hold CSV rows
struct CSVRow {
    int parentKZA;
    int daughterKZA;
    std::string emittedParticles;
    int nonZeroGroups;
    std::vector<float> crossSections;
};

// Global variables to hold pre-loaded CSV data
static std::vector<CSVRow> csvData;
static size_t currentRowIndex = 0;
static int currentParent = -1;

/// Free helper: Parse cross section array from Python list (of floats) format
static std::vector<float> parseXSectionArray(const std::string& arrayStr) {
    std::vector<float> xs;
    if (arrayStr.size() < 2) return xs;

    std::string clean = arrayStr.substr(1, arrayStr.size()-2);
    std::stringstream ss(clean);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        size_t a = item.find_first_not_of(" \t");
        size_t b = item.find_last_not_of(" \t");
        if (a == std::string::npos) continue;
        xs.push_back(std::stof(item.substr(a, b - a + 1)));
    }
    return xs;
}

// Free helper: Parse a single CSV row
static CSVRow parseCSVRow(const std::string& line)
{
    CSVRow row;
    std::stringstream ss(line);
    std::string item;

    // Column 0: Index (skip)
    std::getline(ss, item, ',');

    // Column 1: Parent KZA
    std::getline(ss, item, ',');
    row.parentKZA = std::stoi(item);

    // Column 2: Daughter KZA
    std::getline(ss, item, ',');
    row.daughterKZA = std::stoi(item);

    // Column 3: Emitted Particles
    std::getline(ss, item, ',');
    row.emittedParticles = item;

    // Column 4: Non-Zero Groups
    std::getline(ss, item, ',');
    row.nonZeroGroups = std::stoi(item);

    // Column 5: Cross Sections
    std::string rest;
    std::getline(ss, rest);

    // Trim surrounding whitespace
    auto l = rest.find_first_not_of(" \t");
    auto r = rest.find_last_not_of(" \t");
    if (l == std::string::npos) rest.clear();
    else rest = rest.substr(l, r - l + 1);

    // Strip optional surrounding quotes
    if (!rest.empty() && (rest.front()=='\"' || rest.front()=='\'')) rest.erase(rest.begin());
    if (!rest.empty() && (rest.back()=='\"'  || rest.back()=='\''))  rest.pop_back();

    row.crossSections = parseXSectionArray(rest);
    
    return row;
}

// Free helper: Running the Python preprocessor and storing data path
std::string pythonPreprocess() {

  char buffer[256];
  std::string result;

  FILE* pipe = popen("python ./fendl32B_retrofit/process_fendl3.2.py 2>&1", "r");

  if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result = buffer;
  }

  int ret = pclose(pipe);

  // Remove trailing newline
  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }

  return result;

}

// Constructor
FENDL3LIB::FENDL3LIB(
    const char *transFname, const char *decayFname, const char *alaraFname
) : ASCIILib(DATALIB_FENDL3)
{
    // Run Python preprocessing to get CSV Path
    std::string csvPath = pythonPreprocess();
    
    // Open the CSV file
    inTrans.open(csvPath, ios::in);
    if (inTrans.is_open()) {
        loadCSVData(); // Pre-load entire CSV
        makeBinLib(alaraFname);
    }
}

// Destructor
FENDL3LIB::~FENDL3LIB()
{
    if (inTrans.is_open())
        inTrans.close();

    // Clean up allocated arrays (if they exist)
    if (xSection != NULL) {
        for (int rxnNum = 0; rxnNum < MAXFENDL3RXNS; rxnNum++) {
            delete xSection[rxnNum];
        }
        delete xSection;
    }

    if (emitted != NULL) {
        for (int rxnNum = 0; rxnNum < MAXFENDL3RXNS; rxnNum++) {
            delete emitted[rxnNum];
        }
        delete emitted;
    }

    if (transKza != NULL) {
        delete transKza;
    }
}

// Pre-load entire CSV into memory
void FENDL3LIB::loadCSVData()
{
    csvData.clear();
    currentRowIndex = 0;
    currentParent = -1;
    std::string line;

    // Skip header line
    std::getline(inTrans, line);

    // Read all data rows
    while (std::getline(inTrans, line))
    {
        if (line.empty()) continue; // Skip empty lines
    
        CSVRow row = parseCSVRow(line);
        csvData.push_back(row);
    }

    // Set initial parent
    if (!csvData.empty()) {
        currentParent = csvData[0].parentKZA;
    }
}

// Read library header information
void FENDL3LIB::getTransInfo()
{
    // Fixed for Vitamin J energy group structure
    nGroups = 75;

    // Allocate arrays for maximum reactions
    transKza = new int[MAXFENDL3RXNS];
    xSection = new float*[MAXFENDL3RXNS];
    emitted = new char*[MAXFENDL3RXNS];

    for (int rxnNum = 0; rxnNum < MAXFENDL3RXNS; rxnNum++)
    {
        xSection[rxnNum] = new float[nGroups];
        emitted[rxnNum]  =        new char[6];
    }
}

// Read transmutation data for next parent isotope
int FENDL3LIB::getTransData()
{
    if (currentRowIndex >= csvData.size()) {
        return LASTISO; // end of data
    }

    int rxnNum = 0;
    int parentKZA = currentParent;

    // Read all reactions for current parent
    while (currentRowIndex < csvData.size() && 
    csvData[currentRowIndex].parentKZA == currentParent)
    {
        const CSVRow& row = csvData[currentRowIndex];

        // Store reaction data
        transKza[rxnNum] = row.daughterKZA;
        strcpy(emitted[rxnNum], row.emittedParticles.c_str());

        // Fill cross sections (75 groups total)
        for (int g = 0; g < nGroups; g++)
        {
            xSection[rxnNum][g] = (g < row.nonZeroGroups) ? row.crossSections[g] : 0.0;
        }

        rxnNum++;
        currentRowIndex++;
    }

    // Set return values
    nTRxns = rxnNum;

    // Move to next parent for next call
    if (currentRowIndex < csvData.size()) {
        currentParent  = csvData[currentRowIndex].parentKZA;
    } else {
        currentParent = -1; // End of data
    }

    return parentKZA;
}

// Stub implementations for decay data
void FENDL3LIB::getDecayInfo()
{
  // No decay data needed for FENDL3
}

int FENDL3LIB::getDecayData()
{
  return LASTISO;  // No decay data
}