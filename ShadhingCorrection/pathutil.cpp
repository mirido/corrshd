#include "stdafx.h"
#include "pathutil.h"

/// Parse fine name to directory, filename, and extension.
void parse_file_name(const char* const fpath, std::string& dir, std::string& fnameMajor, std::string& ext)
{
	const size_t len = strlen(fpath);

	// �t�@�C�����̐擪�ʒu�擾
	size_t k1 = len;
	while (k1 > 0 && strchr("\\/", fpath[k1 - 1]) == NULL) {
		k1--;
	}

	// �g���q�̐擪�ʒu�擾
	size_t k2 = len;
	while (k2 > k1 && fpath[k2 - 1] != '.') {
		k2--;
	}
	if (k2 <= k1) {
		// (�t�@�C���������� '.' ���� = �g���q����)
		k2 = len;
	}

	dir = fpath;
	dir = (k1 > 0) ? dir.substr(0, k1) : "";

	fnameMajor = &(fpath[k1]);
	fnameMajor = (k2 - k1 > 0) ? fnameMajor.substr(0, k2 - k1) : "";

	ext = &(fpath[k2]);

	if (fnameMajor.length() > 0 && fnameMajor.back() == '.') {
		fnameMajor.pop_back();
		ext = std::string(".") + ext;
	}
}

/// Get command name from argv[0].
std::string get_prog_name(const char* const argv0)
{
	std::string dir, fileMajor, ext;
	parse_file_name(argv0, dir, fileMajor, ext);
	return fileMajor;
}
