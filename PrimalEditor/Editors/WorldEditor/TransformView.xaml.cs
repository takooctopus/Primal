using PrimalEditor.Components;
using PrimalEditor.GameProject;
using PrimalEditor.Utilities;
using System;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace PrimalEditor.Editors
{
    /// <summary>
    /// Interaction logic for TransformView.xaml
    /// </summary>
    public partial class TransformView : UserControl
    {
        private Action _undoAction = null;
        private bool _propertyChanged = false;

        public TransformView()
        {
            InitializeComponent();
            Loaded += OnTransformViewLoaded;
        }

        private void OnTransformViewLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnTransformViewLoaded;
            (DataContext as MSTransform).PropertyChanged += (sender, e) => _propertyChanged = true;
        }

        /// <summary>
        /// 作为一个泛用类，实现模板参数
        /// 参数有俩，一个下面选择的方法，以Position举例，(x) => (x, x.Position) 这个Func就是将选择的Item和其中的一个属性合并成一个tuple
        /// 另一个参数是Foreach循环里要使用的方法 (x) => x.transform.Position = x.Item2 就是在循环里要将x.transform.Position的值设置为之前取出的初始值，即tuple中的第二个参数
        /// </summary>
        /// <param name="selector"></param>
        /// <param name="forEachAction"></param>
        /// <returns></returns>
        private Action GetAction(Func<Transform, (Transform transform, Vector3)> selector,
            Action<(Transform transform, Vector3)> forEachAction)
        {
            if (!(DataContext is MSTransform vm))
            {
                _undoAction = null;
                _propertyChanged = false;
                return null;
            }
            // 将所有已选择的组件重新格式化为一个怎么说呢，类似于一个tuplelist吧，[把组件和组件的Position组合成一个tuple]，再将tuple组成list
            var selection = vm.SelectedComponents.Select(x => selector(x)).ToList();
            return new Action(() =>
            {
                selection.ForEach(x => forEachAction(x));
                (GameEntityView.Instance.DataContext as MSEntity)?.GetMSComponent<MSTransform>().Refresh();
            });
        }
        private Action GetPositionAction() => GetAction((x) => (x, x.Position), (x) => x.transform.Position = x.Item2);
        private Action GetRotationAction() => GetAction((x) => (x, x.Rotation), (x) => x.transform.Rotation = x.Item2);
        private Action GetScaleAction() => GetAction((x) => (x, x.Scale), (x) => x.transform.Scale = x.Item2);

        private void RecordActions(Action redoAction, string name)
        {
            if (_propertyChanged)
            {
                Debug.Assert(_undoAction != null);
                _propertyChanged = false;
                Project.UndoRedo.Add(new UndoRedoAction(_undoAction, redoAction, name));
            }
        }
        private void OnPosition_VectorBox_PreviewMouse_LBD(object sender, MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetPositionAction();
        }

        private void OnPosition_VectorBox_PreviewMouse_LBU(object sender, MouseButtonEventArgs e)
        {
            RecordActions(GetPositionAction(), "Position changed");
        }

        private void OnRotation_VectorBox_PreviewMouse_LBD(object sender, MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetRotationAction();
        }

        private void OnRotation_VectorBox_PreviewMouse_LBU(object sender, MouseButtonEventArgs e)
        {
            RecordActions(GetRotationAction(), "Rotation changed");
        }
        private void OnScale_VectorBox_PreviewMouse_LBD(object sender, MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetScaleAction();
        }

        private void OnScale_VectorBox_PreviewMouse_LBU(object sender, MouseButtonEventArgs e)
        {
            RecordActions(GetScaleAction(), "Scale changed");
        }

        private void OnPosition_VectorBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyChanged && _undoAction != null)
            {
                OnPosition_VectorBox_PreviewMouse_LBU(sender, null);
            }
        }
        private void OnRotation_VectorBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyChanged && _undoAction != null)
            {
                OnRotation_VectorBox_PreviewMouse_LBU(sender, null);
            }
        }
        private void OnScale_VectorBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyChanged && _undoAction != null)
            {
                OnScale_VectorBox_PreviewMouse_LBU(sender, null);
            }
        }
    }
}
