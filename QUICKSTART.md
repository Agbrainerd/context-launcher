# Context Launcher - Quick Start Guide

## Installation

### Using the Installer (Recommended)
1. Download `ContextLauncher-Setup-1.1.0.exe` from the [releases page](https://github.com/agbrainerd/context-launcher/releases)
2. Run the installer
3. The config editor will open automatically
4. Configure your applications and hotkeys
5. Click "Save & Start Launcher"

### Build from Source
1. Run `build-all.bat`
2. Both `context-launcher.exe` and `ConfigEditor.exe` will be created

## Configuration File Location

Your settings are stored in:
```
%APPDATA%\ContextLauncher\launcher.ini
```

This means:
- No admin access needed to edit settings
- Each user has their own configuration
- Settings persist across updates

## Default Configuration

On first install, only PowerShell (Ctrl+Alt+P) is enabled. Enable additional apps in the config editor:

- **Ctrl+Alt+P** - PowerShell
- **Ctrl+Alt+Shift+P** - PowerShell (Admin)
- **Ctrl+Alt+C** - Command Prompt
- **Ctrl+Alt+T** - Windows Terminal
- **Ctrl+Alt+V** - VS Code
- **Ctrl+Alt+G** - Git Bash

## Configuration Methods

### GUI Config Editor (Recommended)

**Advantages:**
- Visual interface with checkboxes and fields
- Hotkey recording (just press the keys)
- Input validation
- "Save & Start Launcher" button for immediate testing

**How to access:**
- Start Menu: Search for "Context Launcher Configuration Editor"
- Or run `ConfigEditor.exe` from the installation directory

### Manual INI Editing

**Advantages:**
- Quick for power users
- Version control friendly
- No GUI needed

**How to access:**
- Open `%APPDATA%\ContextLauncher\launcher.ini` in any text editor
- Right-click tray icon → "Reload Config" after saving changes

## How Directory Detection Works

When you press a hotkey:

1. **Check mouse position** - Is your cursor over a File Explorer window?
2. **Check focused window** - Is the focused window File Explorer?
3. **Use priority setting** - If both are true, the "Priority when both available" setting determines which is used
4. **Fallback** - If no Explorer window is found, use your home directory

This means:
- Hover your mouse over any Explorer window and press your hotkey
- Have Explorer focused and press your hotkey
- Press your hotkey anywhere and it opens in your home folder

## Adding Custom Applications

### Example: Python REPL in current directory
```ini
Python=python.exe|false||Ctrl+Alt+Y
```

### Example: Conda Environment
```ini
Conda Dev=cmd.exe|false|/k conda activate myenv|Ctrl+Alt+D
```

### Example: Custom Script
```ini
My Script=C:\Scripts\my-script.bat|false||Ctrl+Alt+M
```

## Troubleshooting

### "Failed to register hotkeys"
The error message will show which hotkeys failed. This means another application is already using those key combinations.

**Solutions:**
- Try different key combinations in the config editor
- Close conflicting applications (PowerToys, graphics driver shortcuts, etc.)

### App doesn't launch
- Verify the executable path in the config editor
- Use the full path if the app isn't in your system PATH
- Test the command manually in Command Prompt first

### Config changes not applied
- Right-click the system tray icon and select "Reload Config"
- Or restart the launcher

## Tips

- Use the "Save & Start Launcher" button in the config editor to test changes immediately
- The system tray icon menu has quick access to "Reload Config" and "Exit"
- You can temporarily disable apps using the checkboxes without deleting them
- F-keys (F1-F24) work great for hotkeys and rarely conflict with other software

## Additional Resources

For detailed documentation, see the full `README.md` included with the installation:
- Complete hotkey reference (all supported keys)
- Advanced configuration examples
- Integration with AutoHotkey and other tools
- Troubleshooting guide
 
