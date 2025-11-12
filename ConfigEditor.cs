using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace ContextLauncherConfig
{
    public class ConfigEditor : Form
    {
        // Win32 API for sending messages to launcher
        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

        private const uint WM_USER = 0x0400;
        private const uint WM_RELOAD_CONFIG = WM_USER + 100;

        private ListView appListView;
        private Button addButton;
        private Button editButton;
        private Button deleteButton;
        private Button saveButton;
        private GroupBox settingsGroupBox;
        private CheckBox checkMouseHoverCheckBox;
        private CheckBox checkFocusedWindowCheckBox;
        private ComboBox priorityComboBox;
        private string configFilePath;
        private StatusStrip statusStrip;
        private ToolStripStatusLabel statusLabel;

        public ConfigEditor()
        {
            InitializeUI();
            LoadConfig();
        }

        private void InitializeUI()
        {
            this.Text = "Context Launcher - Configuration";
            this.Size = new Size(800, 600);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.MinimumSize = new Size(600, 400);

            // Settings GroupBox
            settingsGroupBox = new GroupBox
            {
                Text = "Detection Settings",
                Location = new Point(10, 10),
                Size = new Size(760, 120),
                Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right
            };

            checkMouseHoverCheckBox = new CheckBox
            {
                Text = "Check Mouse Hover Position",
                Location = new Point(10, 25),
                Size = new Size(250, 20),
                Checked = true
            };

            checkFocusedWindowCheckBox = new CheckBox
            {
                Text = "Check Focused Window",
                Location = new Point(10, 50),
                Size = new Size(250, 20),
                Checked = true
            };

            Label priorityLabel = new Label
            {
                Text = "Priority when both available:",
                Location = new Point(10, 80),
                Size = new Size(180, 20)
            };

            priorityComboBox = new ComboBox
            {
                Location = new Point(200, 77),
                Size = new Size(200, 25),
                DropDownStyle = ComboBoxStyle.DropDownList
            };
            priorityComboBox.Items.AddRange(new object[] { "Hover", "Focus" });
            priorityComboBox.SelectedIndex = 0;

            settingsGroupBox.Controls.Add(checkMouseHoverCheckBox);
            settingsGroupBox.Controls.Add(checkFocusedWindowCheckBox);
            settingsGroupBox.Controls.Add(priorityLabel);
            settingsGroupBox.Controls.Add(priorityComboBox);

            // ListView for applications
            appListView = new ListView
            {
                Location = new Point(10, 140),
                Size = new Size(760, 330),
                View = View.Details,
                FullRowSelect = true,
                GridLines = true,
                Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
                CheckBoxes = true
            };

            appListView.Columns.Add("Enabled", 60);
            appListView.Columns.Add("Name", 150);
            appListView.Columns.Add("Executable", 200);
            appListView.Columns.Add("Arguments", 100);
            appListView.Columns.Add("Hotkey", 120);
            appListView.Columns.Add("Admin", 60);

            // Track mouse state to prevent checkbox toggle on double-click
            bool isDoubleClickOnRow = false;

            appListView.MouseDown += (s, e) => {
                var hitTest = appListView.HitTest(e.Location);
                // Reset flag on single click
                if (e.Clicks == 1)
                {
                    isDoubleClickOnRow = false;
                }
                // Set flag on double-click anywhere except checkbox
                else if (e.Clicks == 2 && hitTest.Location != ListViewHitTestLocations.StateImage)
                {
                    isDoubleClickOnRow = true;
                }
            };

            appListView.ItemCheck += (s, e) => {
                // Cancel checkbox change if it's happening during a double-click on the row
                if (isDoubleClickOnRow)
                {
                    e.NewValue = e.CurrentValue;
                }
            };

            appListView.MouseDoubleClick += (s, e) => {
                var hitTest = appListView.HitTest(e.Location);
                // Only open edit dialog if not clicking on checkbox
                if (hitTest.Location != ListViewHitTestLocations.StateImage)
                {
                    EditSelectedApp();
                }
                // Reset flag after handling
                isDoubleClickOnRow = false;
            };

            // Status bar (replaces noisy MessageBox)
            statusStrip = new StatusStrip();
            statusLabel = new ToolStripStatusLabel("Ready")
            {
                Spring = true,
                TextAlign = ContentAlignment.MiddleLeft
            };
            statusStrip.Items.Add(statusLabel);

            // Buttons
            int buttonY = 480;
            addButton = new Button
            {
                Text = "Add Application",
                Location = new Point(10, buttonY),
                Size = new Size(120, 30),
                Anchor = AnchorStyles.Bottom | AnchorStyles.Left
            };
            addButton.Click += AddButton_Click;

            editButton = new Button
            {
                Text = "Edit",
                Location = new Point(140, buttonY),
                Size = new Size(80, 30),
                Anchor = AnchorStyles.Bottom | AnchorStyles.Left
            };
            editButton.Click += (s, e) => EditSelectedApp();

            deleteButton = new Button
            {
                Text = "Delete",
                Location = new Point(230, buttonY),
                Size = new Size(80, 30),
                Anchor = AnchorStyles.Bottom | AnchorStyles.Left
            };
            deleteButton.Click += DeleteButton_Click;

            saveButton = new Button
            {
                Text = "Save Configuration",
                Location = new Point(470, buttonY),
                Size = new Size(140, 30),
                Anchor = AnchorStyles.Bottom | AnchorStyles.Right,
                Font = new Font(this.Font, FontStyle.Bold)
            };
            saveButton.Click += SaveButton_Click;

            Button launchButton = new Button
            {
                Text = "Save && Start Launcher",
                Location = new Point(620, buttonY),
                Size = new Size(150, 30),
                Anchor = AnchorStyles.Bottom | AnchorStyles.Right,
                Font = new Font(this.Font, FontStyle.Bold),
                BackColor = System.Drawing.Color.FromArgb(102, 126, 234),
                ForeColor = System.Drawing.Color.White
            };
            launchButton.Click += LaunchButton_Click;

            // Add controls to form
            this.Controls.Add(settingsGroupBox);
            this.Controls.Add(appListView);
            this.Controls.Add(addButton);
            this.Controls.Add(editButton);
            this.Controls.Add(deleteButton);
            this.Controls.Add(saveButton);
            this.Controls.Add(launchButton);
            this.Controls.Add(statusStrip);
        }

        private void ShowStatus(string message, bool isError = false)
        {
            statusLabel.Text = message;
            statusLabel.ForeColor = isError ? Color.Red : Color.DarkGreen;
            
            // Auto-clear after 3 seconds
            System.Threading.Tasks.Task.Delay(3000).ContinueWith(_ => 
            {
                if (statusLabel.GetCurrentParent() != null)
                {
                    if (this.InvokeRequired)
                        this.Invoke(new Action(() => statusLabel.Text = "Ready"));
                    else
                        statusLabel.Text = "Ready";
                }
            });
        }

        private void NotifyLauncherToReload()
        {
            // Find the launcher window
            IntPtr hwnd = FindWindow("LauncherWindowClass", "Launcher");
            
            if (hwnd != IntPtr.Zero)
            {
                // Send reload message
                PostMessage(hwnd, WM_RELOAD_CONFIG, IntPtr.Zero, IntPtr.Zero);
            }
            // If launcher not running, that's okay - it will load fresh config when started
        }

        private void LoadConfig()
        {
            // Use AppData for config file location
            string appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
            string configDir = Path.Combine(appDataPath, "ContextLauncher");
            
            // Create directory if it doesn't exist
            if (!Directory.Exists(configDir))
            {
                Directory.CreateDirectory(configDir);
            }
            
            configFilePath = Path.Combine(configDir, "launcher.ini");

            if (File.Exists(configFilePath))
            {
                ParseConfig(configFilePath);
            }
            else
            {
                // Load defaults
                LoadDefaultApps();
            }
        }

        private void ParseConfig(string filePath)
        {
            try
            {
                string[] lines = File.ReadAllLines(filePath);
                string currentSection = "";

                foreach (string line in lines)
                {
                    string trimmed = line.Trim();

                    if (string.IsNullOrEmpty(trimmed) || trimmed.StartsWith(";") || trimmed.StartsWith("#"))
                        continue;

                    if (trimmed.StartsWith("[") && trimmed.EndsWith("]"))
                    {
                        currentSection = trimmed.Substring(1, trimmed.Length - 2);
                        continue;
                    }

                    if (!trimmed.Contains("="))
                        continue;

                    int equalsIndex = trimmed.IndexOf('=');
                    string key = trimmed.Substring(0, equalsIndex).Trim();
                    string value = trimmed.Substring(equalsIndex + 1).Trim();

                    if (currentSection == "Settings")
                    {
                        if (key == "checkMouseHover")
                            checkMouseHoverCheckBox.Checked = value.ToLower() == "true";
                        else if (key == "checkFocusedWindow")
                            checkFocusedWindowCheckBox.Checked = value.ToLower() == "true";
                        else if (key == "priorityWhenBothAvailable")
                            priorityComboBox.SelectedItem = char.ToUpper(value[0]) + value.Substring(1).ToLower();
                    }
                    else if (currentSection == "Apps")
                    {
                        string[] parts = value.Split('|');
                        if (parts.Length >= 4)
                        {
                            var item = new ListViewItem("");
                            item.SubItems.Add(key); // Name
                            item.SubItems.Add(parts[0]); // Executable
                            item.SubItems.Add(parts[2]); // Args
                            item.SubItems.Add(parts[3]); // Hotkey
                            item.SubItems.Add(parts[1].ToLower() == "true" ? "Yes" : "No"); // Admin
                            
                            bool enabled = parts.Length >= 5 ? parts[4].ToLower() == "true" : true;
                            item.Checked = enabled;

                            appListView.Items.Add(item);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ShowStatus($"Error loading config: {ex.Message}", true);
                LoadDefaultApps();
            }
        }

        private void LoadDefaultApps()
        {
            var defaults = new[]
            {
                new { Name = "PowerShell", Exe = "powershell.exe", Args = "", Hotkey = "Ctrl+Alt+P", Admin = false, Enabled = true },
                new { Name = "PowerShell Admin", Exe = "powershell.exe", Args = "", Hotkey = "Ctrl+Alt+Shift+P", Admin = true, Enabled = false },
                new { Name = "Command Prompt", Exe = "cmd.exe", Args = "", Hotkey = "Ctrl+Alt+C", Admin = false, Enabled = false },
                new { Name = "Windows Terminal", Exe = "wt.exe", Args = "", Hotkey = "Ctrl+Alt+T", Admin = false, Enabled = false },
                new { Name = "VS Code", Exe = "code", Args = ".", Hotkey = "Ctrl+Alt+V", Admin = false, Enabled = false }
            };

            foreach (var app in defaults)
            {
                var item = new ListViewItem("");
                item.SubItems.Add(app.Name);
                item.SubItems.Add(app.Exe);
                item.SubItems.Add(app.Args);
                item.SubItems.Add(app.Hotkey);
                item.SubItems.Add(app.Admin ? "Yes" : "No");
                item.Checked = app.Enabled;
                appListView.Items.Add(item);
            }
        }

        private void AddButton_Click(object sender, EventArgs e)
        {
            var dialog = new AppEditDialog();
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                var item = new ListViewItem("");
                item.SubItems.Add(dialog.AppName);
                item.SubItems.Add(dialog.Executable);
                item.SubItems.Add(dialog.Arguments);
                item.SubItems.Add(dialog.Hotkey);
                item.SubItems.Add(dialog.RunAsAdmin ? "Yes" : "No");
                item.Checked = dialog.Enabled;
                appListView.Items.Add(item);
            }
        }

        private void EditSelectedApp()
        {
            if (appListView.SelectedItems.Count == 0)
                return;

            var item = appListView.SelectedItems[0];
            var dialog = new AppEditDialog
            {
                AppName = item.SubItems[1].Text,
                Executable = item.SubItems[2].Text,
                Arguments = item.SubItems[3].Text,
                Hotkey = item.SubItems[4].Text,
                RunAsAdmin = item.SubItems[5].Text == "Yes",
                Enabled = item.Checked
            };

            if (dialog.ShowDialog() == DialogResult.OK)
            {
                item.SubItems[1].Text = dialog.AppName;
                item.SubItems[2].Text = dialog.Executable;
                item.SubItems[3].Text = dialog.Arguments;
                item.SubItems[4].Text = dialog.Hotkey;
                item.SubItems[5].Text = dialog.RunAsAdmin ? "Yes" : "No";
                item.Checked = dialog.Enabled;
            }
        }

        private void DeleteButton_Click(object sender, EventArgs e)
        {
            if (appListView.SelectedItems.Count == 0)
                return;

            if (MessageBox.Show("Delete selected application?", "Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
            {
                appListView.Items.Remove(appListView.SelectedItems[0]);
            }
        }

        private void SaveButton_Click(object sender, EventArgs e)
        {
            try
            {
                using (StreamWriter writer = new StreamWriter(configFilePath))
                {
                    writer.WriteLine("; Launcher Configuration File");
                    writer.WriteLine();
                    writer.WriteLine("[Settings]");
                    writer.WriteLine("; Enable/disable detection methods");
                    writer.WriteLine($"checkMouseHover={checkMouseHoverCheckBox.Checked.ToString().ToLower()}");
                    writer.WriteLine($"checkFocusedWindow={checkFocusedWindowCheckBox.Checked.ToString().ToLower()}");
                    writer.WriteLine();
                    writer.WriteLine("; Priority when both are available (hover or focus)");
                    writer.WriteLine("; hover = Mouse position takes precedence");
                    writer.WriteLine("; focus = Focused window takes precedence");
                    writer.WriteLine($"priorityWhenBothAvailable={priorityComboBox.SelectedItem.ToString().ToLower()}");
                    writer.WriteLine();
                    writer.WriteLine("[Apps]");
                    writer.WriteLine("; Format: name=executable|runAsAdmin|args|hotkey|enabled");
                    writer.WriteLine("; runAsAdmin: true or false");
                    writer.WriteLine("; args: additional command line arguments (use empty string if none)");
                    writer.WriteLine("; hotkey: e.g., Ctrl+Alt+P, Ctrl+Shift+C, etc.");
                    writer.WriteLine("; enabled: true or false (allows disabling apps without deleting them)");
                    writer.WriteLine();

                    foreach (ListViewItem item in appListView.Items)
                    {
                        string name = item.SubItems[1].Text;
                        string exe = item.SubItems[2].Text;
                        string args = item.SubItems[3].Text;
                        string hotkey = item.SubItems[4].Text;
                        bool admin = item.SubItems[5].Text == "Yes";
                        bool enabled = item.Checked;

                        writer.WriteLine($"{name}={exe}|{admin.ToString().ToLower()}|{args}|{hotkey}|{enabled.ToString().ToLower()}");
                    }
                }

                // Notify launcher to reload configuration
                NotifyLauncherToReload();
                
                // Show quiet status message instead of noisy MessageBox
                ShowStatus("Configuration saved and applied");
            }
            catch (Exception ex)
            {
                ShowStatus($"Error saving config: {ex.Message}", true);
            }
        }

        private void LaunchButton_Click(object sender, EventArgs e)
        {
            // First, save the configuration
            SaveButton_Click(sender, e);
            
            // Check if launcher is already running
            string launcherProcessName = "context-launcher";
            var existingProcesses = System.Diagnostics.Process.GetProcessesByName(launcherProcessName);
            
            if (existingProcesses.Length > 0)
            {
                var result = MessageBox.Show(
                    "Context Launcher is already running.\n\nDo you want to restart it to apply the new configuration?",
                    "Launcher Already Running",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Question);
                
                if (result == DialogResult.Yes)
                {
                    // Kill existing instances
                    foreach (var process in existingProcesses)
                    {
                        try
                        {
                            process.Kill();
                            process.WaitForExit(2000);
                        }
                        catch { }
                    }
                }
                else
                {
                    return; // User chose not to restart
                }
            }
            
            // Launch the application
            try
            {
                string exeDir = Path.GetDirectoryName(Application.ExecutablePath);
                string launcherPath = Path.Combine(exeDir, "context-launcher.exe");
                
                if (File.Exists(launcherPath))
                {
                    System.Diagnostics.Process.Start(launcherPath);
                    ShowStatus("Context Launcher started successfully");
                }
                else
                {
                    ShowStatus("Could not find context-launcher.exe", true);
                }
            }
            catch (Exception ex)
            {
                ShowStatus($"Error starting launcher: {ex.Message}", true);
            }
        }

        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new ConfigEditor());
        }
    }

    public class AppEditDialog : Form
    {
        private TextBox nameTextBox;
        private TextBox exeTextBox;
        private TextBox argsTextBox;
        private TextBox hotkeyTextBox;
        private CheckBox adminCheckBox;
        private CheckBox enabledCheckBox;
        private Button browseButton;

        public string AppName { get; set; }
        public string Executable { get; set; }
        public string Arguments { get; set; }
        public string Hotkey { get; set; }
        public bool RunAsAdmin { get; set; }
        public new bool Enabled { get; set; } = true;

        public AppEditDialog()
        {
            InitializeDialog();
        }

        private void InitializeDialog()
        {
            this.Text = "Edit Application";
            this.Size = new Size(500, 320);
            this.StartPosition = FormStartPosition.CenterParent;
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;

            int y = 20;

            // Name
            AddLabel("Application Name:", y);
            nameTextBox = AddTextBox(y);
            y += 40;

            // Executable
            AddLabel("Executable:", y);
            exeTextBox = AddTextBox(y);
            browseButton = new Button
            {
                Text = "Browse...",
                Location = new Point(370, y - 3),
                Size = new Size(80, 25)
            };
            browseButton.Click += BrowseButton_Click;
            this.Controls.Add(browseButton);
            y += 40;

            // Arguments
            AddLabel("Arguments:", y);
            argsTextBox = AddTextBox(y);
            y += 40;

            // Hotkey
            AddLabel("Hotkey:", y);
            hotkeyTextBox = AddTextBox(y);
            hotkeyTextBox.KeyDown += HotkeyTextBox_KeyDown;
            y += 40;

            // Checkboxes
            adminCheckBox = new CheckBox
            {
                Text = "Run as Administrator",
                Location = new Point(120, y),
                Size = new Size(200, 20)
            };
            this.Controls.Add(adminCheckBox);
            y += 25;

            enabledCheckBox = new CheckBox
            {
                Text = "Enabled",
                Location = new Point(120, y),
                Size = new Size(200, 20),
                Checked = true
            };
            this.Controls.Add(enabledCheckBox);
            y += 40;

            // OK/Cancel buttons
            Button okButton = new Button
            {
                Text = "OK",
                DialogResult = DialogResult.OK,
                Location = new Point(280, y),
                Size = new Size(80, 30)
            };
            okButton.Click += (s, e) => SaveValues();

            Button cancelButton = new Button
            {
                Text = "Cancel",
                DialogResult = DialogResult.Cancel,
                Location = new Point(370, y),
                Size = new Size(80, 30)
            };

            this.Controls.Add(okButton);
            this.Controls.Add(cancelButton);
            this.AcceptButton = okButton;
            this.CancelButton = cancelButton;
        }

        private Label AddLabel(string text, int y)
        {
            var label = new Label
            {
                Text = text,
                Location = new Point(20, y + 3),
                Size = new Size(100, 20)
            };
            this.Controls.Add(label);
            return label;
        }

        private TextBox AddTextBox(int y)
        {
            var textBox = new TextBox
            {
                Location = new Point(120, y),
                Size = new Size(240, 25)
            };
            this.Controls.Add(textBox);
            return textBox;
        }

        private void BrowseButton_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog dialog = new OpenFileDialog())
            {
                dialog.Filter = "Executable Files (*.exe)|*.exe|All Files (*.*)|*.*";
                if (dialog.ShowDialog() == DialogResult.OK)
                {
                    exeTextBox.Text = dialog.FileName;
                }
            }
        }

        private void HotkeyTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            // If user is typing normally (no modifiers), allow it
            if (!e.Control && !e.Alt && !e.Shift)
            {
                // Let them type normally
                return;
            }

            // Otherwise, capture the hotkey
            e.SuppressKeyPress = true;

            // Ignore modifier-only keys
            if (e.KeyCode == Keys.ControlKey || e.KeyCode == Keys.ShiftKey || 
                e.KeyCode == Keys.Alt || e.KeyCode == Keys.LWin || e.KeyCode == Keys.RWin)
                return;

            List<string> parts = new List<string>();
            if (e.Control) parts.Add("Ctrl");
            if (e.Alt) parts.Add("Alt");
            if (e.Shift) parts.Add("Shift");

            // Convert key to string
            string key = e.KeyCode.ToString();
            
            // Handle F-keys
            if (key.StartsWith("F") && key.Length <= 3)
            {
                parts.Add(key);
            }
            // Handle other special keys
            else if (key == "Delete" || key == "Insert" || key == "Home" || key == "End" ||
                     key == "PageUp" || key == "PageDown" || key == "Space" || key == "Tab")
            {
                parts.Add(key);
            }
            // Handle letter/number keys
            else if (key.Length == 1 || (key.Length == 2 && key.StartsWith("D")))
            {
                // D0-D9 are the number keys
                if (key.StartsWith("D"))
                    parts.Add(key.Substring(1));
                else
                    parts.Add(key);
            }
            else
            {
                parts.Add(key);
            }

            hotkeyTextBox.Text = string.Join("+", parts);
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            nameTextBox.Text = AppName ?? "";
            exeTextBox.Text = Executable ?? "";
            argsTextBox.Text = Arguments ?? "";
            hotkeyTextBox.Text = Hotkey ?? "";
            adminCheckBox.Checked = RunAsAdmin;
            enabledCheckBox.Checked = Enabled;
        }

        private void SaveValues()
        {
            AppName = nameTextBox.Text;
            Executable = exeTextBox.Text;
            Arguments = argsTextBox.Text;
            Hotkey = hotkeyTextBox.Text;
            RunAsAdmin = adminCheckBox.Checked;
            Enabled = enabledCheckBox.Checked;
        }
    }
}