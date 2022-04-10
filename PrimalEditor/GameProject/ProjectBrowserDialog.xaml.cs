using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Animation;

namespace PrimalEditor.GameProject
{
    /// <summary>
    /// Interaction logic for ProjectBrowserDialog.xaml
    /// </summary>
    public partial class ProjectBrowserDialog : Window
    {
        // 用来平滑动画的函数类，对于easing类可以去查文档
        private readonly CubicEase _easing = new CubicEase() { EasingMode = EasingMode.EaseInOut };
        public ProjectBrowserDialog()
        {
            InitializeComponent();
            Loaded += OnProjectBrowserDialogLoaded;
        }

        private void OnProjectBrowserDialogLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnProjectBrowserDialogLoaded;
            if (!OpenProject.Projects.Any())
            {
                openProjectButton.IsEnabled = false;
                openProjectView.Visibility = Visibility.Hidden;
                OnToggleButton_Click(createProjectButton, new RoutedEventArgs());
            }
        }

        private void AnimateToCreateProject()
        {
            var highlightAnimation = new DoubleAnimation(200, 400, new Duration(TimeSpan.FromSeconds(0.2)));
            highlightAnimation.EasingFunction = _easing;
            //highlightAnimation.Completed += (s, e) =>
            {
                var animation = new ThicknessAnimation(new Thickness(0, 0, 0, 0), new Thickness(-1600, 0, 0, 0), new Duration(TimeSpan.FromSeconds(0.5)));
                animation.EasingFunction = _easing;
                browserContent.BeginAnimation(MarginProperty, animation);
            };
            highlightRect.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        }

        private void AnimateToOpenProject()
        {
            var highlightAnimation = new DoubleAnimation(400, 200, new Duration(TimeSpan.FromSeconds(0.2)));
            highlightAnimation.EasingFunction = _easing;
            // 在highlightAnimation执行结束后继续执行的lamda函数
            //highlightAnimation.Completed += (s, e) =>
            {
                var animation = new ThicknessAnimation(new Thickness(-1600, 0, 0, 0), new Thickness(0, 0, 0, 0), new Duration(TimeSpan.FromSeconds(0.5)));
                animation.EasingFunction = _easing;
                // 这个是viewmodel里面的组件名称
                browserContent.BeginAnimation(MarginProperty, animation);
            };
            // 这个是打光的那个长方形组件
            highlightRect.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        }

        private void OnToggleButton_Click(object sender, RoutedEventArgs e)
        {
            if (sender == openProjectButton)
            {
                if (createProjectButton.IsChecked == true)
                {
                    createProjectButton.IsChecked = false;
                    // 加入滑动动画到OpenProjectView那边[毕竟两个本质上是左右平移视图，看不见就是不存在]
                    AnimateToOpenProject();
                    openProjectView.IsEnabled = true;
                    newProjectView.IsEnabled = false;
                }
                openProjectButton.IsChecked = true;
            }
            else
            {
                if (openProjectButton.IsChecked == true)
                {
                    openProjectButton.IsChecked = false;
                    // 加入滑动动画到CreateProjecttView那边[毕竟两个本质上是左右平移视图，看不见就是不存在]
                    AnimateToCreateProject();
                    openProjectView.IsEnabled = false;
                    newProjectView.IsEnabled = true;
                }
                createProjectButton.IsChecked = true;
            }
        }


    }
}
