#include "ALARAJOY.h"
#include "DataLib/ALARALib_def.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

// Define DSV row parsing structure
std::vector<ALARAJOYLib::DSVRow> ALARAJOYLib::dsvData;

// Constructor
ALARAJOYLib::ALARAJOYLib(
    const char* transFname, const char* decayFname, const char* alaraFname
) : EAFLib(decayFname),
    currentRowIndex(0),
    currentParent(-1)
{
    // Open the DSV file
    inTrans.open(transFname, ios::in);
    if (inTrans.is_open()) {
//        getTransInfo();
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
    DSVRow row;

    // Read header containing energy group number
    inTrans >> nGroups;

    // Extract Parent KZA until EOF at pKZA == -1
    while ((inTrans >> row.parentKZA) && row.parentKZA != -1)
    {
        // Extract Daughter KZA, emitted particles, and non-zero groups
        inTrans >> row.daughterKZA;
        inTrans >> row.emittedParticles >> row.nonZeroGroups;

        /* Iterate through rest of line for cross sections 
           based on non-zero groups*/
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

    inTrans >> nGroups;
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

        // Fill non-zero cross sections
        std::memcpy(xSection[rxnNum],
                    row.crossSections.data(), 
                    row.nonZeroGroups * sizeof(float)
        );

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