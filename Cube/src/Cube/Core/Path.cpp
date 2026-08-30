#include "Cube/Core/Path.h"

#include <ostream>
#include <vector>

namespace Cube {

namespace {

constexpr char kSeparator = '/';
constexpr char kAlternateSeparator = '\\';
constexpr std::string_view kCurrentComponent = ".";
constexpr std::string_view kParentComponent = "..";

bool isSeparator(char c) {
    return c == kSeparator || c == kAlternateSeparator;
}

bool isDriveLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Length of a windows drive prefix such as "C:", zero when there is none.
std::size_t drivePrefixLength(std::string_view path) {
    if (path.size() >= 2 && isDriveLetter(path[0]) && path[1] == ':') return 2;
    return 0;
}

// Length of the root prefix of a raw path, 2 for "//" or "C:", 1 for "/" and 3 for "C:/".
std::size_t rootPrefixLength(std::string_view path) {
    if (path.empty()) return 0;
    if (isSeparator(path[0])) {
        if (path.size() >= 2 && isSeparator(path[1])) return 2;
        return 1;
    }
    const std::size_t drive = drivePrefixLength(path);
    if (drive == 0) return 0;
    if (path.size() > drive && isSeparator(path[drive])) return drive + 1;
    return drive;
}

// Appends 'component' to 'target', 'root' is the size of the root prefix of 'target'.
void appendComponent(std::string& target, std::size_t root, std::string_view component) {
    if (target.size() > root) target += kSeparator;
    target.append(component);
}

// The last component of 'target', or an empty view when only the root is left.
std::string_view lastComponent(const std::string& target, std::size_t root) {
    if (target.size() <= root) return {};
    const std::size_t pos = target.rfind(kSeparator);
    if (pos == std::string::npos || pos < root) return std::string_view(target).substr(root);
    return std::string_view(target).substr(pos + 1);
}

// Drops the last component of 'target', never removes the root prefix itself.
void popComponent(std::string& target, std::size_t root) {
    const std::size_t pos = target.rfind(kSeparator);
    if (pos == std::string::npos || pos < root) target.erase(root);
    else target.erase(pos);
}

// Splits the components of an already normalized path.
std::vector<std::string_view> splitComponents(std::string_view path) {
    std::vector<std::string_view> components;
    std::size_t pos = 0;
    while (pos < path.size()) {
        while (pos < path.size() && path[pos] == kSeparator) ++pos;
        const std::size_t begin = pos;
        while (pos < path.size() && path[pos] != kSeparator) ++pos;
        if (begin < pos) components.push_back(path.substr(begin, pos - begin));
    }
    return components;
}

}  // namespace

std::string Path::normalize(std::string_view source) {
    const std::size_t root = rootPrefixLength(source);

    std::string result;
    result.reserve(source.size());
    for (std::size_t i = 0; i < root; ++i) {
        result += isSeparator(source[i]) ? kSeparator : source[i];
    }

    // NOTE: a leading "//" is kept as it is, it is a valid root on both windows and posix.
    std::size_t pos = root;
    while (pos < source.size()) {
        while (pos < source.size() && isSeparator(source[pos])) ++pos;
        const std::size_t begin = pos;
        while (pos < source.size() && !isSeparator(source[pos])) ++pos;
        if (begin == pos) break;  // trailing separators

        const std::string_view component = source.substr(begin, pos - begin);
        if (component == kCurrentComponent) continue;
        if (component == kParentComponent) {
            // ".." cannot pop another "..", nor the root of an absolute path.
            if (result.size() > root && lastComponent(result, root) != kParentComponent) {
                popComponent(result, root);
            }
            else if (root == 0) {
                appendComponent(result, root, component);
            }
            continue;
        }
        appendComponent(result, root, component);
    }

    // Everything got resolved away, only a relative path can collapse to the current directory.
    if (result.empty() && !source.empty()) return std::string(kCurrentComponent);
    return result;
}

std::string_view Path::rootName() const {
    const std::size_t drive = drivePrefixLength(path);
    if (drive == 0) return {};
    return std::string_view(path).substr(0, drive);
}

std::string_view Path::rootDirectory() const {
    const std::size_t name = rootName().size();
    if (path.size() <= name || path[name] != separator) return {};
    if (path.size() > name + 1 && path[name + 1] == separator) return std::string_view(path).substr(name, 2);
    return std::string_view(path).substr(name, 1);
}

std::size_t Path::rootSize() const {
    return rootName().size() + rootDirectory().size();
}

std::string_view Path::relativePathView() const {
    return std::string_view(path).substr(rootSize());
}

Path Path::rootPath() const {
    return Path(std::string_view(path).substr(0, rootSize()));
}

Path Path::relativePath() const {
    return Path(relativePathView());
}

Path Path::parentPath() const {
    const std::string_view relative = relativePathView();
    if (relative.empty()) return *this;  // empty path, or a path made of nothing but a root

    const std::size_t root = path.size() - relative.size();
    const std::size_t pos = relative.rfind(separator);
    if (pos == std::string_view::npos) return Path(std::string_view(path).substr(0, root));
    return Path(std::string_view(path).substr(0, root + pos));
}

std::string_view Path::filename() const {
    const std::string_view relative = relativePathView();
    const std::size_t pos = relative.rfind(separator);
    if (pos == std::string_view::npos) return relative;
    return relative.substr(pos + 1);
}

std::string_view Path::stem() const {
    const std::string_view name = filename();
    if (name == kCurrentComponent || name == kParentComponent) return name;

    const std::size_t pos = name.rfind('.');
    if (pos == std::string_view::npos || pos == 0) return name;  // a leading dot starts a name
    return name.substr(0, pos);
}

std::string_view Path::extension() const {
    const std::string_view name = filename();
    if (name == kCurrentComponent || name == kParentComponent) return {};

    const std::size_t pos = name.rfind('.');
    if (pos == std::string_view::npos || pos == 0) return {};
    return name.substr(pos);
}

Path& Path::removeFilename() {
    *this = parentPath();
    return *this;
}

Path& Path::replaceFilename(std::string_view filename) {
    return removeFilename().append(filename);
}

Path& Path::replaceExtension(std::string_view extension) {
    const std::string_view current = this->extension();
    if (!current.empty()) path.erase(path.size() - current.size());

    if (!extension.empty()) {
        if (extension.front() != '.') path += '.';
        path.append(extension);
    }
    return *this;
}

Path& Path::append(std::string_view source) {
    return appendNormalized(Path(source));
}

Path& Path::concat(std::string_view source) {
    if (source.empty()) return *this;
    path.append(source);
    path = normalize(path);
    return *this;
}

Path& Path::appendNormalized(const Path& other) {
    if (other.empty()) return *this;

    // An absolute path, or a path rooted on another drive, replaces this one.
    if (other.isAbsolute() || (other.hasRootName() && other.rootName() != rootName())) {
        *this = other;
        return *this;
    }

    std::string joined = path;
    const bool driveRelative = hasRootName() && !hasRootDirectory();
    if (!joined.empty() && joined.back() != separator && !driveRelative) joined += separator;
    joined.append(other.relativePathView());
    path = normalize(joined);
    return *this;
}

Path Path::lexicallyRelative(const Path& base) const {
    if (isAbsolute() != base.isAbsolute() || rootName() != base.rootName()) return Path();

    const std::vector<std::string_view> from = splitComponents(base.relativePathView());
    const std::vector<std::string_view> to = splitComponents(relativePathView());

    std::size_t common = 0;
    while (common < from.size() && common < to.size() && from[common] == to[common]) ++common;

    std::string result;
    for (std::size_t i = common; i < from.size(); ++i) appendComponent(result, 0, kParentComponent);
    for (std::size_t i = common; i < to.size(); ++i) appendComponent(result, 0, to[i]);

    if (result.empty()) return Path(kCurrentComponent);
    return Path(result);
}

Path& Path::operator/=(const Path& other) {
    return appendNormalized(other);
}

std::ostream& operator<<(std::ostream& os, const Path& path) {
    return os << path.string();
}

}  // namespace Cube
