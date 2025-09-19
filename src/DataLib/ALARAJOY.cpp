#include "ALARAJOY.h"
#include "DataLib/ALARALib_def.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

// Define static member
std::vector<DSVRow> ALARAJOYLib::dsvData;

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

// Clean out quotes and whitespaces from cross section list
std::string ALARAJOYLib::cleanXSectionString(std::stringstream& ss)
{
    std::string pyList;
    std::getline(ss, pyList);

    // Trim surrounding whitespace
    auto l = pyList.find_first_not_of(" \t");
    auto r = pyList.find_last_not_of(" \t");
    if (l == std::string::npos) pyList.clear();
    else pyList = pyList.substr(l, r - l + 1);

    // Strip optional surrounding quotes
    if (!pyList.empty() && (pyList.front()=='\"' || 
        pyList.front()=='\'')) pyList.erase(pyList.begin());
    if (!pyList.empty() && (pyList.back()=='\"'  || 
        pyList.back()=='\''))  pyList.pop_back();
    
    return pyList;
}

DSVRow ALARAJOYLib::parseDSVRow(const std::string& line)
{
    DSVRow row;
    std::stringstream ss(line);
    int index;

    ss >> index; 
    ss >> row.parentKZA >> row.daughterKZA;
    ss >> row.emittedParticles >> row.nonZeroGroups;

    std::string pyList = cleanXSectionString(ss);
    row.crossSections = parseXSectionArray(pyList);

    return row;
}

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

/* Empty Destructor
   (by linking decay methods through EAFLib,
   destruction is automaticaly handled by ~EAFLib() ) */
ALARAJOYLib::~ALARAJOYLib(){}

// Pre-load entire DSV into memory
void ALARAJOYLib::loadDSVData()
{
    dsvData.clear();
    currentRowIndex = 0;
    currentParent = -1;
    std::string line;

    // Skip header line
    std::getline(inTrans, line);

    // Read all data rows
    while (std::getline(inTrans, line))
    {
        if (line.empty()) continue; // Skip empty lines

        DSVRow row = parseDSVRow(line);
        dsvData.push_back(row);
    }

    // Set initial parent
    if (!dsvData.empty()) {
        currentParent = dsvData[0].parentKZA;
    }
}

// Read library header information
void ALARAJOYLib::getTransInfo()
{
    // Fixed for Vitamin J energy group structure
    nGroups = 175;
    nParents = 0;

    // Allocate arrays for maximum reactions
    transKza = new int[MAXALARAJOYRXNS];
    xSection = new float*[MAXALARAJOYRXNS];
    emitted = new char*[MAXALARAJOYRXNS];

    for (int rxnNum = 0; rxnNum < MAXALARAJOYRXNS; rxnNum++)
    {
        xSection[rxnNum] = new float[nGroups];
        emitted[rxnNum]  =        new char[6];
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