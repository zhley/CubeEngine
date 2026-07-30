#include "FileDialog.h"

#include <commctrl.h>
#include <commdlg.h>
#include <ShlObj.h>

#include"Cube/Utils/Utils.h"

namespace Cube {

    std::string FileDialog::openFile(const char* filter, HWND owner) {
        OPENFILENAMEA ofn;
        char filePath[260] = {};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = owner;
        ofn.lpstrFile = filePath;
        ofn.nMaxFile = sizeof(filePath);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if(GetOpenFileNameA(&ofn)) {
            std::string res(filePath);
            Utils::normalizePath(res);
            return res;
        }
        return "";
    }

    std::string FileDialog::saveFile(const char* filter, HWND owner) {
        OPENFILENAMEA ofn;
        char filePath[260] = {};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = owner;
        ofn.lpstrFile = filePath;
        ofn.nMaxFile = sizeof(filePath);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        if(GetOpenFileNameA(&ofn)) {
            std::string res(filePath);
            Utils::normalizePath(res);
            return res;
        }
        return "";
    }

    std::string FileDialog::selectDir(HWND owner) {
        BROWSEINFOA bi;
        char path[260] = {};

        ZeroMemory(&bi, sizeof(bi));
        bi.hwndOwner = owner;
        bi.pszDisplayName = path;
        bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX;

        LPITEMIDLIST idl = SHBrowseForFolderA(&bi);
        if(idl != NULL && SHGetPathFromIDListA(idl, path)) {
            std::string res(path);
            Utils::normalizePath(res);
            return res;
        }
        return "";
    }

    std::vector<std::string> FileDialog::openMultiFiles(const char* filter, HWND owner) {
        OPENFILENAMEA ofn;
        char filePath[4096] = {};
        
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = owner;
        ofn.lpstrFile = filePath;
        ofn.nMaxFile = sizeof(filePath);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

        std::vector<std::string> res;
        if(GetOpenFileNameA(&ofn)) {
            std::string dir(filePath);
            Utils::normalizePath(dir);
            if(filePath[dir.size() + 1] == '\0') {
                res.push_back(std::move(dir));
                return res;
            }
            dir += '/';
            std::string fileName;
            for(auto i = dir.size(); i < 4096; ++i) {
                if(filePath[i] == '\0') {
                    if(fileName.empty()) {
                        break;
                    }
                    res.push_back(dir + fileName);
                    fileName = "";
                    continue;
                }
                fileName += filePath[i];
            }
        }
        return res;
    }

}  // namespace Cube