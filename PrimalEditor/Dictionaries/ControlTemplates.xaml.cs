using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace PrimalEditor.Dictionaries
{
    public partial class ControlTemplates : ResourceDictionary
    {
        // TextBox *********************************************************************************************************************************

        private void OnTextBox_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            var textBox = sender as TextBox;
            var exp = textBox.GetBindingExpression(TextBox.TextProperty);
            if (exp == null) return;
            if (e.Key == Key.Enter)
            {
                if (textBox.Tag is ICommand command && command.CanExecute(textBox.Text))
                {
                    command.Execute(textBox.Text);
                }
                else
                {
                    exp.UpdateSource();
                }
                Keyboard.ClearFocus();
                e.Handled = true;
            }
            else if (e.Key == Key.Escape)
            {
                exp.UpdateTarget();
                Keyboard.ClearFocus();
            }
        }

        private void OnTextBoxRename_KeyDown(object sender, KeyEventArgs e)
        {
            var textBox = sender as TextBox;
            var exp = textBox.GetBindingExpression(TextBox.TextProperty);
            if (exp == null) return;
            if (e.Key == Key.Enter)
            {
                if (textBox.Tag is ICommand command && command.CanExecute(textBox.Text))
                {
                    command.Execute(textBox.Text);
                }
                else
                {
                    exp.UpdateSource();
                }
                textBox.Visibility = Visibility.Collapsed;
                e.Handled = true;
            }
            else if (e.Key == Key.Escape)
            {
                exp.UpdateTarget();
                textBox.Visibility = Visibility.Collapsed;
            }
        }

        private void OnTextBoxRename_LostFocus(object sender, RoutedEventArgs e)
        {
            var textBox = sender as TextBox;
            if (!textBox.IsVisible) return;
            var exp = textBox.GetBindingExpression(TextBox.TextProperty);
            // 一旦失去焦点，我们就将值设置成原来的值，并将焦点目标设置成之前的焦点目标控件
            if (exp != null)
            {
                exp.UpdateTarget();
                //textBox.MoveFocus(new TraversalRequest(FocusNavigationDirection.Previous));
                textBox.Visibility = Visibility.Collapsed;
            }
        }


        // Button *********************************************************************************************************************************

        private void OnClose_Button_Click(object sender, RoutedEventArgs e)
        {
            var window = ((sender as FrameworkElement).TemplatedParent) as Window;
            //下面这种写法也是一样的
            //var window = (Window)((FrameworkElement)sender).TemplatedParent;
            window.Close();
        }

        private void OnMaximizeRestore_Button_Click(object sender, RoutedEventArgs e)
        {
            var window = ((sender as FrameworkElement).TemplatedParent) as Window;
            window.WindowState = window.WindowState == WindowState.Normal ? WindowState.Maximized : WindowState.Normal;
        }

        private void OnMinimize_Button_Click(object sender, RoutedEventArgs e)
        {
            var window = ((sender as FrameworkElement).TemplatedParent) as Window;
            window.WindowState = WindowState.Minimized;
        }

    }
}
