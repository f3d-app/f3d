void DxfFile::readFile(const std::string& filePath) {
    // ...
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::string line;
    std::getline(file, line); // Read the first line (0)
    if (line != "0") {
        // Optional header, skip it if it's not present
        while (std::getline(file, line) && line != "0") {
            if (line == "SECTION") {
                break;
            }
        }
    }

    // Continue reading the file as before
    // ...
}