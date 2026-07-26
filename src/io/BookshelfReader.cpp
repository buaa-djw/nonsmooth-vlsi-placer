#include "placer/io/BookshelfReader.hpp"
#include "placer/io/TextInput.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;
namespace placer
{
namespace
{
std::string lower(std::string value) { for (char &c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return value; }
bool ends(const std::string &value, const std::string &suffix) { const auto v = lower(value); return v.size() >= suffix.size() && v.compare(v.size() - suffix.size(), suffix.size(), suffix) == 0; }
[[noreturn]] void fail(const std::string &path, std::size_t line, const std::string &message, const std::string &raw = {})
{
    throw std::runtime_error(path + ":" + std::to_string(line) + ": " + message + (raw.empty() ? "" : "; line='" + raw + "'"));
}
double number(const std::string &token, const std::string &path, std::size_t line, const std::string &field, const std::string &raw)
{
    std::size_t used = 0;
    double value = 0.0;
    try { value = std::stod(token, &used); }
    catch (const std::exception &) { fail(path, line, "invalid " + field + ": '" + token + "'", raw); }
    if (used != token.size() || !std::isfinite(value)) fail(path, line, "invalid " + field + ": '" + token + "'", raw);
    return value;
}
std::size_t countValue(const std::string &token, const std::string &path, std::size_t line, const std::string &field, const std::string &raw)
{
    if (token.empty() || token.front() == '-') fail(path, line, "invalid " + field + ": '" + token + "'", raw);
    std::size_t used = 0;
    unsigned long long value = 0;
    try { value = std::stoull(token, &used); }
    catch (const std::exception &) { fail(path, line, "invalid " + field + ": '" + token + "'", raw); }
    if (used != token.size() || value > std::numeric_limits<std::size_t>::max()) fail(path, line, "invalid " + field + ": '" + token + "'", raw);
    return static_cast<std::size_t>(value);
}
std::size_t afterColon(const std::vector<std::string> &items, const std::string &path, std::size_t line, const std::string &field, const std::string &raw)
{
    const auto it = std::find(items.begin(), items.end(), ":");
    if (it == items.end() || std::next(it) == items.end()) fail(path, line, "missing value for " + field, raw);
    return countValue(*std::next(it), path, line, field, raw);
}
std::vector<std::pair<std::size_t, std::string>> lines(const std::string &path)
{
    std::istringstream input(readTextFile(path));
    std::vector<std::pair<std::size_t, std::string>> result;
    std::string line;
    for (std::size_t number = 1; std::getline(input, line); ++number) result.emplace_back(number, line);
    return result;
}
std::vector<std::string> auxTokens(const std::string &text, const std::string &path)
{
    std::vector<std::string> result;
    std::string current;
    bool quoted = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (!quoted && c == '#') { while (i < text.size() && text[i] != '\n') ++i; continue; }
        if (c == '"') { quoted = !quoted; if (!quoted) { result.push_back(current); current.clear(); } continue; }
        if (quoted) { current.push_back(c); continue; }
        if (std::isspace(static_cast<unsigned char>(c))) { if (!current.empty()) { result.push_back(current); current.clear(); } }
        else current.push_back(c);
    }
    if (quoted) fail(path, 1, "unterminated quoted AUX path");
    if (!current.empty()) result.push_back(current);
    return result;
}
std::string resolve(const fs::path &aux, std::string token)
{
    for (char &c : token) if (c == '\\') c = '/';
    const fs::path candidate = token.empty() ? fs::path{} : (fs::path(token).is_absolute() ? fs::path(token) : aux.parent_path() / token);
    return fs::absolute(candidate).lexically_normal().string();
}
void requireCount(const std::optional<std::size_t> &declared, std::size_t actual, const std::string &path, const std::string &field)
{
    if (declared && *declared != actual) fail(path, 0, field + " mismatch: declared " + std::to_string(*declared) + ", parsed " + std::to_string(actual));
}
}

std::map<std::string, std::string> parseAux(const std::string &aux)
{
    std::map<std::string, std::string> result;
    for (const auto &token : auxTokens(readTextFile(aux), aux)) {
        for (const std::string type : {".nodes", ".nets", ".pl", ".scl"}) {
            if (!ends(token, type) && !ends(token, type + ".gz")) continue;
            const auto path = resolve(fs::path(aux), token);
            const auto found = result.find(type);
            if (found != result.end() && found->second != path) fail(aux, 1, "conflicting " + type + " references: '" + found->second + "' and '" + path + "'");
            result[type] = path;
        }
    }
    for (const std::string type : {".nodes", ".nets", ".pl", ".scl"}) {
        const auto found = result.find(type);
        if (found == result.end()) fail(aux, 1, "missing " + type + " reference");
        if (!fs::exists(found->second)) fail(aux, 1, type + " file not found: '" + found->second + "'");
    }
    return result;
}

void parseNodes(const std::string &path, PlacementDB &db)
{
    std::optional<std::size_t> nodes, terminals;
    std::size_t actual_terminals = 0;
    for (const auto &[line_no, raw] : lines(path)) {
        const auto item = tokens(raw);
        if (item.empty() || item[0] == "UCLA") continue;
        if (item[0] == "NumNodes") { if (nodes) fail(path, line_no, "duplicate NumNodes", raw); nodes = afterColon(item, path, line_no, "NumNodes", raw); continue; }
        if (item[0] == "NumTerminals") { if (terminals) fail(path, line_no, "duplicate NumTerminals", raw); terminals = afterColon(item, path, line_no, "NumTerminals", raw); continue; }
        if (item.size() < 3) fail(path, line_no, "malformed node record", raw);
        const double width = number(item[1], path, line_no, "width for node '" + item[0] + "'", raw);
        const double height = number(item[2], path, line_no, "height for node '" + item[0] + "'", raw);
        if (width <= 0.0 || height <= 0.0) fail(path, line_no, "non-positive dimensions for node '" + item[0] + "'", raw);
        bool terminal = false;
        for (std::size_t i = 3; i < item.size(); ++i) {
            const auto marker = lower(item[i]);
            if (marker == "terminal" || marker == "terminal_ni") terminal = true;
            else fail(path, line_no, "unknown node marker '" + item[i] + "' for '" + item[0] + "'", raw);
        }
        try { db.addCell(item[0], width, height, terminal); }
        catch (const std::runtime_error &error) { fail(path, line_no, error.what(), raw); }
        actual_terminals += terminal ? 1U : 0U;
    }
    requireCount(nodes, db.cells.size(), path, "NumNodes");
    requireCount(terminals, actual_terminals, path, "NumTerminals");
    db.declared_counts.num_nodes = nodes;
    db.declared_counts.num_terminals = terminals;
}

void parsePl(const std::string &path, PlacementDB &db)
{
    std::set<CellId> seen;
    for (const auto &[line_no, raw] : lines(path)) {
        const auto item = tokens(raw);
        if (item.empty() || item[0] == "UCLA") continue;
        if (item.size() < 3) fail(path, line_no, "malformed PL record", raw);
        const auto found = db.cell_name_to_id.find(item[0]);
        if (found == db.cell_name_to_id.end()) fail(path, line_no, "unknown PL cell '" + item[0] + "'", raw);
        if (!seen.insert(found->second).second) fail(path, line_no, "duplicate PL record for cell '" + item[0] + "'", raw);
        auto &cell = db.cells[found->second];
        cell.x = number(item[1], path, line_no, "x coordinate for '" + item[0] + "'", raw);
        cell.y = number(item[2], path, line_no, "y coordinate for '" + item[0] + "'", raw);
        bool fixed = cell.terminal;
        for (std::size_t i = 3; i < item.size(); ++i) {
            if (item[i] == ":") { if (i + 1 >= item.size() || item[i + 1].empty() || item[i + 1][0] == '/') fail(path, line_no, "missing orientation for '" + item[0] + "'", raw); cell.orientation = item[++i]; }
            else if (lower(item[i]) == "/fixed" || lower(item[i]) == "/fixed_ni") fixed = true;
            else fail(path, line_no, "unknown PL token '" + item[i] + "' for '" + item[0] + "'", raw);
        }
        cell.fixed = fixed;
    }
    if (seen.size() != db.cells.size()) fail(path, 0, "PL record count mismatch: expected " + std::to_string(db.cells.size()) + ", parsed " + std::to_string(seen.size()));
}

void parseScl(const std::string &path, PlacementDB &db)
{
    std::optional<std::size_t> declared_rows;
    std::size_t core_rows = 0;
    bool in_row = false;
    std::size_t start_line = 0;
    std::optional<double> coordinate, height, site_width, site_spacing;
    std::optional<std::string> site_orient, site_symmetry;
    std::vector<std::pair<double, std::size_t>> subrows;
    const auto finish = [&](std::size_t line_no, const std::string &raw) {
        if (!in_row) fail(path, line_no, "End outside CoreRow", raw);
        if (!coordinate || !height || !site_width || !site_spacing || !site_orient || !site_symmetry || subrows.empty()) fail(path, start_line, "CoreRow missing required field or subrow");
        if (*height <= 0.0 || *site_width <= 0.0 || *site_spacing <= 0.0) fail(path, start_line, "CoreRow has non-positive geometry");
        for (const auto &[origin, sites] : subrows) {
            if (sites == 0 || sites > static_cast<std::size_t>(std::numeric_limits<int>::max())) fail(path, start_line, "invalid NumSites");
            db.rows.push_back({*coordinate, *height, origin, origin + static_cast<double>(sites) * *site_spacing, *site_width, *site_spacing, static_cast<int>(sites)});
        }
    };
    for (const auto &[line_no, raw] : lines(path)) {
        const auto item = tokens(raw);
        if (item.empty() || item[0] == "UCLA") continue;
        if (item[0] == "NumRows") { if (declared_rows) fail(path, line_no, "duplicate NumRows", raw); declared_rows = afterColon(item, path, line_no, "NumRows", raw); continue; }
        if (item[0] == "CoreRow") { if (in_row) fail(path, line_no, "CoreRow before previous End", raw); if (item.size() < 2) fail(path, line_no, "missing CoreRow orientation", raw); in_row = true; start_line = line_no; ++core_rows; coordinate.reset(); height.reset(); site_width.reset(); site_spacing.reset(); site_orient.reset(); site_symmetry.reset(); subrows.clear(); continue; }
        if (item[0] == "End") { finish(line_no, raw); in_row = false; continue; }
        if (!in_row) fail(path, line_no, "unexpected SCL record", raw);
        auto valueAfter = [&](std::size_t i, const std::string &field) -> const std::string & { std::size_t j = i + 1; if (j < item.size() && item[j] == ":") ++j; if (j >= item.size()) fail(path, line_no, "missing " + field, raw); return item[j]; };
        if (item[0] == "Coordinate") coordinate = number(valueAfter(0, "Coordinate"), path, line_no, "Coordinate", raw);
        else if (item[0] == "Height") height = number(valueAfter(0, "Height"), path, line_no, "Height", raw);
        else if (item[0] == "Sitewidth") site_width = number(valueAfter(0, "Sitewidth"), path, line_no, "Sitewidth", raw);
        else if (item[0] == "Sitespacing") site_spacing = number(valueAfter(0, "Sitespacing"), path, line_no, "Sitespacing", raw);
        else if (item[0] == "Siteorient") site_orient = valueAfter(0, "Siteorient");
        else if (item[0] == "Sitesymmetry") site_symmetry = valueAfter(0, "Sitesymmetry");
        else if (item[0] != "SubrowOrigin") fail(path, line_no, "unknown SCL field '" + item[0] + "'", raw);
        for (std::size_t i = 0; i < item.size(); ++i) if (item[i] == "SubrowOrigin") {
            const double origin = number(valueAfter(i, "SubrowOrigin"), path, line_no, "SubrowOrigin", raw);
            auto net = std::find(item.begin() + static_cast<std::ptrdiff_t>(i + 1), item.end(), "NumSites");
            if (net == item.end()) fail(path, line_no, "SubrowOrigin missing NumSites", raw);
            const auto ni = static_cast<std::size_t>(std::distance(item.begin(), net));
            subrows.emplace_back(origin, countValue(valueAfter(ni, "NumSites"), path, line_no, "NumSites", raw));
            i = ni;
        }
    }
    if (in_row) fail(path, start_line, "CoreRow missing End");
    requireCount(declared_rows, core_rows, path, "NumRows");
    db.declared_counts.num_rows = declared_rows;
}

void parseNets(const std::string &path, PlacementDB &db)
{
    std::optional<std::size_t> declared_nets, declared_pins;
    std::set<std::string> names;
    const auto input = lines(path);
    for (std::size_t pos = 0; pos < input.size(); ++pos) {
        const auto &[line_no, raw] = input[pos];
        const auto item = tokens(raw);
        if (item.empty() || item[0] == "UCLA") continue;
        if (item[0] == "NumNets") { if (declared_nets) fail(path, line_no, "duplicate NumNets", raw); declared_nets = afterColon(item, path, line_no, "NumNets", raw); continue; }
        if (item[0] == "NumPins") { if (declared_pins) fail(path, line_no, "duplicate NumPins", raw); declared_pins = afterColon(item, path, line_no, "NumPins", raw); continue; }
        if (item[0] != "NetDegree") fail(path, line_no, "unexpected NETS record", raw);
        const auto colon = std::find(item.begin(), item.end(), ":");
        if (colon == item.end() || std::next(colon) == item.end()) fail(path, line_no, "malformed NetDegree", raw);
        const auto degree = countValue(*std::next(colon), path, line_no, "NetDegree", raw);
        const auto degree_index = static_cast<std::size_t>(std::distance(item.begin(), std::next(colon)));
        const std::string name = degree_index + 1 < item.size() ? item[degree_index + 1] : "net_" + std::to_string(db.nets.size());
        if (!names.insert(name).second) fail(path, line_no, "duplicate net name '" + name + "'", raw);
        const auto net_id = db.addNet(name);
        std::size_t parsed = 0;
        while (parsed < degree) {
            if (++pos >= input.size()) fail(path, line_no, "premature EOF in net '" + name + "'");
            const auto &[pin_line, pin_raw] = input[pos];
            const auto pin = tokens(pin_raw);
            if (pin.empty()) continue;
            if (pin[0] == "NetDegree" || pin[0] == "NumNets" || pin[0] == "NumPins" || pin[0] == "UCLA") fail(path, pin_line, "NetDegree mismatch for net '" + name + "': expected " + std::to_string(degree) + ", parsed " + std::to_string(parsed), pin_raw);
            if (pin.size() < 5) fail(path, pin_line, "malformed pin in net '" + name + "'", pin_raw);
            const auto cell = db.cell_name_to_id.find(pin[0]);
            if (cell == db.cell_name_to_id.end()) fail(path, pin_line, "unknown pin cell '" + pin[0] + "' in net '" + name + "'", pin_raw);
            const auto pin_colon = std::find(pin.begin(), pin.end(), ":");
            if (pin_colon == pin.end() || std::distance(pin_colon, pin.end()) < 3) fail(path, pin_line, "missing pin offsets in net '" + name + "'", pin_raw);
            const auto ox = number(*std::next(pin_colon), path, pin_line, "pin x offset", pin_raw);
            const auto oy = number(*std::next(pin_colon, 2), path, pin_line, "pin y offset", pin_raw);
            db.addPin(cell->second, net_id, ox, oy, pin.size() > 1 ? pin[1] : "");
            ++parsed;
        }
    }
    requireCount(declared_nets, db.nets.size(), path, "NumNets");
    requireCount(declared_pins, db.pins.size(), path, "NumPins");
    db.declared_counts.num_nets = declared_nets;
    db.declared_counts.num_pins = declared_pins;
}

PlacementDB loadBookshelf(const std::string &aux)
{
    const auto files = parseAux(aux);
    PlacementDB db;
    parseNodes(files.at(".nodes"), db);
    parsePl(files.at(".pl"), db);
    parseScl(files.at(".scl"), db);
    parseNets(files.at(".nets"), db);
    db.validate();
    return db;
}
}
