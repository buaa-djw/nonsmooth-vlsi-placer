#include "placer/io/TextInput.hpp"
#include <zlib.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
namespace placer
{
    static bool ends(const std::string &s, const std::string &e) { return s.size() >= e.size() && s.substr(s.size() - e.size()) == e; }
    std::string readTextFile(const std::string &path)
    {
        if (ends(path, ".gz"))
        {
            gzFile f = gzopen(path.c_str(), "rb");
            if (!f)
                throw std::runtime_error("cannot open " + path);
            std::string out;
            char buf[8192];
            int n = 0;
            while ((n = gzread(f, buf, sizeof(buf))) > 0)
                out.append(buf, static_cast<std::size_t>(n));
            if (n < 0) { int code = Z_OK; const char *message = gzerror(f, &code); const std::string detail = message ? message : "unknown zlib error"; gzclose(f); throw std::runtime_error("error reading " + path + ": " + detail); }
            if (gzclose(f) != Z_OK) throw std::runtime_error("error closing gzip input " + path);
            return out;
        }
        std::ifstream in(path);
        if (!in)
            throw std::runtime_error("cannot open " + path);
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }
    std::vector<std::string> tokens(const std::string &line)
    {
        std::string s = line.substr(0, line.find('#'));
        std::string r;
        for (char c : s)
        {
            if (c == ':')
                r += " : ";
            else
                r.push_back(c);
        }
        std::istringstream is(r);
        std::vector<std::string> v;
        std::string x;
        while (is >> x)
            v.push_back(x);
        return v;
    }
}
