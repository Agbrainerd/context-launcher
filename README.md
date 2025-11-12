# Context Launcher

A lightweight Windows utility that launches applications (PowerShell, CMD, VS Code, etc.) with the working directory automatically set to the File Explorer window under your mouse cursor or the currently focused window.

## Features

- **Smart directory detection** - Automatically detects Explorer windows under cursor or in focus (configurable, enable either or both and set precedence)
- **Configurable hotkeys** - Set custom keyboard shortcuts for each application
- **Multiple applications** - Launch PowerShell, CMD, Windows Terminal, VS Code, Git Bash, or any other app (configurable via GUI or .ini file)
- **Admin support** - Optionally launch apps with administrator privileges
- **INI configuration** - Simple text-based config file stored in AppData
- **System tray** - Runs quietly in the background
- **Native GUI config editor** - Easy-to-use Windows application for editing configuration

## Quick Start

### Download and Install
1. Download `ContextLauncher-Setup-1.1.0.exe` from the [releases page](https://github.com/agbrainerd/context-launcher/releases)
2. Run the installer
3. Configure your applications in the config editor
4. The launcher will start in the system tray
5. Use your configured hotkeys

### Build from Source

**Requirements:**
- Visual Studio 2019 or later (with C++ desktop development)
- Windows SDK

**Build Steps:**

Run the build script:
```bash
build-all.bat
```

This will compile both the launcher and config editor.

## Configuration

### Configuration File Location

The configuration file `launcher.ini` is stored in:
```
%APPDATA%\ContextLauncher\launcher.ini
```

This allows each user to have their own configuration without requiring administrator privileges to edit settings.

### Using the GUI Editor (Recommended)

1. Open the Start Menu and search for "Context Launcher Configuration Editor"
2. Add or modify application entries
3. Configure detection settings (mouse hover and/or focused window)
4. Click "Save Configuration"
5. Click "Save & Start Launcher" to apply changes

The config editor opens automatically on first install.

### Manual Configuration

Edit `%APPDATA%\ContextLauncher\launcher.ini` directly with any text editor.

**Format:**
```ini
[Settings]
checkMouseHover=true
checkFocusedWindow=true
priorityWhenBothAvailable=hover

[Apps]
name=executable|runAsAdmin|args|hotkey|enabled
```

**Example:**
```ini
[Apps]
PowerShell=powershell.exe|false||Ctrl+Alt+P|true
Command Prompt=cmd.exe|false||Ctrl+Alt+C|true
Windows Terminal=wt.exe|false||Ctrl+Alt+T|true
VS Code=code|false|.|Ctrl+Alt+V|true
```

**Configuration Fields:**
- **name**: Display name for the application
- **executable**: Path to the executable (can be in PATH or full path)
- **runAsAdmin**: `true` or `false` - whether to run with admin privileges
- **args**: Command line arguments (leave empty if none)
- **hotkey**: Keyboard shortcut (e.g., `Ctrl+Alt+P`, `Shift+F7`)
- **enabled**: `true` or `false` - whether this hotkey is active

**Supported Hotkey Modifiers:**
- `Ctrl` - Control key
- `Alt` - Alt key
- `Shift` - Shift key
- `Win` - Windows key

**Supported Keys:**
- Letters: `A` through `Z`
- Numbers: `0` through `9`
- Function keys: `F1` through `F24`
- Special keys: `Delete`, `Insert`, `Home`, `End`, `PageUp`, `PageDown`, `Space`, `Tab`, `Enter`, `Escape`, `Backspace`
- Arrow keys: `Up`, `Down`, `Left`, `Right`
- Numpad: `Numpad0` through `Numpad9`, `Multiply`, `Add`, `Subtract`, `Divide`, `Decimal`

## Usage

The launcher runs in the background as a system tray application. Once configured:

1. Press your configured hotkey anywhere in Windows
2. The launcher detects which File Explorer window is relevant
3. Your application opens in that directory

**System Tray Icon:**
- Right-click the tray icon for options
- "Reload Config" - Apply configuration changes without restarting
- "Exit" - Close the launcher

### Command Line Options

**One-Shot Mode:**
```bash
context-launcher.exe --oneshot
```
Launches the first configured app and exits immediately. Useful for integration with other tools.

**Force Setup Mode:**
```bash
context-launcher.exe --setup
```
Opens the configuration editor even if config already exists.

**Create Default Config:**
```bash
context-launcher.exe --create-config
```
Generates a default configuration file and exits.

## How It Works

When you press a configured hotkey:

1. **Check mouse position** - Is the cursor over a File Explorer window?
2. **Check focused window** - Is the focused window File Explorer?
3. **Extract directory** - Get the current directory from that Explorer window
4. **Launch application** - Start your app in that directory
5. **Fallback** - If no Explorer window is found, use your home directory

**Detection Priority:**
The "Priority when both available" setting in the config editor determines which window is used when both mouse hover and focused window detect Explorer windows.

## Hotkey Configuration Tips

**Recommended combinations:**
- `Ctrl+Alt+P` - PowerShell
- `Ctrl+Alt+C` - Command Prompt
- `Ctrl+Alt+T` - Windows Terminal
- `Ctrl+Alt+V` - VS Code
- `Ctrl+Alt+Shift+P` - PowerShell Admin

**Avoid:**
- System hotkeys: `Ctrl+Alt+Del`, `Win+L`, `Alt+Tab`
- Common application shortcuts (e.g., `Ctrl+C`, `Ctrl+V`)

## Application Examples

### PowerShell
```ini
PowerShell=powershell.exe|false||Ctrl+Alt+P
```

### PowerShell 7
```ini
PowerShell 7=pwsh.exe|false||Ctrl+Alt+Shift+P
```

### Windows Terminal
```ini
Windows Terminal=wt.exe|false||Ctrl+Alt+T
```

### VS Code (opens current directory)
```ini
VS Code=code|false|.|Ctrl+Alt+V
```

### Git Bash
```ini
Git Bash=C:\Program Files\Git\git-bash.exe|false||Ctrl+Alt+G
```

### Custom Python Environment
```ini
Python Env=cmd.exe|false|/k "cd /d {dir} && conda activate myenv"|Ctrl+Alt+Y
```

## Troubleshooting

### Hotkeys not working
- Check if hotkeys are already in use by another application (PowerToys, graphics drivers, etc.)
- Try different key combinations in the config editor
- Ensure the launcher is running (check system tray for icon)
- Right-click the tray icon and select "Reload Config" after making changes

### Application not launching
- Verify the executable path in the config editor
- Check if the executable is in your system PATH
- Try using the full path to the executable (e.g., `C:\Program Files\App\app.exe`)
- Test the command manually in Command Prompt first

### "Failed to register hotkeys" error
The error message will list which specific hotkeys failed. This means another application is using those keys.

**Common conflicts:**
- PowerToys Keyboard Manager
- Graphics driver shortcuts (NVIDIA, AMD)
- Other launcher utilities
- AutoHotkey scripts

**Solutions:**
- Close conflicting applications
- Change your hotkey combinations in the config editor
- Disable conflicting shortcuts in other applications

### Configuration not saving
- The config editor requires normal user privileges (not admin)
- Config is stored in `%APPDATA%\ContextLauncher\launcher.ini`
- Check that the folder exists and is writable
- Try running the config editor normally (not as administrator)

### Launcher won't start
- Check if an instance is already running (look in Task Manager)
- Verify the config file is valid by opening it in Notepad
- Delete the config file and let the launcher create a new default one
- Check Windows Event Viewer for crash details

## Advanced Usage

### Integration with Other Tools
Call the launcher from scripts or other applications:
```batch
start context-launcher.exe --oneshot
```

### AutoHotkey Integration
```ahk
^!p::  ; Ctrl+Alt+P
    Run, context-launcher.exe --oneshot
return
```

### Startup on Login
The installer provides an option to "Run at Windows startup". If you didn't select it during installation:

1. Press `Win+R`
2. Type `shell:startup` and press Enter
3. Create a shortcut to `context-launcher.exe` in that folder

## FAQ

**Q: Does this work with network drives?**
A: Yes, as long as the network drive is mounted and accessible in Explorer.

**Q: Can I launch apps that require parameters?**
A: Yes, use the `args` field in the config.

**Q: Does it work with Windows 11?**
A: Yes, fully compatible with Windows 10 and 11.

**Q: Can I use relative paths?**
A: Only if the executable is in your PATH. Otherwise use full paths.

**Q: How do I exit the listener?**
A: Right-click the system tray icon or use Task Manager.

## Contributing

Contributions welcome! Feel free to:
- Report bugs
- Suggest features
- Submit pull requests

Visit the [GitHub repository](https://github.com/agbrainerd/context-launcher) to contribute.

## Author

**Audrey Brainerd**
- GitHub: [@agbrainerd](https://github.com/agbrainerd)

## License

MIT License - feel free to use and modify as needed. See LICENSE.txt for details.

## Changelog

### v1.1.1
- Fixed a bug where config editor was not launching from task bar menu
- Added support for single-key shortcuts for e.g. F13-F24
- Made quality of life updates to config editor

### v1.1.0
- Config file now stored in AppData (no admin needed to edit settings)
- Improved first-run experience with automatic config editor launch
- Better hotkey conflict detection with detailed error messages
- Fixed double-click behavior in config editor
- Uninstaller now prompts to remove user settings
- Auto-detecting build scripts for easier compilation
- Support for both typing and recording hotkeys in config editor

### v1.0.0 (Initial Release)
- Context-aware directory detection (mouse hover + focused window)
- Multi-application support with configurable hotkeys
- INI configuration file
- Native Windows config editor GUI
- System tray icon with menu
- Enable/disable apps without deletion
- F-key and special key support
- Admin privilege support
- Background listener mode
