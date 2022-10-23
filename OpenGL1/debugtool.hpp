

#ifndef DEBUGTOOL_H
#ifdef _DEBUG_
#define DEBUG_TOOL_H
// static API

static std::vector<std::string> g_supportExtensions;
static void GetSupportExtensions() {
	if (!g_supportExtensions.empty())
		return;
	GLint n, i;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for (i = 0; i < n; i++) {
		std::string extension = (char*)glGetStringi(GL_EXTENSIONS, i);
		g_supportExtensions.push_back(extension);
	}
}

// public API
bool CheckExtension(const std::string& extensionName) {
	GetSupportExtensions();

	for (int i = 0; i < g_supportExtensions.size(); i++) {
		if (g_supportExtensions[i] == extensionName)
			return true;
	}
	return false;
}




#endif // DEBUG



#endif