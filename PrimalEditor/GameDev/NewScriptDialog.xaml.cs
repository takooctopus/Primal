using PrimalEditor.GameProject;
using PrimalEditor.Utilities;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;

namespace PrimalEditor.GameDev
{
    /// <summary>
    /// NewScriptDialog.xaml 的逻辑类【新建脚本类的对话框】
    /// </summary>
    public partial class NewScriptDialog : Window
    {
        /// <summary>
        /// 要放进项目的.cpp脚本文件模板
        /// </summary>
        private static readonly string _cppCode = @"
#include ""{0}.h""
namespace {1}
{{
    REGISTER_SCRIPT({0});
    void {0}::start(){{}}
    void {0}::update(float dt){{}}
}} // namespace {1}
";
        private static readonly string _hCode = @"
#pragma once
namespace {1}
{{
    class {0} : public primal::script::entity_script {{
	public:
		constexpr explicit {0}(primal::game_entity::entity entity)
		: primal::script::entity_script{{entity}} {{}}
        void start( ) override;
        void update(float dt) override;
    private:
	}};
}} // namespace {1}
";

        private static readonly string _namespace = GetNamespaceFromPropertyName();

        private static string GetNamespaceFromPropertyName()
        {
            var projectName = Project.Current.Name;
            Debug.Assert(!string.IsNullOrEmpty(projectName));
            projectName = projectName.Replace(' ', '_');
            return projectName;
        }

        private bool Validate()
        {
            bool isValid = false;
            var name = scriptNameBox.Text.Trim();
            var path = scriptPathBox.Text.Trim();
            string errMsg = string.Empty;
            if (string.IsNullOrEmpty(name))
            {
                errMsg = "Type in a script name.";
            }
            else if (name.IndexOfAny(Path.GetInvalidFileNameChars()) != -1 || name.Any(x => char.IsWhiteSpace(x)))
            {
                errMsg = "Invalid character(s) in script name.";
            }
            else if (string.IsNullOrEmpty(path))
            {
                errMsg = "Type in a path.";
            }
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                errMsg = "Invalid character(s) in script path.";
            }
            else if (!Path.GetFullPath(Path.Combine(Project.Current.Path, path)).Contains(Path.Combine(Project.Current.Path, @"GameCode\")))
            {
                errMsg = "Script must be addede to (a sub_folder of) GameCode.";
            }
            else if (File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(Project.Current.Path, path), $"{name}.cpp"))) ||
                File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(Project.Current.Path, path), $"{name}.h"))))
            {
                errMsg = $"Script {name} already exists in this folder.";
            }
            else
            {
                isValid = true;
            }

            if (!isValid)
            {
                messageTextBlock.Foreground = FindResource("Editor.RedBrush") as Brush;
            }
            else
            {
                messageTextBlock.Foreground = FindResource("Editor.FontBrush") as Brush;
            }

            messageTextBlock.Text = errMsg;

            return isValid;
        }

        private void OnScriptName_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (!Validate()) return;
            var name = scriptNameBox.Text.Trim();
            var project = Project.Current;
            messageTextBlock.Text = $"{name}.h and {name}.cpp will be added to {Project.Current.Name}";
        }


        private void OnScriptPath_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (!Validate()) return;
        }

        private async void OnCreate_Button_Click(object sender, RoutedEventArgs e)
        {
            if (!Validate()) return;
            IsEnabled = false;
            busyAnimation.Opacity = 0;
            busyAnimation.Visibility = Visibility.Visible;
            DoubleAnimation fadeIn = new DoubleAnimation(0, 1, new Duration(TimeSpan.FromMilliseconds(500)));
            busyAnimation.BeginAnimation(OpacityProperty, fadeIn);

            try
            {
                var name = scriptNameBox.Text.Trim();
                var path = Path.GetFullPath(Path.Combine(Project.Current.Path, scriptPathBox.Text.Trim()));
                var solution = Project.Current.Solution;
                var projectName = Project.Current.Name;
                await Task.Run(() => CreateScript(name, path, solution, projectName));
                Debug.WriteLine("Script created");
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, $"Failed to create script {scriptNameBox.Text}");
            }
            finally
            {
                DoubleAnimation fadeOut = new DoubleAnimation(1, 0, new Duration(TimeSpan.FromMilliseconds(200)));
                fadeOut.Completed += (s, e) =>
                {
                    busyAnimation.Visibility = Visibility.Hidden;
                    Close();
                };
                busyAnimation.BeginAnimation(OpacityProperty, fadeOut);

            }
        }

        private void CreateScript(string name, string path, string solution, string projectName)
        {
            if (!Directory.Exists(path)) Directory.CreateDirectory(path);
            var cpp = Path.GetFullPath(Path.Combine(path, $"{name}.cpp"));
            var h = Path.GetFullPath(Path.Combine(path, $"{name}.h"));

            using (var sw = File.CreateText(cpp))
            {
                sw.WriteLine(string.Format(_cppCode, name, _namespace));
            }

            using (var sw = File.CreateText(h))
            {
                sw.WriteLine(string.Format(_hCode, name, _namespace));
            }

            string[] files = new string[] { cpp, h };
            for (int i = 0; i < 3; ++i)
            {
                if (!VisualStudio.AddFilesToSolution(solution, projectName, files)) System.Threading.Thread.Sleep(1000);
                else break;
            }
        }
        public NewScriptDialog()
        {
            InitializeComponent();
            Owner = Application.Current.MainWindow;
            scriptPathBox.Text = @"GameCode\";
        }
    }
}
