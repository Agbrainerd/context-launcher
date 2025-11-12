# Building the Installer

## Prerequisites

1. **Download and install Inno Setup:**
   - Go to: https://jrsoftware.org/isinfo.php
   - Download and install the latest version (6.3.3 or newer)

## Steps to Build

### 1. Prepare Your Files

Make sure you have these files in a folder:
```
project-folder/
├── context-launcher.exe  (compiled executable)
├── ConfigEditor.exe      (config editor)
├── README.md            (documentation)
├── LICENSE.txt          (MIT license)
└── installer-script.iss (Inno Setup script)
```

Note: launcher.ini is NOT included in the installer - it's created in %APPDATA% on first run.

### 2. Verify LICENSE.txt

The LICENSE.txt file should already be correct:

```
MIT License

Copyright (c) 2025 Audrey Brainerd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### 3. Generate a Unique GUID (First Time Only)

**To generate a GUID:**
1. Open `installer-script.iss` in Inno Setup
2. Find the line: `AppId={{YOUR-GUID-HERE}}`
3. Click on that line, then go to Tools → Generate GUID
4. The GUID will be inserted automatically
5. Save the file

**IMPORTANT:** Once you set a GUID, never change it! Windows uses this to identify your app for updates.

### 4. Verify Settings in installer-script.iss

The installer should already have the correct settings:

```iss
#define MyAppName "Context Launcher"
#define MyAppVersion "1.1.0"
#define MyAppPublisher "Audrey Brainerd"
#define MyAppURL "https://github.com/agbrainerd/context-launcher"
```

Only change MyAppVersion when releasing new versions.

### 5. Compile the Installer

**Option A: Using Inno Setup IDE (Recommended)**
1. Right-click on `installer-script.iss`
2. Select "Compile"
3. The installer will be created in the `output/` folder as:
   `ContextLauncher-Setup-1.1.0.exe`

**Option B: Command Line**
```batch
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer-script.iss
```

### 6. Find Your Installer

After compilation:
```
output/
└── ContextLauncher-Setup-1.1.0.exe
```

This is the distributable installer.

## What the Installer Does

When users run the installer:

1. **Welcome screen** - Shows application information
2. **License agreement** - Displays LICENSE.txt
3. **Choose install location** - Default: `C:\Program Files\ContextLauncher`
4. **Select options** - Desktop icon, startup on login
5. **Install files** - Copies executables and documentation
6. **Finish** - Options to launch config editor or start the launcher

## Testing the Installer

Before distributing:

1. Test on a clean system or virtual machine
2. Verify all files are installed correctly
3. Test that shortcuts work
4. Run the uninstaller and verify cleanup
5. Test upgrade by installing a new version over an old one

## Distribution

### GitHub Releases

1. Go to the GitHub repository
2. Click "Releases" → "Create a new release"
3. Tag version: `v1.1.0`
4. Title: `Context Launcher v1.1.0`
5. Upload: `ContextLauncher-Setup-1.1.0.exe`
6. Write release notes describing changes
7. Publish release

Users can then download the installer directly from the releases page.

## Troubleshooting

**Error: "File not found"**
- Ensure all files referenced in the `[Files]` section exist
- Check that `context-launcher.exe` and `ConfigEditor.exe` are compiled

**Installer doesn't run**
- Check if Windows Defender or antivirus blocked it
- Ensure you're using Inno Setup 6.3.3 or newer

**Uninstaller doesn't remove config**
- This is intentional - the uninstaller prompts whether to keep or remove user settings
- Config stored in AppData is preserved unless the user chooses to remove it

The installer is fully portable - users just download and run the .exe file!
