using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace PrimalEditor.GameProject
{
    /// <summary>
    /// Interaction logic for OpenProjectView.xaml
    /// </summary>
    public partial class OpenProjectView : UserControl
    {
        public OpenProjectView()
        {
            InitializeComponent();
        }
        private void OnOpen_Button_click(object sender, RoutedEventArgs e)
        {
            OpenSelectedProject();

            Loaded += (s, e) =>
            {
                var item = projectsListBox.ItemContainerGenerator.ContainerFromIndex(projectsListBox.SelectedIndex) as ListBoxItem;
                item?.Focus();
            };
        }

        private void OnListBoxItem_Mouse_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            OpenSelectedProject();
        }

        /// <summary>
        /// 打开选中的项目函数
        /// </summary>
        private void OpenSelectedProject()
        {
            var project = OpenProject.Open(projectsListBox.SelectedItem as ProjectData);
            var dialogResult = false;
            var win = Window.GetWindow(this);
            if (project != null)
            {
                dialogResult = true;
                win.DataContext = project;
            }
            win.DialogResult = dialogResult;
            win.Close();
        }
    }
}
