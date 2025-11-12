#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <atlbase.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#pragma comment(lib, "shlwapi.lib")

// Structure to hold application configuration
struct AppConfig {
    std::string executable;
    bool runAsAdmin;
    std::string args;
    int hotkeyId;
    UINT modifiers;
    UINT vkCode;
    bool enabled;  // Whether this app is currently active

    AppConfig() : runAsAdmin(false), hotkeyId(0), modifiers(0), vkCode(0), enabled(true) {}
};

// Structure to hold settings configuration
struct Settings {
    bool checkMouseHover;
    bool checkFocusedWindow;
    std::string priorityWhenBothAvailable; // "hover" or "focus"

    Settings() : checkMouseHover(true), checkFocusedWindow(true), priorityWhenBothAvailable("hover") {}
};

// Global configuration
std::map<std::string, AppConfig> g_appConfigs;
Settings g_settings;
std::string g_configPath;

// Forward declarations
bool RegisterHotkeys();
void UnregisterHotkeys();

// Function to get the executable directory
std::string GetExeDirectory() {
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    PathRemoveFileSpec(path);
    return std::string(path);
}

// Function to get the AppData directory for config storage
std::string GetConfigDirectory() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        std::string configDir = std::string(path) + "\\ContextLauncher";
        // Create directory if it doesn't exist
        CreateDirectory(configDir.c_str(), NULL);
        return configDir;
    }
    // Fallback to exe directory if AppData fails
    return GetExeDirectory();
}

// Function to trim whitespace from string
std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

// FIXED: Helper function to convert virtual key code to string representation
std::string VirtualKeyToString(UINT vkCode) {
    // Handle F-keys
    if (vkCode >= VK_F1 && vkCode <= VK_F24) {
        int fNum = (vkCode - VK_F1) + 1;
        return "F" + std::to_string(fNum);
    }

    // Handle special keys
    static const std::map<UINT, std::string> vkToString = {
        {VK_DELETE, "Delete"}, {VK_INSERT, "Insert"},
        {VK_HOME, "Home"}, {VK_END, "End"},
        {VK_PRIOR, "PageUp"}, {VK_NEXT, "PageDown"},
        {VK_UP, "Up"}, {VK_DOWN, "Down"}, {VK_LEFT, "Left"}, {VK_RIGHT, "Right"},
        {VK_SPACE, "Space"}, {VK_TAB, "Tab"},
        {VK_RETURN, "Enter"}, {VK_ESCAPE, "Escape"},
        {VK_BACK, "Backspace"},
        {VK_NUMPAD0, "Numpad0"}, {VK_NUMPAD1, "Numpad1"}, {VK_NUMPAD2, "Numpad2"},
        {VK_NUMPAD3, "Numpad3"}, {VK_NUMPAD4, "Numpad4"}, {VK_NUMPAD5, "Numpad5"},
        {VK_NUMPAD6, "Numpad6"}, {VK_NUMPAD7, "Numpad7"}, {VK_NUMPAD8, "Numpad8"},
        {VK_NUMPAD9, "Numpad9"},
        {VK_MULTIPLY, "Multiply"}, {VK_ADD, "Add"}, {VK_SUBTRACT, "Subtract"},
        {VK_DIVIDE, "Divide"}, {VK_DECIMAL, "Decimal"}
    };

    auto it = vkToString.find(vkCode);
    if (it != vkToString.end()) {
        return it->second;
    }

    // For regular character keys
    if (vkCode >= 0x30 && vkCode <= 0x5A) { // 0-9, A-Z
        return std::string(1, (char)vkCode);
    }

    // Fallback
    return std::string(1, (char)vkCode);
}

// FIXED: Function to parse hotkey string - now supports single keys without modifiers
bool ParseHotkey(const std::string& hotkeyStr, UINT& modifiers, UINT& vkCode) {
    modifiers = 0;
    vkCode = 0;

    std::string str = hotkeyStr;
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);

    if (str.find("CTRL") != std::string::npos) modifiers |= MOD_CONTROL;
    if (str.find("ALT") != std::string::npos) modifiers |= MOD_ALT;
    if (str.find("SHIFT") != std::string::npos) modifiers |= MOD_SHIFT;
    if (str.find("WIN") != std::string::npos) modifiers |= MOD_WIN;

    // Extract the final key
    size_t lastPlus = str.find_last_of('+');
    std::string key;

    // FIXED: Handle both cases - with and without modifiers
    if (lastPlus != std::string::npos && lastPlus < str.length() - 1) {
        key = str.substr(lastPlus + 1);
    }
    else {
        // No modifiers found - entire string is the key
        key = str;
    }
    key = Trim(key);

    if (key.empty()) {
        return false;
    }

    // Handle F-keys (F1-F24)
    if (key.length() >= 2 && key[0] == 'F') {
        std::string numStr = key.substr(1);
        try {
            int fNum = std::stoi(numStr);
            if (fNum >= 1 && fNum <= 24) {
                vkCode = VK_F1 + (fNum - 1);
                return true;
            }
        }
        catch (...) {
            return false;
        }
    }

    // Handle special keys
    static const std::map<std::string, UINT> specialKeys = {
        {"DELETE", VK_DELETE}, {"DEL", VK_DELETE},
        {"INSERT", VK_INSERT}, {"INS", VK_INSERT},
        {"HOME", VK_HOME},
        {"END", VK_END},
        {"PAGEUP", VK_PRIOR}, {"PGUP", VK_PRIOR},
        {"PAGEDOWN", VK_NEXT}, {"PGDN", VK_NEXT},
        {"UP", VK_UP}, {"DOWN", VK_DOWN}, {"LEFT", VK_LEFT}, {"RIGHT", VK_RIGHT},
        {"SPACE", VK_SPACE},
        {"TAB", VK_TAB},
        {"ENTER", VK_RETURN}, {"RETURN", VK_RETURN},
        {"ESCAPE", VK_ESCAPE}, {"ESC", VK_ESCAPE},
        {"BACKSPACE", VK_BACK}, {"BACK", VK_BACK},
        {"NUMPAD0", VK_NUMPAD0}, {"NUMPAD1", VK_NUMPAD1}, {"NUMPAD2", VK_NUMPAD2},
        {"NUMPAD3", VK_NUMPAD3}, {"NUMPAD4", VK_NUMPAD4}, {"NUMPAD5", VK_NUMPAD5},
        {"NUMPAD6", VK_NUMPAD6}, {"NUMPAD7", VK_NUMPAD7}, {"NUMPAD8", VK_NUMPAD8},
        {"NUMPAD9", VK_NUMPAD9},
        {"MULTIPLY", VK_MULTIPLY}, {"ADD", VK_ADD}, {"SUBTRACT", VK_SUBTRACT},
        {"DIVIDE", VK_DIVIDE}, {"DECIMAL", VK_DECIMAL}
    };

    auto it = specialKeys.find(key);
    if (it != specialKeys.end()) {
        vkCode = it->second;
        return true;
    }

    // Handle single character keys (letters, numbers, symbols)
    if (key.length() == 1) {
        vkCode = VkKeyScan(key[0]) & 0xFF;
        return true;
    }

    return false;
}

// Function to load configuration from INI file
bool LoadConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    std::string currentSection;
    int hotkeyCounter = 1;

    while (std::getline(file, line)) {
        line = Trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Check for section header
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            currentSection = line.substr(1, line.length() - 2);
            currentSection = Trim(currentSection);
            continue;
        }

        // Parse key=value
        size_t equalsPos = line.find('=');
        if (equalsPos != std::string::npos) {
            std::string key = Trim(line.substr(0, equalsPos));
            std::string value = Trim(line.substr(equalsPos + 1));

            if (currentSection == "Settings") {
                // Parse settings
                if (key == "checkMouseHover") {
                    g_settings.checkMouseHover = (value == "true" || value == "1");
                }
                else if (key == "checkFocusedWindow") {
                    g_settings.checkFocusedWindow = (value == "true" || value == "1");
                }
                else if (key == "priorityWhenBothAvailable") {
                    std::string lowerValue = value;
                    std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);
                    if (lowerValue == "hover" || lowerValue == "focus") {
                        g_settings.priorityWhenBothAvailable = lowerValue;
                    }
                }
            }
            else if (currentSection == "Apps") {
                // Format: name=executable|admin|args|hotkey|enabled
                std::string name = key;
                AppConfig config;

                // Parse pipe-delimited values
                std::vector<std::string> parts;
                std::stringstream ss(value);
                std::string part;
                while (std::getline(ss, part, '|')) {
                    parts.push_back(Trim(part));
                }

                if (parts.size() >= 4) {
                    config.executable = parts[0];
                    config.runAsAdmin = (parts[1] == "true" || parts[1] == "1");
                    config.args = parts[2];

                    // Parse enabled flag (optional, defaults to true)
                    if (parts.size() >= 5) {
                        config.enabled = (parts[4] == "true" || parts[4] == "1");
                    }
                    else {
                        config.enabled = true; // Default to enabled for backward compatibility
                    }

                    // Parse hotkey
                    if (ParseHotkey(parts[3], config.modifiers, config.vkCode)) {
                        config.hotkeyId = hotkeyCounter++;
                        g_appConfigs[name] = config;
                    }
                }
            }
        }
    }

    file.close();
    return !g_appConfigs.empty();
}

// Function to create default config file
void CreateDefaultConfig(const std::string& configPath) {
    std::ofstream file(configPath);
    if (!file.is_open()) {
        return;
    }

    file << "; Launcher Configuration File\n";
    file << "\n";
    file << "[Settings]\n";
    file << "; Enable/disable detection methods\n";
    file << "checkMouseHover=true\n";
    file << "checkFocusedWindow=true\n";
    file << "\n";
    file << "; Priority when both are available (hover or focus)\n";
    file << "; hover = Mouse position takes precedence\n";
    file << "; focus = Focused window takes precedence\n";
    file << "priorityWhenBothAvailable=hover\n";
    file << "\n";
    file << "[Apps]\n";
    file << "; Format: name=executable|runAsAdmin|args|hotkey|enabled\n";
    file << "; runAsAdmin: true or false\n";
    file << "; args: additional command line arguments (use empty string if none)\n";
    file << "; hotkey: e.g., Ctrl+Alt+P, Ctrl+Shift+C, etc.\n";
    file << "; enabled: true or false (allows disabling apps without deleting them)\n";
    file << "\n";
    file << "PowerShell=powershell.exe|false||Ctrl+Alt+P|true\n";
    file << "PowerShell Admin=powershell.exe|true||Ctrl+Alt+Shift+P|true\n";
    file << "Command Prompt=cmd.exe|false||Ctrl+Alt+C|true\n";
    file << "Windows Terminal=wt.exe|false||Ctrl+Alt+T|true\n";
    file << "VS Code=code|false|.|Ctrl+Alt+V|true\n";
    file << "Git Bash=C:\\Program Files\\Git\\git-bash.exe|false||Ctrl+Alt+G|true\n";

    file.close();
}

// Function to initialize COM
void InitializeCOM() {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM." << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Function to uninitialize COM
void UninitializeCOM() {
    CoUninitialize();
}

// Function to get the window under the cursor
HWND GetWindowUnderCursor() {
    POINT point;
    GetCursorPos(&point);
    HWND hwnd = WindowFromPoint(point);

    if (hwnd == NULL) {
        return NULL;
    }

    HWND parentHwnd = GetAncestor(hwnd, GA_ROOT);
    if (parentHwnd == hwnd) {
        return hwnd;
    }

    return parentHwnd;
}

// Function to get the currently focused window
HWND GetFocusedWindow() {
    return GetForegroundWindow();
}

// Function to get the relevant File Explorer window
HWND GetFileExplorerWindow(HWND hwnd) {
    char className[256];
    while (hwnd != NULL) {
        GetClassName(hwnd, className, sizeof(className));
        if (strcmp(className, "CabinetWClass") == 0 ||
            strcmp(className, "WorkerW") == 0 ||
            strcmp(className, "SysTreeView32") == 0) {
            return hwnd;
        }
        hwnd = GetParent(hwnd);
    }
    return NULL;
}

// Function to check if the window is a File Explorer window
bool IsFileExplorerWindow(HWND hwnd) {
    hwnd = GetFileExplorerWindow(hwnd);
    if (hwnd != NULL) {
        char className[256];
        GetClassName(hwnd, className, sizeof(className));
        return strcmp(className, "CabinetWClass") == 0 ||
            strcmp(className, "WorkerW") == 0 ||
            strcmp(className, "SysTreeView32") == 0;
    }
    return false;
}

// Function to retrieve the directory of the File Explorer window
std::string GetExplorerWindowDirectory(HWND hwnd) {
    std::string directory = "";

    if (hwnd == NULL) {
        return "";
    }

    if (!IsWindow(hwnd)) {
        return "";
    }

    hwnd = GetFileExplorerWindow(hwnd);
    if (hwnd == NULL) {
        return "";
    }

    CComPtr<IShellWindows> spShellWindows;
    HRESULT hr = spShellWindows.CoCreateInstance(CLSID_ShellWindows);
    if (FAILED(hr)) {
        return "";
    }

    SHANDLE_PTR hwndShell;
    CComPtr<IDispatch> spdisp;

    for (long i = 0; ; i++) {
        spdisp.Release();

        hr = spShellWindows->Item(CComVariant(i), &spdisp);
        if (FAILED(hr)) {
            break;
        }

        if (spdisp == nullptr) {
            break;
        }

        CComPtr<IWebBrowserApp> spWebBrowserApp;
        spdisp.QueryInterface(&spWebBrowserApp);

        if (spWebBrowserApp) {
            spWebBrowserApp->get_HWND(&hwndShell);

            if ((HWND)hwndShell == hwnd) {
                CComPtr<IDispatch> spDispDoc;
                hr = spWebBrowserApp->get_Document(&spDispDoc);
                if (FAILED(hr) || spDispDoc == nullptr) {
                    continue;
                }

                CComPtr<IServiceProvider> spServiceProvider;
                hr = spDispDoc->QueryInterface(IID_PPV_ARGS(&spServiceProvider));
                if (SUCCEEDED(hr) && spServiceProvider) {
                    CComPtr<IShellBrowser> spShellBrowser;
                    hr = spServiceProvider->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&spShellBrowser));
                    if (SUCCEEDED(hr) && spShellBrowser) {
                        CComPtr<IShellView> spShellView;
                        hr = spShellBrowser->QueryActiveShellView(&spShellView);
                        if (SUCCEEDED(hr) && spShellView) {
                            CComPtr<IFolderView> spFolderView;
                            hr = spShellView->QueryInterface(IID_PPV_ARGS(&spFolderView));
                            if (SUCCEEDED(hr) && spFolderView) {
                                CComPtr<IPersistFolder2> spPersistFolder2;
                                hr = spFolderView->GetFolder(IID_PPV_ARGS(&spPersistFolder2));
                                if (SUCCEEDED(hr) && spPersistFolder2) {
                                    LPITEMIDLIST pidl = nullptr;
                                    hr = spPersistFolder2->GetCurFolder(&pidl);
                                    if (SUCCEEDED(hr) && pidl) {
                                        char path[MAX_PATH];
                                        if (SHGetPathFromIDList(pidl, path)) {
                                            directory = std::string(path);
                                        }
                                        CoTaskMemFree(pidl);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return directory;
}

// Function to get the user's home directory
std::string GetUserHomeDirectory() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        return std::string(path);
    }
    return "";
}

// Function to get the directory to launch in
std::string GetLaunchDirectory() {
    std::string hoverDir = "";
    std::string focusDir = "";

    // Check mouse hover if enabled
    if (g_settings.checkMouseHover) {
        HWND hwndUnderCursor = GetWindowUnderCursor();
        if (hwndUnderCursor != NULL && IsFileExplorerWindow(hwndUnderCursor)) {
            hoverDir = GetExplorerWindowDirectory(hwndUnderCursor);
        }
    }

    // Check focused window if enabled
    if (g_settings.checkFocusedWindow) {
        HWND hwndFocused = GetFocusedWindow();
        // Make sure it's different from cursor window to avoid redundant checks
        if (hwndFocused != NULL && IsFileExplorerWindow(hwndFocused)) {
            focusDir = GetExplorerWindowDirectory(hwndFocused);
        }
    }

    // Determine which directory to use based on priority
    if (!hoverDir.empty() && !focusDir.empty()) {
        // Both found - use priority setting
        if (g_settings.priorityWhenBothAvailable == "hover") {
            return hoverDir;
        }
        else {
            return focusDir;
        }
    }
    else if (!hoverDir.empty()) {
        // Only hover found
        return hoverDir;
    }
    else if (!focusDir.empty()) {
        // Only focus found
        return focusDir;
    }

    // Nothing found - use home directory
    return GetUserHomeDirectory();
}

// Function to launch application
void LaunchApplication(const AppConfig& config) {
    std::string directory = GetLaunchDirectory();
    const char* verb = config.runAsAdmin ? "runas" : "open";
    const char* args = config.args.empty() ? NULL : config.args.c_str();
    const char* dir = directory.empty() ? NULL : directory.c_str();

    HINSTANCE result = ShellExecute(NULL, verb, config.executable.c_str(),
        args, dir, SW_SHOWNORMAL);

    if ((INT_PTR)result <= 32) {
        // If launch failed, try from home directory
        std::string homeDir = GetUserHomeDirectory();
        ShellExecute(NULL, verb, config.executable.c_str(),
            args, homeDir.empty() ? NULL : homeDir.c_str(), SW_SHOWNORMAL);
    }
}

// Hidden window for message processing
HWND g_hwnd = NULL;

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_HOTKEY:
        // Find which app corresponds to this hotkey ID
        for (const auto& pair : g_appConfigs) {
            if (pair.second.hotkeyId == (int)wParam && pair.second.enabled) {
                LaunchApplication(pair.second);
                break;
            }
        }
        return 0;

    case WM_USER + 1: // Tray icon message
        if (lParam == WM_RBUTTONUP) {
            // Right-click on tray icon
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = CreatePopupMenu();

            // Add app status submenu
            int menuId = 100;
            for (const auto& pair : g_appConfigs) {
                UINT flags = MF_STRING;
                if (pair.second.enabled) {
                    flags |= MF_CHECKED;
                }
                AppendMenu(hMenu, flags, menuId++, pair.first.c_str());
            }

            if (!g_appConfigs.empty()) {
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            }

            AppendMenu(hMenu, MF_STRING, 1, "Open Config Editor");
            AppendMenu(hMenu, MF_STRING, 2, "Open Config File");
            AppendMenu(hMenu, MF_STRING, 3, "Reload Config");
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, 99, "Exit");

            // Required for popup menu to work correctly
            SetForegroundWindow(hwnd);

            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);

            // FIXED: Changed from config-editor.html to ConfigEditor.exe
            if (cmd == 1) {
                // Open config editor
                char path[MAX_PATH];
                GetModuleFileName(NULL, path, MAX_PATH);
                PathRemoveFileSpec(path);

                std::string editorPath = std::string(path) + "\\ConfigEditor.exe";

                // Check if already running
                HWND existingWindow = FindWindow(NULL, "Context Launcher - Configuration");
                if (existingWindow) {
                    // Bring existing window to front
                    SetForegroundWindow(existingWindow);
                    ShowWindow(existingWindow, SW_RESTORE);
                }
                else if (PathFileExists(editorPath.c_str())) {
                    ShellExecute(NULL, "open", editorPath.c_str(), NULL, NULL, SW_SHOW);
                }
                else {
                    MessageBox(NULL, "ConfigEditor.exe not found in the same directory as the launcher!", "Error", MB_OK | MB_ICONERROR);
                }
            }
            else if (cmd == 2) {
                // Open config file in default text editor
                ShellExecute(NULL, "open", g_configPath.c_str(), NULL, NULL, SW_SHOW);
            }
            else if (cmd == 3) {
                // Reload config
                UnregisterHotkeys();
                g_appConfigs.clear();
                if (LoadConfig(g_configPath)) {
                    if (RegisterHotkeys()) {
                        MessageBox(NULL, "Configuration reloaded successfully!", "Success", MB_OK | MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(NULL, "Failed to register hotkeys after reload.", "Error", MB_OK | MB_ICONERROR);
                    }
                }
                else {
                    MessageBox(NULL, "Failed to reload configuration file.", "Error", MB_OK | MB_ICONERROR);
                }
            }
            else if (cmd >= 100 && cmd < 100 + (int)g_appConfigs.size()) {
                // Toggle app enabled state
                int index = cmd - 100;
                auto it = g_appConfigs.begin();
                std::advance(it, index);

                // Toggle the enabled state
                it->second.enabled = !it->second.enabled;

                // Update the config file
                std::ofstream file(g_configPath);
                if (file.is_open()) {
                    file << "; Launcher Configuration File\n\n";

                    // Write Settings section
                    file << "[Settings]\n";
                    file << "; Enable/disable detection methods\n";
                    file << "checkMouseHover=" << (g_settings.checkMouseHover ? "true" : "false") << "\n";
                    file << "checkFocusedWindow=" << (g_settings.checkFocusedWindow ? "true" : "false") << "\n";
                    file << "\n";
                    file << "; Priority when both are available (hover or focus)\n";
                    file << "; hover = Mouse position takes precedence\n";
                    file << "; focus = Focused window takes precedence\n";
                    file << "priorityWhenBothAvailable=" << g_settings.priorityWhenBothAvailable << "\n";
                    file << "\n";

                    // Write Apps section
                    file << "[Apps]\n";
                    file << "; Format: name=executable|runAsAdmin|args|hotkey|enabled\n";
                    file << "; runAsAdmin: true or false\n";
                    file << "; args: additional command line arguments (use empty string if none)\n";
                    file << "; hotkey: e.g., Ctrl+Alt+P, Ctrl+Shift+C, etc.\n";
                    file << "; enabled: true or false (allows disabling apps without deleting them)\n";
                    file << "\n";

                    for (const auto& pair : g_appConfigs) {
                        file << pair.first << "="
                            << pair.second.executable << "|"
                            << (pair.second.runAsAdmin ? "true" : "false") << "|"
                            << pair.second.args << "|";

                        // FIXED: Reconstruct hotkey string using proper helper function
                        std::string hotkey;
                        if (pair.second.modifiers & MOD_CONTROL) hotkey += "Ctrl+";
                        if (pair.second.modifiers & MOD_ALT) hotkey += "Alt+";
                        if (pair.second.modifiers & MOD_SHIFT) hotkey += "Shift+";
                        if (pair.second.modifiers & MOD_WIN) hotkey += "Win+";
                        hotkey += VirtualKeyToString(pair.second.vkCode);

                        file << hotkey << "|"
                            << (pair.second.enabled ? "true" : "false") << "\n";
                    }

                    file.close();

                    // Reload hotkeys
                    UnregisterHotkeys();
                    RegisterHotkeys();
                }
            }
            else if (cmd == 99) {
                // Exit
                PostQuitMessage(0);
            }

            DestroyMenu(hMenu);
        }
        // FIXED: Changed from config-editor.html to ConfigEditor.exe
        else if (lParam == WM_LBUTTONDBLCLK) {
            // Double-click on tray icon - open config editor
            char path[MAX_PATH];
            GetModuleFileName(NULL, path, MAX_PATH);
            PathRemoveFileSpec(path);

            std::string editorPath = std::string(path) + "\\ConfigEditor.exe";

            // Check if already running
            HWND existingWindow = FindWindow(NULL, "Context Launcher - Configuration");
            if (existingWindow) {
                // Bring existing window to front
                SetForegroundWindow(existingWindow);
                ShowWindow(existingWindow, SW_RESTORE);
            }
            else if (PathFileExists(editorPath.c_str())) {
                ShellExecute(NULL, "open", editorPath.c_str(), NULL, NULL, SW_SHOW);
            }
            else {
                MessageBox(NULL, "ConfigEditor.exe not found in the same directory as the launcher!", "Error", MB_OK | MB_ICONERROR);
            }
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Function to create hidden window for message processing
bool CreateMessageWindow() {
    const char CLASS_NAME[] = "LauncherWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    g_hwnd = CreateWindowEx(
        0, CLASS_NAME, "Launcher", 0,
        0, 0, 0, 0,
        HWND_MESSAGE, NULL, GetModuleHandle(NULL), NULL
    );

    return g_hwnd != NULL;
}

// Function to register all hotkeys
bool RegisterHotkeys() {
    for (const auto& pair : g_appConfigs) {
        const AppConfig& config = pair.second;
        // Only register hotkey if the app is enabled
        if (config.enabled) {
            if (!RegisterHotKey(g_hwnd, config.hotkeyId, config.modifiers, config.vkCode)) {
                return false;
            }
        }
    }
    return true;
}

// Function to unregister all hotkeys
void UnregisterHotkeys() {
    for (const auto& pair : g_appConfigs) {
        UnregisterHotKey(g_hwnd, pair.second.hotkeyId);
    }
}

// Function to show tray icon
NOTIFYICONDATA g_nid = {};

void CreateTrayIcon() {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = g_hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_USER + 1;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy_s(g_nid.szTip, "Context Launcher");

    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    InitializeCOM();

    // Determine config file path in AppData
    g_configPath = GetConfigDirectory() + "\\launcher.ini";

    // Check command line arguments
    bool oneShot = (lpCmdLine != NULL && strstr(lpCmdLine, "--oneshot") != NULL);
    bool createConfig = (lpCmdLine != NULL && strstr(lpCmdLine, "--create-config") != NULL);
    bool skipConfigEditor = (lpCmdLine != NULL && strstr(lpCmdLine, "--skip-config") != NULL);
    bool forceSetup = (lpCmdLine != NULL && strstr(lpCmdLine, "--setup") != NULL);

    // Check if this is first run (config doesn't exist)
    bool isFirstRun = !PathFileExists(g_configPath.c_str());

    // Create default config if requested or if it doesn't exist
    if (createConfig || isFirstRun) {
        CreateDefaultConfig(g_configPath);
        if (createConfig) {
            MessageBox(NULL, "Default configuration file created.", "Success", MB_OK | MB_ICONINFORMATION);
            UninitializeCOM();
            return 0;
        }
    }

    // On first run or forced setup, launch config editor
    if ((isFirstRun || forceSetup) && !skipConfigEditor) {
        std::string configEditorPath = GetExeDirectory() + "\\ConfigEditor.exe";
        if (PathFileExists(configEditorPath.c_str())) {
            ShellExecute(NULL, "open", configEditorPath.c_str(), NULL, NULL, SW_SHOW);
            if (isFirstRun) {
                MessageBox(NULL, "Welcome to Context Launcher!\n\nThe configuration editor has been opened. Please configure your applications and hotkeys, then restart the launcher.", "First Run", MB_OK | MB_ICONINFORMATION);
            }
            UninitializeCOM();
            return 0;
        }
    }

    // Load configuration
    if (!LoadConfig(g_configPath)) {
        MessageBox(NULL, "Failed to load configuration file.", "Error", MB_OK | MB_ICONERROR);
        UninitializeCOM();
        return 1;
    }

    // One-shot mode: launch default app (first in config) and exit
    if (oneShot) {
        if (!g_appConfigs.empty()) {
            LaunchApplication(g_appConfigs.begin()->second);
        }
        UninitializeCOM();
        return 0;
    }

    // Listener mode: create window and register hotkeys
    if (!CreateMessageWindow()) {
        MessageBox(NULL, "Failed to create message window.", "Error", MB_OK | MB_ICONERROR);
        UninitializeCOM();
        return 1;
    }

    if (!RegisterHotkeys()) {
        // Build error message with list of failed hotkeys
        std::string errorMsg = "Failed to register one or more hotkeys. They may already be in use by another application.\n\nFailed hotkeys:\n";

        // Try to identify which hotkeys failed
        for (const auto& pair : g_appConfigs) {
            const AppConfig& config = pair.second;
            if (config.enabled) {
                // Test each hotkey individually
                int testId = 9999;
                if (!RegisterHotKey(g_hwnd, testId, config.modifiers, config.vkCode)) {
                    errorMsg += "- " + pair.first + "\n";
                }
                UnregisterHotKey(g_hwnd, testId);
            }
        }

        errorMsg += "\nPlease:\n1. Close other applications that might be using these hotkeys\n2. Open the Configuration Editor to change the hotkeys\n3. Try again";

        MessageBox(NULL, errorMsg.c_str(), "Hotkey Registration Failed", MB_OK | MB_ICONERROR);
        DestroyWindow(g_hwnd);
        UninitializeCOM();
        return 1;
    }

    CreateTrayIcon();

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    RemoveTrayIcon();
    UnregisterHotkeys();
    DestroyWindow(g_hwnd);
    UninitializeCOM();

    return 0;
}