using PrimalEditor.Content;
using PrimalEditor.GameDev;
using System.Windows;
using System.Windows.Controls;

namespace PrimalEditor.Editors
{
    /// <summary>
    /// Interaction logic for WorldEditorView.xaml
    /// </summary>
    public partial class WorldEditorView : UserControl
    {
        public WorldEditorView()
        {
            InitializeComponent();
            Loaded += OnWorldEditorViewLoaded;
        }

        private void OnWorldEditorViewLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnWorldEditorViewLoaded;
            Focus();

            // TODO: 修改焦点逻辑,因为下面的修改会导致全局快捷键失效，这个留着以后再弄
            // TOREMOVE: 修复对MSentityList里面复数选中时的背景失效问题，你看下面这个函数就是当返回null时，会重新焦点主界面，就是这个bug
            //((INotifyCollectionChanged)Project.UndoRedo.UndoList).CollectionChanged += (s, e) => Focus();

        }
        /// <summary>
        /// 新建脚本按钮点击事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnNewScript_Button_Click(object sender, RoutedEventArgs e)
        {
            var dlg = new NewScriptDialog();
            dlg.ShowDialog();
        }

        private void OnCreatePrimitiveMesh_Button_Click(object sender, RoutedEventArgs e)
        {
            var dlg = new PrimitiveMeshDialog();
            dlg.ShowDialog();
        }
    }
}
