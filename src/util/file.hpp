#ifndef SERVER_UTIL_FILE_HPP_
#define SERVER_UTIL_FILE_HPP_

#include "log.hpp"
#include <boost/beast.hpp>
#include <string>
namespace beast = boost::beast;

namespace util {

inline void
LoadFile(const std::string& path, std::string& out)
{
    log::Debug("LoadFile", "Loading \"{}\"", path);
    beast::file file;
    beast::error_code ec;
    file.open(path.c_str(), beast::file_mode::read, ec);
    // TODO: make this DRY
    if (ec) {
        out.clear();
        return log::Error("LoadFile", "Failed to load \"{}\": {}", path, ec.message());
    }
    auto size = file.size(ec);
    if (ec) {
        out.clear();
        return log::Error("LoadFile", "Failed to load \"{}\": {}", path, ec.message());
    }
    out.resize(size);
    file.read(out.data(), size, ec);
    if (ec) {
        out.clear();
        return log::Error("LoadFile", "Failed to load \"{}\": {}", path, ec.message());
    }
}

} // namespace util

#endif // SERVER_UTIL_FILE_HPP_