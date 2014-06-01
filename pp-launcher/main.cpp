#include <Windows.h>
#include <stdio.h>
#include <sstream>
#include <memory>
#include <filesystem>
#include <vector>

namespace fs = std::tr2::sys;

#pragma comment(lib, "Version.lib")

struct LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
};

class SubBlock
{
	LPVOID block;
	std::string prefix;
	static std::string make_prefix(const LANGANDCODEPAGE& translation)
	{
		char prefix[100];
		sprintf_s(prefix, "\\StringFileInfo\\%04x%04x\\", translation.wLanguage, translation.wCodePage);
		return prefix;
	}
public:
	SubBlock() : block(nullptr) {}

	SubBlock(LPVOID block, const LANGANDCODEPAGE& translation)
		: block(block)
		, prefix(make_prefix(translation))
	{

	}

	explicit operator bool() const { return !!block; }

	std::string query(const char* name)
	{
		if (!block)
			return{};

		LPVOID buffer;
		UINT size = 0;
		if (VerQueryValueA(block, (prefix + name).c_str(), &buffer, &size))
			return (char*)buffer;

		return{};
	}
};

class VersionInfo
{
	std::unique_ptr<char[]> block;
	LANGANDCODEPAGE *translations;
	UINT translation_count;
public:
	explicit VersionInfo(const std::string& path)
		: translations(nullptr)
		, translation_count(0)
	{
		auto size = GetFileVersionInfoSizeA(path.c_str(), nullptr);
		if (!size)
		{
			auto err = GetLastError();
			if (err == ERROR_RESOURCE_TYPE_NOT_FOUND ||
				err == ERROR_RESOURCE_NAME_NOT_FOUND ||
				err == ERROR_RESOURCE_LANG_NOT_FOUND)
			{
				printf("No VERSIONINFO (%s).\n", path.c_str());
			}
			return;
		}

		block.reset( new (std::nothrow) char[size] );
		if (!block)
		{
			printf("OOM while getting VERSIONINFO.\n");
			return;
		}

		if (!GetFileVersionInfoA(path.c_str(), 0, size, block.get()))
		{
			printf("GetFileVersionInfo error (%d).\n", GetLastError());
			block.reset();
			return;
		}

		if (!VerQueryValueA(block.get(), "\\VarFileInfo\\Translation", (LPVOID*)&translations, &translation_count) ||
			translation_count < sizeof(LANGANDCODEPAGE))
		{
			printf("No \\VarFileInfo\\Translation in VERSIONINFO.\n");
			block.reset();
			return;
		}

		translation_count /= sizeof(LANGANDCODEPAGE);
	}

	explicit operator bool() const { return !!block; }

	SubBlock operator[](UINT i)
	{
		if (i >= translation_count)
			return{};

		return{ block.get(), translations[i] };
	}
};

std::string PluginInfo(const std::string& path)
{
	VersionInfo info{ path };
	if (!info)
		return std::string();

	auto block = info[0];
	if (!block)
		return std::string();

	auto ProductName = block.query("ProductName");
	auto FileDescription = block.query("FileDescription");
	auto FileVersion = block.query("FileVersion");
	auto MIMEType = block.query("MIMEType");

	if (MIMEType.empty())
		return std::string();

	// plugin-entry =
	//    <file-path> +
	//    ["#" + <name> + ["#" + <description> + ["#" + <version>]]] +
	//    *1( LWS + ";" + LWS + <mime-type> )

	printf("Adding %s (%s)\n", ProductName.c_str(), MIMEType.c_str());
	return path + "#" + ProductName + "#" + FileDescription + "#" + FileVersion + ";" + MIMEType;
}

fs::path canonical(const fs::path& path)
{
	std::vector<fs::path> segments;
	for (auto&& seg : fs::complete(path))
	{
		if (seg == ".") continue;
		if (seg == ".." && segments.size() > 1)
		{
			segments.pop_back();
			continue;
		}
		segments.push_back(seg);
	}

	auto out = segments.front();
	segments.erase(segments.begin());
	for (auto&& seg : segments)
		out /= seg;

	return out;
}

std::string PluginInfos(const std::string& dir = ".")
{
	std::string out;
	bool first = true;

	fs::directory_iterator cur{ canonical(dir) }, end{};
	for (; cur != end; ++cur)
	{
		auto&& entry = *cur;
		auto info = PluginInfo(entry.path().file_string());
		if (info.empty())
			continue;

		if (first) first = false;
		else out.push_back(',');
		out += info;
	}

	return out;
}

bool needs_escaping(const std::string& arg)
{
	for (auto&& c : arg)
	{
		if (c == ' ' || c == '"')
			return true;
	}

	return false;
}

std::string do_escape(const std::string& arg)
{
	std::string out;
	out.reserve(arg.length() + 2); // +10%
	out.push_back('"');
	out.append(arg);
	out.push_back('"');

	return out;
}

std::string escape(const std::string& arg)
{
	if (needs_escaping(arg))
		return do_escape(arg);
	return arg;
}

std::string module_dir()
{
	char buffer[2048];
	GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
	auto slash = strrchr(buffer, '\\');
	if (slash)
		slash[1] = 0;

	return buffer;
}

std::string lastError(DWORD dw)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	auto size = WideCharToMultiByte(CP_OEMCP, 0, (LPTSTR)lpMsgBuf, -1, nullptr, 0, nullptr, nullptr);
	if (size < 1)
	{
		LocalFree(lpMsgBuf);
		return "?";
	}

	auto msg_buf = (char*)malloc(size + 1);
	if (!msg_buf)
	{
		LocalFree(lpMsgBuf);
		return "?";
	}

	WideCharToMultiByte(CP_OEMCP, 0, (LPTSTR)lpMsgBuf, -1, msg_buf, size, nullptr, nullptr);
	std::string out{ msg_buf };

	free(msg_buf);
	LocalFree(lpMsgBuf);

	return out;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "pp-launcher <chrome.exe> [<chrome switches>]\n");
		return 1;
	}

	auto plugins = PluginInfos();// module_dir());
	if (plugins.empty())
	{
		printf("No modules with VERISONINFO and MIMEType found.\n");
		return 1;
	}

	std::ostringstream o;
	for (int i = 1; i < argc; ++i)
		o << escape(argv[i]) << ' ';
	o << escape("--register-pepper-plugins=" + plugins);
	auto cmd = o.str();

	PROCESS_INFORMATION proc = {};
	STARTUPINFOA startup = { sizeof(STARTUPINFO) };

	if (!CreateProcessA(argv[1], (char*)cmd.c_str(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startup, &proc))
	{
		DWORD dw = GetLastError();
		printf("\n%s:\nerror %d: %s", argv[1], dw, lastError(dw));
		return 1;
	}

	printf("Waiting...\n");
	WaitForSingleObject(proc.hProcess, INFINITE);
	CloseHandle(proc.hProcess);
	CloseHandle(proc.hThread);

	return 0;
}
