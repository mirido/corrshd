#pragma once

/// Parse fine name to directory, filename, and extension.
void parse_file_name(const char* const fpath, std::string& dir, std::string& fnameMajor, std::string& ext);

/// Get command name from argv[0].
std::string get_prog_name(const char* const argv0);
