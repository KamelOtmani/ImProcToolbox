#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

namespace utility
{
	auto ReadCSVFile(const std::filesystem::path& path)
	{

		std::ifstream in(path.c_str());

		if (!in.is_open()) throw std::runtime_error("Could not open file");

        // Helper vars
        std::string line, colname;
        int val;
        // Create a vector of <string, int vector> pairs to store the result
        std::vector<std::pair<std::string, std::vector<int>>> result;

        // Read the column names
        if (in.good())
        {
            // Extract the first line in the file
            std::getline(in, line);

            // Create a stringstream from line
            std::stringstream ss(line);

            // Extract each column name
            while (std::getline(ss, colname, ',')) {

                // Initialize and add <colname, int vector> pairs to result
                result.push_back({ colname, std::vector<int> {} });
            }
        }
        // Read data, line by line
        while (std::getline(in, line))
        {
            // Create a stringstream of the current line
            std::stringstream ss(line);

            // Keep track of the current column index
            int colIdx = 0;

            // Extract each integer
            while (ss >> val) {

                // Add the current integer to the 'colIdx' column's values vector
                result.at(colIdx).second.push_back(val);

                // If the next token is a comma, ignore it and move on
                if (ss.peek() == ',') ss.ignore();

                // Increment the column index
                colIdx++;
            }
        }

        // Close file
        in.close();

        return result;
	}
}