using System.Windows;
using System.Windows.Controls;

namespace PrimalEditor.Utilities
{
    /// <summary>
    /// Interaction logic for LoggerView.xaml
    /// </summary>
    public partial class LoggerView : UserControl
    {
        public LoggerView()
        {
            InitializeComponent();
            // TODEL: Test
            //Loaded += (s, e) =>
            //{
            //    Logger.Log(MessageType.Info, $"Log Info Test");
            //    Logger.Log(MessageType.Warning, $"Log Warning Test");
            //    Logger.Log(MessageType.Error, $"Log Error Test");
            //};
        }

        private void OnClear_Button_Click(object sender, RoutedEventArgs e)
        {
            Logger.Clear();
        }

        private void OnMessageFilter_Button_Click(object sender, RoutedEventArgs e)
        {
            var filter = 0x0;
            if (toggleInfos.IsChecked == true) filter |= (int)MessageType.Info;
            if (toggleWarnings.IsChecked == true) filter |= (int)MessageType.Warning;
            if (toggleErrors.IsChecked == true) filter |= (int)MessageType.Error;
            Logger.SetMessageFilter(filter);
        }
    }
}
