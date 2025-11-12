; Inno Setup Script for Context Launcher
; Download Inno Setup from: https://jrsoftware.org/isinfo.php

#define MyAppName "Context Launcher"
#define MyAppVersion "1.1.0"
#define MyAppPublisher "Audrey Brainerd"
#define MyAppURL "https://github.com/agbrainerd/context-launcher"
#define MyAppExeName "context-launcher.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
AppId={{81A57DAF-FAD6-4A5D-A496-89A5E0465522}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\ContextLauncher
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=LICENSE.txt
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=output
OutputBaseFilename=ContextLauncher-Setup-{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode
Name: "startupicon"; Description: "Run at Windows startup"; GroupDescription: "Startup Options:"; Flags: unchecked

[Files]
Source: "context-launcher.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "ConfigEditor.exe"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
; launcher.ini is now stored in %APPDATA%\ContextLauncher\ and created on first run

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Configuration Editor"; Filename: "{app}\ConfigEditor.exe"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Registry]
; Add to startup if user selected that option
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "ContextLauncher"; ValueData: """{app}\{#MyAppExeName}"""; Flags: uninsdeletevalue; Tasks: startupicon

[Run]
Filename: "{app}\ConfigEditor.exe"; Description: "Open Configuration Editor"; Flags: postinstall shellexec skipifsilent nowait
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Parameters: "--setup"; Flags: postinstall nowait skipifsilent unchecked

[Code]
function InitializeSetup(): Boolean;
var
  ResultCode: Integer;
  LauncherRunning: Boolean;
begin
  Result := True;
  
  // Check if context-launcher.exe is running
  LauncherRunning := False;
  if Exec('tasklist.exe', '/FI "IMAGENAME eq context-launcher.exe" /NH', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    LauncherRunning := (ResultCode = 0);
  end;
  
  if LauncherRunning then
  begin
    if MsgBox('Context Launcher is currently running.' + #13#10 + #13#10 + 
              'The installer needs to close it to continue.' + #13#10 + #13#10 +
              'Click OK to close the launcher and continue installation, or Cancel to exit.',
              mbConfirmation, MB_OKCANCEL) = IDOK then
    begin
      // Kill the process
      Exec('taskkill.exe', '/F /IM context-launcher.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
      Sleep(500); // Give it time to close
      Result := True;
    end
    else
    begin
      Result := False; // Cancel installation
    end;
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ConfigDir: string;
  ResultCode: Integer;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    ConfigDir := ExpandConstant('{userappdata}\ContextLauncher');
    
    if DirExists(ConfigDir) then
    begin
      if MsgBox('Do you want to remove your configuration settings?' + #13#10 + #13#10 + 
                'Selecting "Yes" will delete:' + #13#10 + 
                ConfigDir + '\launcher.ini' + #13#10 + #13#10 +
                'Selecting "No" will preserve your settings if you reinstall later.',
                mbConfirmation, MB_YESNO) = IDYES then
      begin
        DelTree(ConfigDir, True, True, True);
      end;
    end;
  end;
end;
