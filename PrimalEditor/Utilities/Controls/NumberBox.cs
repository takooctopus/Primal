using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace PrimalEditor.Utilities.Controls
{
    [TemplatePart(Name = "PART_textBlock", Type = typeof(TextBlock))]
    [TemplatePart(Name = "PART_textBox", Type = typeof(TextBox))]
    internal class NumberBox : Control
    {
        // 用来保存鼠标按下时的原始值
        private double _originalValue;
        // 用来表示是否捕获到了鼠标
        private bool _captured = false;
        // 用来表示拖动时里面的数值到底改变没有
        private bool _valueChanged = false;
        // 按下去时应该记录鼠标的初始位置方便去计算移动距离
        private double _mouseXStart;
        // 倍率，设置同时按下ctrl或者shift来控制滑动的速率
        private double _multiplier;

        public event RoutedEventHandler ValueChanged
        {
            add { AddHandler(ValueChangedEvent, value); }
            remove { RemoveHandler(ValueChangedEvent, value); }
        }
        public static RoutedEvent ValueChangedEvent =
            EventManager.RegisterRoutedEvent(nameof(ValueChanged), RoutingStrategy.Bubble,
                typeof(RoutedEventHandler), typeof(NumberBox));

        public string Value
        {
            get => (string)GetValue(ValueProperty);
            set => SetValue(ValueProperty, value);
        }
        public static readonly DependencyProperty ValueProperty = DependencyProperty.Register(nameof(Value), typeof(string), typeof(NumberBox), new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, new PropertyChangedCallback(OnValueChanged)));

        private static void OnValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as NumberBox).RaiseEvent(new RoutedEventArgs(ValueChangedEvent));
        }

        public double Multiplier
        {
            get => (double)GetValue(MultiplierProperty);
            set => SetValue(MultiplierProperty, value);
        }
        public static readonly DependencyProperty MultiplierProperty = DependencyProperty.Register(nameof(Multiplier), typeof(double), typeof(NumberBox), new PropertyMetadata(1.0));

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();
            // 有textblock就绑定几个函数
            if (GetTemplateChild("PART_textBlock") is TextBlock textBlock)
            {
                textBlock.MouseLeftButtonDown += OnTextBlock_Mouse_LBD;
                textBlock.MouseLeftButtonUp += OnTextBlock_Mouse_LBU;
                textBlock.MouseMove += OnTextBlock_Mouse_Move;
            }
        }

        private void OnTextBlock_Mouse_LBD(object sender, MouseButtonEventArgs e)
        {
            double.TryParse(Value, out _originalValue);
            Mouse.Capture(sender as UIElement);
            _captured = true;
            _valueChanged = false;
            e.Handled = true;

            // TODEBUG: 我TM就不明白这个GetPosition为什么返回来的X就是整数间隔的，就离谱。 明明Y那边都是好的，这边X就不是了!!!
            _mouseXStart = e.GetPosition(this).X;

            Focus();
        }
        private void OnTextBlock_Mouse_LBU(object sender, MouseButtonEventArgs e)
        {
            if (_captured)
            {
                Mouse.Capture(null);
                _captured = false;
                e.Handled = true;
                // 要是是一个点击事件[即你按下去弹起来并没有拖动改变值，那么我们就显示下面的输入文本框，并将里面的值全部选中]
                if (!_valueChanged && GetTemplateChild("PART_textBox") is TextBox textBox)
                {
                    textBox.Visibility = Visibility.Visible;
                    textBox.Focus();
                    textBox.SelectAll();
                }
            }
        }
        private void OnTextBlock_Mouse_Move(object sender, MouseEventArgs e)
        {
            if (_captured)
            {
                var mouseX = e.GetPosition(this).X;
                var d = mouseX - _mouseXStart;
                // 只要两者间距大于最小的系统拖拽间距
                if (Math.Abs(d) > SystemParameters.MinimumHorizontalDragDistance)
                {
                    if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control)) _multiplier = 0.001;
                    else if (Keyboard.Modifiers.HasFlag(ModifierKeys.Shift)) _multiplier = 0.1;
                    else _multiplier = 0.01;
                    var newValue = _originalValue + (d * _multiplier * Multiplier);
                    Value = newValue.ToString("G5");
                    _valueChanged = true;
                }
            }
        }

        static NumberBox()
        {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(NumberBox), new FrameworkPropertyMetadata(typeof(NumberBox)));
        }
    }
}
