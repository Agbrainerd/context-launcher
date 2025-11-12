# Building the Config Editor

## Quick Build (Easiest)

Just run the build script:

```batch
build-config-editor.bat
```

That's it! You'll get `ConfigEditor.exe`.

## Requirements

- .NET Framework 4.0 or later (comes with Windows)
- Visual Studio (for the C# compiler)

If you have Visual Studio installed, you already have everything you need.

## Manual Build (if you really want to)

If the batch file doesn't work, you can compile manually:

```batch
C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe /target:winexe /out:ConfigEditor.exe /r:System.dll /r:System.Windows.Forms.dll /r:System.Drawing.dll ConfigEditor.cs
```

## What You Get

A native Windows app (`ConfigEditor.exe`) that:
- Reads and writes `launcher.ini` in `%APPDATA%\ContextLauncher\`
- Has proper Windows controls
- Allows hotkey capture by pressing keys
- Browse button to find executables
- Checkbox to enable/disable apps
- Saves directly to INI file

## For Distribution

Add `ConfigEditor.exe` to your installer so users get it with the launcher.

