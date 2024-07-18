#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <limits.h>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>

// Constant for the directory path
const std::string RETROFIT_DIR = "/fendl32B_retrofit";

// Function to get the current working directory
std::string getCurrentWorkingDirectory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        return std::string(cwd);
    } else {
        std::cerr << "Error: Could not get the current working directory." << std::endl;
        return "";
    }
}

// Function to execute the Python script
int executePythonScript(const std::string& scriptPath, int argc, char* argv[]) {
    std::string command = "python3 " + scriptPath;

    // Append any additional arguments
    for (int i = 1; i < argc; ++i) {
        command += " ";
        command += argv[i];
    }

    // Execute the Python script
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Error: Python script execution failed." << std::endl;
        return 1;
    }
    return 0;
}

// Function to read a CSV file
std::vector<std::vector<std::string>> readCSV(const std::string& csvPath) {
    std::ifstream file(csvPath);
    std::vector<std::vector<std::string>> data;
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the CSV file." << std::endl;
        return data;
    }

    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string value;

        while (std::getline(ss, value, ',')) {
            row.push_back(value);
        }
        data.push_back(row);
    }

    file.close();
    return data;
}

int main(int argc, char* argv[]) {
    std::string cwd = getCurrentWorkingDirectory();
    if (!cwd.empty()) {
        std::string pythonScriptPath = cwd + RETROFIT_DIR + "/fendl3_retrofit.py";
        if (executePythonScript(pythonScriptPath, argc, argv) == 0) {
            std::string csvPath = cwd + RETROFIT_DIR + "/gendf_data.csv";
            std::vector<std::vector<std::string>> csvData = readCSV(csvPath);

        } else {
            return 1;
        }
    }
    return 0;
}
