#include "html_generator.hpp"

std::string toLowercase(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] = static_cast<char>(tolower(result[i]));
    }
    return result;
}

bool compareDirentNamesCaseInsensitive(const dirent* a, const dirent* b) {
    std::string nameA = toLowercase(a->d_name);
    std::string nameB = toLowercase(b->d_name);

    return nameA < nameB;
}


std::string Generator::ErrorPage(int error_number, std::string error_string)
{
    std::string output = "<!DOCTYPE html>\n"
                         "<html>\n"
                         "\n"
                         "<head>\n"
                         "  <title>Error:</title>\n"
                         "  <meta charset=\"utf-8\">\n"
                         "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                         "</head>\n"
                         "\n"
                         "<body classes=\"Error_Number, Error_Text\">\n"
                         "  <h1>Error:</h1>\n"
                         "<hr>\n"
                         "  Error Number ";
    output += std::to_string(error_number);
    output += " : ";
    output += error_string;
    output += "\n"
              "\n"
              "</body>\n"
              "\n"
              "</html>";
    return output;
}
std::string Generator::DirectoryList(std::string path)
{
    struct dirent *currentFile;
    std::vector<dirent*> files;
    DIR *directory =  opendir(path.c_str());
   if (!directory)
       throw std::runtime_error("Unable to open directory");

   do{
      currentFile = readdir(directory);
      if (currentFile)
        files.push_back(currentFile);
   }
   while (currentFile);

    closedir(directory);
    std::sort(files.begin(), files.end(), compareDirentNamesCaseInsensitive);

    return arrangeOutput(path, files);
}

std::string Generator::arrangeOutput(std::string path, std::vector<dirent *> files)
{
    std::string output = "<!DOCTYPE html>\n"
                         "<html lang=\"en\">\n"
                         "<head>\n"
                         "    <meta charset=\"UTF-8\">\n"
                         "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                         "    <title>File List</title>\n"
                         "    <style>\n"
                         "        body {\n"
                         "            font-family: Arial, sans-serif;\n"
                         "        }\n"
                         "    </style>\n"
                         "</head>\n"
                         "<body>\n"
                         "\n"
                         "<h1>File List</h1>\n"
                         "\n"
                         "<ul>";
    for (size_t i = 0; files[i]; i++)
    {
     output += "<li><a href=\"";
     output += path;
     output += "/";
     output += files[i]->d_name;
     output += "\">";
     output += files[i]->d_name;
     output += "</a></li>\n";
    }
    output += "</ul>\n"
              "\n"
              "</body>\n"
              "</html>\n";

    return output;
}