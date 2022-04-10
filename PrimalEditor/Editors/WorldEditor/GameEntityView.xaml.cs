using PrimalEditor.Components;
using PrimalEditor.GameProject;
using PrimalEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Input;

namespace PrimalEditor.Editors
{
    /// <summary>
    /// 将可选null的bool?转化成单纯bool的类
    /// 在绑定元素时，可选为null可能会导致错误
    /// </summary>
    /// <seealso cref="System.Windows.Data.IValueConverter" />
    public class NullableBoolToBoolConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value is bool b && b == true;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value is bool b && b == true;
        }
    }

    /// <summary>
    /// Interaction logic for GameEntityView.xaml
    /// 时刻注意在这里面的DataContext是一个多选实体类MSEntity
    /// </summary>
    public partial class GameEntityView : UserControl
    {
        private Action _undoAction;
        private string _propertyName;

        /// <summary>
        /// 单例模式，类变量里面存放一个指向自己的实例
        /// </summary>
        /// <value>
        /// The instance.
        /// </value>
        public static GameEntityView Instance { get; private set; }
        public GameEntityView()
        {
            InitializeComponent();
            DataContext = null;
            Instance = this;
            // 一旦DataContext有变化就执行lambda函数，记录下MSEntity里面哪一个prop[属性]发生了变化
            DataContextChanged += (_, __) =>
            {
                if (DataContext != null)
                {
                    (DataContext as MSEntity).PropertyChanged += (s, e) => _propertyName = e.PropertyName;
                }
            };
        }

        /// <summary>
        /// Gets the rename action.[将选中元素的名字全部重新设置成快照值]
        /// </summary>
        /// <returns></returns>
        private Action getRenameAction()
        {
            var vm = DataContext as MSEntity;
            // MSEntity[DataContext]中选中的实体的 实例和实例的名字 组一个tuple 再形成一个tuple列表 【相当于保存一个当前名称的快照】
            var selection = vm.SelectedEntities.Select(entity => (entity, entity.Name)).ToList();
            // 这个Action就是将选中元素的名字全部重新设置成快照值
            return new Action(() =>
            {
                selection.ForEach(item => item.entity.Name = item.Name);
                (DataContext as MSEntity).Refresh();
            });
        }

        /// <summary>
        /// Gets the rename action.[将选中元素的名字全部重新设置成快照值]
        /// </summary>
        /// <returns></returns>
        private Action getIsEnabledAction()
        {
            var vm = DataContext as MSEntity;
            // MSEntity[DataContext]中选中的实体的 实例和实例的可用状态 组一个tuple 再形成一个tuple列表 【相当于保存一个当前IsEnable的快照】
            var selection = vm.SelectedEntities.Select(entity => (entity, entity.IsEnabled)).ToList();
            // 这个Action就是将选中元素的可用状态全部重新设置成快照值
            return new Action(() =>
            {
                selection.ForEach(item => item.entity.IsEnabled = item.IsEnabled);
                (DataContext as MSEntity).Refresh();
            });
        }

        /// <summary>
        /// Called when [name text box got key board focus].
        /// 单击textbox的时候，就将现在的_propertyName重置，再拿到重设名称快照的Action
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="KeyboardFocusChangedEventArgs"/> instance containing the event data.</param>
        private void OnName_TextBox_GotKeyBoardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            _propertyName = string.Empty;
            _undoAction = getRenameAction();
        }

        /// <summary>
        /// Called when [name text box lost key board focus].
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="KeyboardFocusChangedEventArgs"/> instance containing the event data.</param>
        private void OnName_TextBox_LostKeyBoardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            // 如果DataContex的属性有更新[更新时会触动最开始的触发器设置_propertyName]，对比其是不是多选组件的名字属性【Name】
            // 还要判断 撤销动作 列表是不是为空
            if (_propertyName == nameof(MSEntity.Name) && _undoAction != null)
            {
                // 拿到一个设置成现在名称的快照Action【相当于重做】
                var redoAction = getRenameAction();
                // 将撤消重做两个动作加进 【全局】整个项目Object的UndoRedo属性中的 撤消重做列表
                Project.UndoRedo.Add(new UndoRedoAction(_undoAction, redoAction, "Rename game entity"));
                _propertyName = null;
            }
            _undoAction = null;
        }

        /// <summary>
        /// Called when [is enabled chekck box click].
        /// 就是打勾那个checkBox点的函数，还是一样的逻辑，一点全部都改
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void OnIsEnabled_ChekckBox_Click(object sender, RoutedEventArgs e)
        {
            // 先拿到初始快照
            var undoAction = getIsEnabledAction();
            var vm = DataContext as MSEntity;
            // 改变MSEntity里所有已选组件的IsChecked属性
            vm.IsEnabled = (sender as CheckBox).IsChecked == true;
            // 拿到改变后的快照
            var redoAction = getIsEnabledAction();
            Project.UndoRedo.Add(new UndoRedoAction(undoAction, redoAction,
                vm.IsEnabled == true ? "Enable game entity" : "Disable game entity"
                ));
        }

        /// <summary>
        /// Called when [add component button preview mouse LBD].
        /// 这个是路由事件，从上到下滑到那里就是了，起作用就是展开addComponentMenu这个ContextMenu
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void OnAddComponent_Button_PreviewMouse_LBD(object sender, MouseButtonEventArgs e)
        {
            var menu = FindResource("addComponentMenu") as ContextMenu;
            var btn = sender as ToggleButton;
            btn.IsChecked = true;
            menu.Placement = PlacementMode.Bottom;
            menu.PlacementTarget = btn;
            menu.MinWidth = btn.ActualWidth;
            menu.IsOpen = true;
        }

        /// <summary>
        /// 私有方法，用来[根据ComponentFactory::ComponentType这个枚举属性]添加组件
        /// </summary>
        /// <param name="componentType">Type of the component.</param>
        /// <param name="data">The data.</param>
        private void AddComponent(ComponentType componentType, object data)
        {
            var creationFunction = ComponentFactory.GetCreationFunction(componentType);
            var changedEntities = new List<(GameEntity entity, Component component)>();
            var vm = DataContext as MSEntity;
            foreach (var entity in vm.SelectedEntities)
            {
                // 对选中的游戏实体进行遍历，将我们
                // 注意在我们的模型中 一个游戏实体每种类型的组件component最多只有一个，要是已经有了就会跳过【弹一条警告】
                // 每个entity都要对应实例化一个组件
                var component = creationFunction(entity, data);
                // 将新生成的组件绑定到游戏实体里[加进去了就是true，已经有了就是false]
                if (entity.AddComponent(component))
                {
                    // 变了的我们就记录一下放到列表里面做一个快照，方便进行撤销和重做
                    changedEntities.Add((entity, component));
                }
                if (changedEntities.Any())
                {
                    vm.Refresh();
                    Project.UndoRedo.Add(new UndoRedoAction(
                        () =>
                        {
                            changedEntities.ForEach(x => x.entity.RemoveComponent(x.component));
                            (DataContext as MSEntity).Refresh();
                        },
                        () =>
                        {
                            changedEntities.ForEach(x => x.entity.AddComponent(x.component));
                            (DataContext as MSEntity).Refresh();
                        },
                        $"Add {componentType} component to selected entities."
                        ));
                }
            }
        }

        /// <summary>
        /// Called when [add script component].
        /// 点下添加组件按钮[togglebutton]下对应脚本名称时的添加函数
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void OnAddScriptComponent(object sender, RoutedEventArgs e)
        {
            AddComponent(ComponentType.Script, (sender as MenuItem).Header.ToString());
        }
    }
}
