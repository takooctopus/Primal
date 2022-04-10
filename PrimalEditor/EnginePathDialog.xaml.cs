using System.IO;
using System.Windows;

namespace PrimalEditor
{
    /// <summary>
    /// 引擎路径的对话框
    /// </summary>
    public partial class EnginePathDialog : Window
    {
        public string PrimalPath { get; private set; }
        public EnginePathDialog()
        {
            InitializeComponent();
            // 要设置对话框的主人，关系到其初始化位置和焦点等
            Owner = Application.Current.MainWindow;
        }

        private void OnOk_Button_Click(object sender, RoutedEventArgs e)
        {
            var path = pathTextBox.Text;
            //messageTextBlock是视图里面定义的控件
            messageTextBlock.Text = string.Empty;
            if (string.IsNullOrEmpty(path))
            {
                messageTextBlock.Text = "Invalid path.";
            }
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                messageTextBlock.Text = "Invalid char(s) used in path.";
            }
            else if (!Directory.Exists(Path.Combine(path, @"Engine\EngineAPI")))
            {
                messageTextBlock.Text = "Unable to find the engine at the specific location.";
            }

            if (string.IsNullOrEmpty(messageTextBlock.Text))
            {
                if (!path.EndsWith(@"\")) path += @"\";
                PrimalPath = path;
                DialogResult = true;
                Close();
            }
        }
    }
}
