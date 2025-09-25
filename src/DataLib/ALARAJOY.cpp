#include "ALARAJOY.h"
#include "DataLib/ALARALib_def.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

// Define DSV row parsing structure
std::vector<DSVRow> ALARAJOYLib::dsvData;

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
        emitted[rxnNum]  = new char[6];
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