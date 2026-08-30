#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <ostream>
#include <string>
#include <string_view>

#include <spdlog/fmt/ostr.h>

namespace Cube {

// A platform independent path stored as an UTF-8 encoded string using forward slashes.
//
// The stored string is always kept normalized:
//   - '\\' is converted to '/'
//   - redundant separators are collapsed
//   - '.' components are dropped, '..' components are resolved lexically
//   - trailing separators are dropped, a root such as "/" or "C:/" keeps its own
//
// NOTE: the file system is never accessed, every operation below is purely lexical.
// NOTE: an empty path is valid and is simply stored as an empty string.
// NOTE: the string views handed out below are invalidated by any non const operation.
class Path {
public:
    // Separator used by the stored string, '\\' is only accepted as an input separator.
    static constexpr char separator = '/';

    Path() = default;
    Path(const Path&) = default;
    Path(Path&&) noexcept = default;
    Path& operator=(const Path&) = default;
    Path& operator=(Path&&) noexcept = default;
    ~Path() = default;

    explicit Path(std::string_view source) : path(normalize(source)) {}
    explicit Path(const char* source) : Path(source ? std::string_view(source) : std::string_view()) {}
    explicit Path(const std::string& source) : Path(std::string_view(source)) {}

    // The normalized, UTF-8 encoded content of this path.
    const std::string& string() const { return path; }
    const char* c_str() const { return path.c_str(); }
    bool empty() const { return path.empty(); }

    // "C:" for "C:/dir/file.txt", empty otherwise, a UNC server name is not detected.
    std::string_view rootName() const;
    // "/" for "/dir/file.txt", "//" for "//server/share", empty for a relative path.
    std::string_view rootDirectory() const;
    Path rootPath() const;
    // Everything standing after the root.
    Path relativePath() const;

    bool hasRootName() const { return !rootName().empty(); }
    bool hasRootDirectory() const { return !rootDirectory().empty(); }
    bool hasRootPath() const { return hasRootName() || hasRootDirectory(); }
    bool isAbsolute() const { return hasRootDirectory(); }
    bool isRelative() const { return !isAbsolute(); }

    // "dir/file.txt" -> "dir", a path without a parent gives back itself.
    Path parentPath() const;
    bool hasParentPath() const { return hasRootPath() || relativePathView() != filename(); }
    // "dir/file.txt" -> "file.txt"
    std::string_view filename() const;
    // "dir/file.tar.gz" -> "file.tar"
    std::string_view stem() const;
    // "dir/file.tar.gz" -> ".gz"
    std::string_view extension() const;

    bool hasFilename() const { return !filename().empty(); }
    bool hasExtension() const { return !extension().empty(); }

    void clear() { path.clear(); }

    Path& removeFilename();
    Path& replaceFilename(std::string_view filename);
    // An empty extension removes the current one, a leading '.' is optional.
    Path& replaceExtension(std::string_view extension = {});

    // Joins with '/', an absolute 'source' replaces this path.
    Path& append(std::string_view source);
    // Appends the raw text of 'source', the result stays normalized.
    Path& concat(std::string_view source);

    // Expresses this path relatively to 'base', gives back an empty path when the two
    // paths have incompatible roots.
    Path lexicallyRelative(const Path& base) const;

    void swap(Path& other) noexcept { path.swap(other.path); }

    bool operator==(const Path& other) const = default;
    std::strong_ordering operator<=>(const Path& other) const = default;

    Path& operator/=(const Path& other);
    Path& operator/=(std::string_view source) { return append(source); }
    Path& operator+=(std::string_view source) { return concat(source); }

    friend Path operator/(const Path& lhs, const Path& rhs) {
        Path result(lhs);
        result /= rhs;
        return result;
    }

    friend Path operator/(const Path& lhs, std::string_view rhs) {
        Path result(lhs);
        result /= rhs;
        return result;
    }

    friend Path operator+(const Path& lhs, std::string_view rhs) {
        Path result(lhs);
        result += rhs;
        return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const Path& path);

    friend void swap(Path& lhs, Path& rhs) noexcept { lhs.swap(rhs); }

private:
    // Turns any input path into the normalized form described above.
    static std::string normalize(std::string_view path);

    std::size_t rootSize() const;
    std::string_view relativePathView() const;

    // Shared implementation of append(), 'other' is already normalized.
    Path& appendNormalized(const Path& other);

    std::string path;
};

}  // namespace Cube

namespace std {

template <>
struct hash<Cube::Path> {
    size_t operator()(const Cube::Path& path) const noexcept {
        return hash<string>()(path.string());
    }
};

}  // namespace std

// spdlog uses the bundled fmt, which no longer falls back to operator<< on its own,
// so a Path cannot be passed to a CB_* logger without this opt-in.
namespace fmt {

template <>
struct formatter<Cube::Path, char> : ostream_formatter {};

}  // namespace fmt
