using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;

namespace PrimalEditor.Editors
{
    [ContentProperty("ComponentContent")]
    /// <summary>
    /// Interaction logic for ComponentView.xaml
    /// </summary>
    public partial class ComponentView : UserControl
    {
        /// <summary>
        /// dependency property Header, 用来绑定 Expander 中的 Header
        /// </summary>
        public string Header
        {
            get { return (string)GetValue(HeaderProperty); }
            set { SetValue(HeaderProperty, value); }
        }
        public static readonly DependencyProperty HeaderProperty =
            DependencyProperty.Register(nameof(Header), typeof(string), typeof(ComponentView));


        /// <summary>
        /// dependency property Header, 用来绑定 Expander 中的 ContentPresenter
        /// </summary>
        public FrameworkElement ComponentContent
        {
            get { return (FrameworkElement)GetValue(ComponentContentProperty); }
            set { SetValue(ComponentContentProperty, value); }
        }
        public static readonly DependencyProperty ComponentContentProperty =
            DependencyProperty.Register(nameof(ComponentContent), typeof(FrameworkElement), typeof(ComponentView));



        public ComponentView()
        {
            InitializeComponent();
        }
    }
}
