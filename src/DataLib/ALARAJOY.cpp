#include "ALARAJOY.h"
#include "DataLib/ALARALib_def.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

<<<<<<< HEAD
// Define DSV row parsing structure
std::vector<DSVRow> ALARAJOYLib::dsvData;
=======
// Define static member
std::vector<CSVRow> ALARAJOYLib::csvData;

// Parse cross section array from Python list (of floats) format
std::vector<float> ALARAJOYLib::parseXSectionArray(
    const std::string& arrayStr
) {
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

    std::getline(ss, item, ',');
    row.parentKZA = std::stoi(item);

    std::getline(ss, item, ',');
    row.daughterKZA = std::stoi(item);

    std::getline(ss, item, ',');
    row.emittedParticles = item;

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
    if (!rest.empty() && (rest.front()=='\"' || 
        rest.front()=='\'')) rest.erase(rest.begin());
    if (!rest.empty() && (rest.back()=='\"'  || 
        rest.back()=='\''))  rest.pop_back();

    row.crossSections = parseXSectionArray(rest);

    return row;
}
>>>>>>> 20646b5 (Fixing formatting issues.)

// Constructor
ALARAJOYLib::ALARAJOYLib(
    const char* transFname, const char* decayFname, const char* alaraFname
) : EAFLib(decayFname, true),
    currentRowIndex(0),
    currentParent(-1)
{
    // Open the DSV file
    inTrans.open(transFname, ios::in);
    if (inTrans.is_open()) {
        loadDSVData(); // Pre-load entire DSV
        makeBinLib(alaraFname);
    }
}

// Pre-load entire DSV into memory
void ALARAJOYLib::loadDSVData()
{
    dsvData.clear();
    currentRowIndex = 0;
    currentParent = -1;

    while (true)
    {
        DSVRow row;

        // Extract Parent KZA, EOF at pKZA == -1
        if (!(inTrans >> row.parentKZA) || row.parentKZA == -1) break;

        // Extract Daughter KZA, emitted particles, and non-zero groups
        inTrans >> row.daughterKZA;
        inTrans >>row.emittedParticles >> row.nonZeroGroups;

        // Iterate through rest of line for cross sections based on nGroups
        row.crossSections = std::vector<float>(row.nonZeroGroups);
        for (int i = 0; i < row.nonZeroGroups; i++)
        {
            inTrans >> row.crossSections[i];
        }

        dsvData.push_back(std::move(row));
    }

    // Set initial parent
    if (!dsvData.empty()){
    currentParent = dsvData[0].parentKZA;
    }
}

// Read library header information
void ALARAJOYLib::getTransInfo()
{

    nParents = 0;

    // Allocate arrays for maximum reactions
    transKza = new int[MAXALARAJOYRXNS];
    xSection = new float*[MAXALARAJOYRXNS];
    emitted = new char*[MAXALARAJOYRXNS];

    for (int rxnNum = 0; rxnNum < MAXALARAJOYRXNS; rxnNum++) {
        xSection[rxnNum] = nullptr;
        emitted[rxnNum]  = new char[MAXALARAJOYEMITTEDSTR];
    }
}

// Read transmutation data for next parent isotope
int ALARAJOYLib::getTransData()
{
    if (currentRowIndex >= dsvData.size()) {
        return LASTISO; // end of data
    }

    int rxnNum = 0;
    int parentKZA = currentParent;

    // Find maximum nonZeroGroups for this parent
    int maxGroups = 0;
    size_t scanIndex = currentRowIndex;
    while (scanIndex < dsvData.size() &&
        dsvData[scanIndex].parentKZA == currentParent) {
            if (dsvData[scanIndex].nonZeroGroups > maxGroups)
                maxGroups = dsvData[scanIndex].nonZeroGroups;
            scanIndex++;
        }
    nGroups = maxGroups;

    // Reallocate xSection arrays for this parent
    for (int rxn = 0; rxn < MAXALARAJOYRXNS; rxn++) {
        delete [] xSection[rxn];
        xSection[rxn] = new float[nGroups]();
    }

    // Read all reactions for current parent
    while (currentRowIndex < dsvData.size() && 
        dsvData[currentRowIndex].parentKZA == currentParent)
    {
        const DSVRow& row = dsvData[currentRowIndex];

        // Store reaction data
        transKza[rxnNum] = row.daughterKZA;
        strcpy(emitted[rxnNum], row.emittedParticles.c_str());

        // Fill cross sections
        for (int g = 0; g < nGroups; g++)
        {
            xSection[rxnNum][g] = 
                (g < row.nonZeroGroups) ? row.crossSections[g] : 0.0;
        }

        rxnNum++;
        currentRowIndex++;
    }

    // Set return values
    nTRxns = rxnNum;

    // Move to next parent for next call
    if (currentRowIndex < dsvData.size()) {
        currentParent  = dsvData[currentRowIndex].parentKZA;
    } else {
        currentParent = -1; // End of data
    }

    return parentKZA;
}