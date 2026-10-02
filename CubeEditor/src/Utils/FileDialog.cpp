#include "FileDialog.h"

#include <shobjidl_core.h>
#include <windows.h>
#include <shobjidl.h>
#include <combaseapi.h> 
#include <winnt.h>

#include <vector>
#include <string>

#include "Cube/Utils/Utils.h"

namespace Utils {
    
namespace {

static std::string getPathFromItem(IShellItem* pItem) {
    if (!pItem) return {};
    PWSTR pszPath = nullptr;
    HRESULT hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
    if (FAILED(hr) || !pszPath) return {};
    std::string result = Cube::Utils::utf16To8((char16_t*)pszPath);
    CoTaskMemFree(pszPath);
    return result;
}

struct FilterSpecs {
    std::vector<COMDLG_FILTERSPEC> specs;
    std::vector<std::u16string> names;
    std::vector<std::u16string> specsW;
};

static FilterSpecs buildFilterSpecs(const std::vector<FileDialog::FilterSpec>& filters) {
    FilterSpecs result;
    if (filters.empty()) {
        result.specs.push_back({ L"All Files", L"*.*" });
        return result;
    } else {
        for (const auto& f : filters) {
            result.names.push_back(Cube::Utils::utf8To16(f.name));
            result.specsW.push_back(Cube::Utils::utf8To16(f.spec));
        }
    }
    for (size_t i = 0; i < result.names.size(); ++i) {
        COMDLG_FILTERSPEC spec;
        spec.pszName = (wchar_t*)(result.names[i].c_str());
        spec.pszSpec = (wchar_t*)(result.specsW[i].c_str());
        result.specs.push_back(spec);
    }
    return result;
}

} // anonymous namespace

// ---------- 公共接口实现 ----------

Cube::Path FileDialog::openFile(const std::string& title, const std::vector<FilterSpec>& filters, const Cube::Path& defaultPath) {
    HRESULT hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        std::string result;
        IFileOpenDialog* pDlg = nullptr;
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, (void**)(&pDlg));
        if (SUCCEEDED(hr)) {
            FILEOPENDIALOGOPTIONS options = 0;
            pDlg->GetOptions(&options);
            options |= FOS_FORCEFILESYSTEM;
            options |= FOS_PATHMUSTEXIST;
            options |= FOS_NOCHANGEDIR;
            options |= FOS_FILEMUSTEXIST;
            options |= FOS_NOREADONLYRETURN;
            pDlg->SetOptions(options);
            if (!title.empty()) {
                pDlg->SetTitle(Cube::Utils::utf8ToWchar(title).c_str());
            }
            if (!defaultPath.empty()) {
                IShellItem* pFolder = nullptr;
                hr = SHCreateItemFromParsingName(Cube::Utils::utf8ToWchar(defaultPath.string()).c_str(), nullptr, IID_IShellItem, (void**)&pFolder);
                if (SUCCEEDED(hr)) {
                    pDlg->SetDefaultFolder(pFolder);
                    pFolder->Release();
                }
            }

            auto filterSpecs = buildFilterSpecs(filters);
            if (!filterSpecs.specs.empty()) {
                pDlg->SetFileTypes(static_cast<UINT>(filterSpecs.specs.size()), filterSpecs.specs.data());
                pDlg->SetFileTypeIndex(1);
            }
            hr = pDlg->Show(nullptr);
            if (SUCCEEDED(hr)) {
                IShellItem* pItem = nullptr;
                hr = pDlg->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    result = getPathFromItem(pItem);
                    pItem->Release();
                }
            }
            pDlg->Release();
        }
        CoUninitialize();
        return Cube::Path(result);
    }
    return Cube::Path();
}

Cube::Path FileDialog::saveFile(const std::string& title, const std::vector<FilterSpec>& filters, const Cube::Path& defaultPath, const std::string& defaultExtension) {
    HRESULT hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        std::string result;
        IFileOpenDialog* pDlg = nullptr;
        hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog, (void**)(&pDlg));
        if (SUCCEEDED(hr)) {
            FILEOPENDIALOGOPTIONS options = 0;
            pDlg->GetOptions(&options);
            options |= FOS_FORCEFILESYSTEM;
            options |= FOS_PATHMUSTEXIST;
            options |= FOS_NOCHANGEDIR;
            options |= FOS_OVERWRITEPROMPT;
            pDlg->SetOptions(options);
            if (!title.empty()) {
                pDlg->SetTitle(Cube::Utils::utf8ToWchar(title).c_str());
            }
            if (!defaultPath.empty()) {
                IShellItem* pFolder = nullptr;
                hr = SHCreateItemFromParsingName(Cube::Utils::utf8ToWchar(defaultPath.string()).c_str(), nullptr, IID_IShellItem, (void**)&pFolder);
                if (SUCCEEDED(hr)) {
                    pDlg->SetDefaultFolder(pFolder);
                    pFolder->Release();
                }
            }
            if (!defaultExtension.empty()) {
                pDlg->SetDefaultExtension(Cube::Utils::utf8ToWchar(defaultPath.string()).c_str());
            }
            auto filterSpecs = buildFilterSpecs(filters);
            if (!filterSpecs.specs.empty()) {
                pDlg->SetFileTypes(static_cast<UINT>(filterSpecs.specs.size()), filterSpecs.specs.data());
                pDlg->SetFileTypeIndex(1);
            }
            hr = pDlg->Show(nullptr);
            if (SUCCEEDED(hr)) {
                IShellItem* pItem = nullptr;
                hr = pDlg->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    result = getPathFromItem(pItem);
                    pItem->Release();
                }
            }
            pDlg->Release();
        }
        CoUninitialize();
        return Cube::Path(result);
    }
    return Cube::Path();
}

Cube::Path FileDialog::selectDir(const std::string& title, const Cube::Path& defaultPath) {
    HRESULT hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        std::string result;
        IFileOpenDialog* pDlg = nullptr;
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, (void**)(&pDlg));
        if (SUCCEEDED(hr)) {
            FILEOPENDIALOGOPTIONS options = 0;
            pDlg->GetOptions(&options);
            options |= FOS_FORCEFILESYSTEM;
            options |= FOS_PATHMUSTEXIST;
            options |= FOS_NOCHANGEDIR;
            options |= FOS_PICKFOLDERS;
            pDlg->SetOptions(options);
            if (!title.empty()) {
                pDlg->SetTitle(Cube::Utils::utf8ToWchar(title).c_str());
            }
            if (!defaultPath.empty()) {
                IShellItem* pFolder = nullptr;
                hr = SHCreateItemFromParsingName(Cube::Utils::utf8ToWchar(defaultPath.string()).c_str(), nullptr, IID_IShellItem, (void**)&pFolder);
                if (SUCCEEDED(hr)) {
                    pDlg->SetDefaultFolder(pFolder);
                    pFolder->Release();
                }
            }
            hr = pDlg->Show(nullptr);
            if (SUCCEEDED(hr)) {
                IShellItem* pItem = nullptr;
                hr = pDlg->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    result = getPathFromItem(pItem);
                    pItem->Release();
                }
            }
            pDlg->Release();
        }
        CoUninitialize();
        return Cube::Path(result);
    }
    return Cube::Path();
}

std::vector<Cube::Path> FileDialog::openMultiFiles(const std::string& title, const std::vector<FilterSpec>& filters, const Cube::Path& defaultPath) {
    HRESULT hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        std::vector<Cube::Path> results;
        IFileOpenDialog* pDlg = nullptr;
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, (void**)(&pDlg));
        if (SUCCEEDED(hr)) {
            FILEOPENDIALOGOPTIONS options = 0;
            pDlg->GetOptions(&options);
            options |= FOS_FORCEFILESYSTEM;
            options |= FOS_PATHMUSTEXIST;
            options |= FOS_NOCHANGEDIR;
            options |= FOS_FILEMUSTEXIST;
            options |= FOS_NOREADONLYRETURN;
            options |= FOS_ALLOWMULTISELECT;
            pDlg->SetOptions(options);
            if (!title.empty()) {
                pDlg->SetTitle(Cube::Utils::utf8ToWchar(title).c_str());
            }
            if (!defaultPath.empty()) {
                IShellItem* pFolder = nullptr;
                hr = SHCreateItemFromParsingName(Cube::Utils::utf8ToWchar(defaultPath.string()).c_str(), nullptr, IID_IShellItem, (void**)&pFolder);
                if (SUCCEEDED(hr)) {
                    pDlg->SetDefaultFolder(pFolder);
                    pFolder->Release();
                }
            }

            auto filterSpecs = buildFilterSpecs(filters);
            if (!filterSpecs.specs.empty()) {
                pDlg->SetFileTypes(static_cast<UINT>(filterSpecs.specs.size()), filterSpecs.specs.data());
                pDlg->SetFileTypeIndex(1);
            }
            hr = pDlg->Show(nullptr);
            if (SUCCEEDED(hr)) {
                IShellItemArray* pItems = nullptr;
                hr = pDlg->GetResults(&pItems);
                if (SUCCEEDED(hr) && pItems) {
                    DWORD count = 0;
                    pItems->GetCount(&count);
                    results.reserve(count);
                    for (DWORD i = 0; i < count; ++i) {
                        IShellItem* pItem = nullptr;
                        hr = pItems->GetItemAt(i, &pItem);
                        if (SUCCEEDED(hr) && pItem) {
                            std::string path = getPathFromItem(pItem);
                            if (!path.empty()) {
                                results.push_back(Cube::Path(path));
                            }
                            pItem->Release();
                        }
                    }
                    pItems->Release();
                }
            }
            pDlg->Release();
        }
        CoUninitialize();
        return results;
    }
    return {};
}

} // namespace Utils
