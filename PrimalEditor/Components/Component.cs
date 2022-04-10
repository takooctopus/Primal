using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;

namespace PrimalEditor.Components
{
    /// <summary>
    /// 多选组件抽象接口，后面的多选组件类应该继承此类
    /// </summary>
    interface IMSComponent { }

    [DataContract]
    abstract class Component : ViewModelBase
    {
        /// <summary>
        /// 每个组件都有一个主游戏实体，是一对多关系
        /// </summary>
        [DataMember]
        public GameEntity Owner { get; private set; }

        /// <summary>
        /// 获取多选组件
        /// </summary>
        /// <param name="msEntity"></param>
        /// <returns></returns>
        public abstract IMSComponent GetMultiSelectionComponent(MSEntity msEntity);

        public abstract void WriteToBinary(BinaryWriter bw);

        public Component(GameEntity owner)
        {
            Debug.Assert(owner != null);
            Owner = owner;
        }
    }

    /// <summary>
    /// MSComponent多选组件类
    /// </summary>
    /// <typeparam name="T"></typeparam>
    abstract class MSComponent<T> : ViewModelBase, IMSComponent where T : Component
    {
        /// <summary>
        /// 是否允许更新，因为UpdateMSComponent()本质上也会更新视图，这时我们要实现一个mutex功能
        /// </summary>
        private bool _enableUpdates = true;

        /// <summary>
        /// 已选择的所有组件列表
        /// </summary>
        public List<T> SelectedComponents { get; }

        /// <summary>
        /// 用来更新组件里的属性的，传入参数比方说是transform的scale属性
        /// </summary>
        /// <param name="propertyName"></param>
        /// <returns></returns>
        protected abstract bool UpdateComponents(string propertyName);

        /// <summary>
        /// 获取更新多选组件状态
        /// </summary>
        /// <returns></returns>
        protected abstract bool UpdateMSComponent();

        /// <summary>
        /// 互斥地更新视图
        /// </summary>
        public void Refresh()
        {
            _enableUpdates = false;
            UpdateMSComponent();
            _enableUpdates = true;
        }
        public MSComponent(MSEntity mSEntity)
        {
            Debug.Assert(mSEntity?.SelectedEntities?.Any() == true);
            // 使用Select将集合展开
            SelectedComponents = mSEntity.SelectedEntities.Select(entity => entity.GetComponent<T>()).ToList();
            PropertyChanged += (s, e) =>
            {
                if (_enableUpdates) UpdateComponents(e.PropertyName);
            };
        }
        public MSComponent() { }
    }
}
