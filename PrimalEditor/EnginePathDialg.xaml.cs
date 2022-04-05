using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace PrimalEditor
{
    /// <summary>
    /// Interaction logic for EnginePathDialg.xaml
    /// </summary>
    public partial class EnginePathDialg : Window
    {
        public string PrimalPath { get; private set; }
        public EnginePathDialg()
        {
            InitializeComponent();
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
