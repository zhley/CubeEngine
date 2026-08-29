#include "FileDialog.h"
#include <windows.h>
#include <shobjidl.h>
#include <combaseapi.h> 
#include <vector>
#include <string>
#include <memory>

namespace Utils {
    
namespace {

// ---------- UTF-8 与 UTF-16 转换 ----------
// 将 UTF-8 字符串转换为 UTF-16 (宽字符)，用于 Windows API
static std::wstring utf8_to_utf16(const std::string& utf8) {
    if (utf8.empty()) return {};
    // 计算需要的宽字符长度（不包含结尾的 L'\0'）
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    if (len <= 0) return {};
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), &wstr[0], len);
    return wstr;
}

// 将 UTF-16 宽字符串转换为 UTF-8
static std::string utf16_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string utf8(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &utf8[0], len, nullptr, nullptr);
    return utf8;
}

// ---------- 从 IShellItem 提取路径 (UTF-8) ----------
static std::string getPathFromItem(IShellItem* pItem) {
    if (!pItem) return {};
    PWSTR pszPath = nullptr;
    HRESULT hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
    if (FAILED(hr) || !pszPath) return {};
    std::string result = utf16_to_utf8(pszPath);
    CoTaskMemFree(pszPath);
    return result;
}

// ---------- 构建 COM 过滤器数组 ----------
// 返回的 vector<COMDLG_FILTERSPEC> 中的指针指向 storedNames/storedSpecs 中的宽字符串
// 因此必须将这些宽字符串容器一并返回，保证生命周期
struct FilterSpecs {
    std::vector<COMDLG_FILTERSPEC> specs;
    std::vector<std::wstring> names;   // 存储转换后的宽字符串
    std::vector<std::wstring> specsW;  // 存储转换后的宽字符串
};

static FilterSpecs buildFilterSpecs(const std::vector<FileDialog::FilterSpec>& filters) {
    FilterSpecs result;
    if (filters.empty()) {
        // 默认：所有文件
        result.names.push_back(L"所有文件 (*.*)");
        result.specsW.push_back(L"*.*");
    } else {
        for (const auto& f : filters) {
            result.names.push_back(utf8_to_utf16(f.name));
            result.specsW.push_back(utf8_to_utf16(f.spec));
        }
    }
    // 构建 COMDLG_FILTERSPEC 数组
    for (size_t i = 0; i < result.names.size(); ++i) {
        COMDLG_FILTERSPEC spec;
        spec.pszName = result.names[i].c_str();
        spec.pszSpec = result.specsW[i].c_str();
        result.specs.push_back(spec);
    }
    return result;
}

// ---------- 通用对话框显示辅助 ----------
// 用于打开/保存单个文件，返回 UTF-8 路径
template<typename DialogType>
static std::string showDialog(
    const std::string& title,
    const std::vector<FileDialog::FilterSpec>& filters,
    const std::string& defaultPath,
    DWORD extraOptions = 0,
    bool isSave = false
) {
    // 初始化 COM（若已初始化则返回 S_FALSE，没问题）
    HRESULT hr = CoInitialize(nullptr);
    bool comInit = SUCCEEDED(hr); // 如果是 S_FALSE 也算成功，但我们不关心，最后都调用 CoUninitialize 是安全的

    std::string result;

    DialogType* pDlg = nullptr;
    CLSID clsid = isSave ? CLSID_FileSaveDialog : CLSID_FileOpenDialog;
    hr = CoCreateInstance(clsid, nullptr, CLSCTX_ALL, __uuidof(DialogType), (void**)&pDlg);
    if (SUCCEEDED(hr)) {
        // 设置标题
        if (!title.empty()) {
            pDlg->SetTitle(utf8_to_utf16(title).c_str());
        }

        // 设置默认文件夹
        if (!defaultPath.empty()) {
            IShellItem* pFolder = nullptr;
            hr = SHCreateItemFromParsingName(utf8_to_utf16(defaultPath).c_str(),
                                             nullptr, IID_IShellItem, (void**)&pFolder);
            if (SUCCEEDED(hr)) {
                pDlg->SetDefaultFolder(pFolder);
                pFolder->Release();
            }
        }

        // 设置过滤器
        auto filterSpecs = buildFilterSpecs(filters);
        if (!filterSpecs.specs.empty()) {
            pDlg->SetFileTypes(static_cast<UINT>(filterSpecs.specs.size()), filterSpecs.specs.data());
            pDlg->SetFileTypeIndex(1);
        }

        // 额外选项（如选择文件夹模式、多选等）
        if (extraOptions) {
            DWORD dwOptions;
            pDlg->GetOptions(&dwOptions);
            pDlg->SetOptions(dwOptions | extraOptions);
        }

        // 如果是保存对话框，设置默认扩展名（外部单独处理，这里无）
        // 注意：对于保存，extraOptions 可能为 0，我们没有传递默认扩展名，在外部单独设置

        // 显示对话框
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

    // 如果本函数调用了 CoInitialize 成功（即 S_OK，而不是 S_FALSE），则调用 CoUninitialize
    if (comInit) {
        CoUninitialize();
    }
    return result;
}

} // anonymous namespace

// ---------- 公共接口实现 ----------

std::string FileDialog::openFile(const std::string& title,
                                 const std::vector<FilterSpec>& filters,
                                 const std::string& defaultPath) {
    return showDialog<IFileOpenDialog>(title, filters, defaultPath, 0, false);
}

std::string FileDialog::saveFile(const std::string& title,
                                 const std::vector<FilterSpec>& filters,
                                 const std::string& defaultPath,
                                 const std::string& defaultExtension) {
    // 对于保存对话框，我们需要单独处理，因为需要设置默认扩展名
    // 而且不能直接用模板，因为模板会统一处理，但我们需要额外的 SetDefaultExtension 调用
    // 我们可以重写一个专门针对保存的实现，但为了不重复，此处专门实现。

    HRESULT hr = CoInitialize(nullptr);
    bool comInit = SUCCEEDED(hr);
    std::string result;

    IFileSaveDialog* pDlg = nullptr;
    hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog, (void**)&pDlg);
    if (SUCCEEDED(hr)) {
        if (!title.empty()) {
            pDlg->SetTitle(utf8_to_utf16(title).c_str());
        }
        if (!defaultPath.empty()) {
            IShellItem* pFolder = nullptr;
            hr = SHCreateItemFromParsingName(utf8_to_utf16(defaultPath).c_str(),
                                             nullptr, IID_IShellItem, (void**)&pFolder);
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

        if (!defaultExtension.empty()) {
            pDlg->SetDefaultExtension(utf8_to_utf16(defaultExtension).c_str());
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

    if (comInit) {
        CoUninitialize();
    }
    return result;
}

std::string FileDialog::selectDir(const std::string& title,
                                  const std::string& defaultPath) {
    // 使用 IFileOpenDialog，并加上 FOS_PICKFOLDERS 选项
    // 此时 filters 参数无意义，我们传入空 vector
    return showDialog<IFileOpenDialog>(title, {}, defaultPath, FOS_PICKFOLDERS, false);
}

std::vector<std::string> FileDialog::openMultiFiles(const std::string& title,
                                                    const std::vector<FilterSpec>& filters,
                                                    const std::string& defaultPath) {
    std::vector<std::string> results;
    HRESULT hr = CoInitialize(nullptr);
    bool comInit = SUCCEEDED(hr);

    IFileOpenDialog* pDlg = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&pDlg);
    if (SUCCEEDED(hr)) {
        // 启用多选
        DWORD dwOptions;
        pDlg->GetOptions(&dwOptions);
        pDlg->SetOptions(dwOptions | FOS_ALLOWMULTISELECT);

        if (!title.empty()) {
            pDlg->SetTitle(utf8_to_utf16(title).c_str());
        }
        if (!defaultPath.empty()) {
            IShellItem* pFolder = nullptr;
            hr = SHCreateItemFromParsingName(utf8_to_utf16(defaultPath).c_str(),
                                             nullptr, IID_IShellItem, (void**)&pFolder);
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
                            results.push_back(path);
                        }
                        pItem->Release();
                    }
                }
                pItems->Release();
            }
        }
        pDlg->Release();
    }

    if (comInit) {
        CoUninitialize();
    }
    return results;
}

} // namespace Utils
