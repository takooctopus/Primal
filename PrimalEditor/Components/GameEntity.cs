using PrimalEditor.DllWrappers;
using PrimalEditor.GameProject;
using PrimalEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;

namespace PrimalEditor.Components
{
    [DataContract]
    [KnownType(typeof(Transform))]
    [KnownType(typeof(Script))]
    class GameEntity : ViewModelBase
    {
        /// <summary>
        /// 这里的EntityId设置是从c++引擎中传回的，对应的是实体在C++引擎创建的实体entity，但是传回的其实只有entity._id，毕竟现在里面只有一个_id属性.
        /// </summary>
        private int _entityId = ID.INVALID_ID;
        public int EntityId
        {
            get => _entityId;
            set
            {
                if (_entityId != value)
                {
                    _entityId = value;
                    OnPropertyChanged(nameof(EntityId));
                }
            }
        }

        /// <summary>
        /// 游戏实体当前是否处于活动状态
        /// 特别的，当我们更改IsActive的值时，会在c++引擎中创建或移除对应实体
        /// </summary>
        private bool _isActive;
        /// <summary>
        /// setter在设置_isActive值时，只有值改变才进行操作
        /// 激活状态F=>T时，在游戏引擎中创建这个实体
        /// 激活状态T=>F时，判断这个id是不是非法的，是非法的就移除
        /// </summary>
        /// <value>
        ///   <c>true</c> if this instance is active; otherwise, <c>false</c>.
        /// </value>
        public bool IsActive
        {
            get => _isActive;
            set
            {
                // 只有值改变时才进行操作
                if (_isActive != value)
                {
                    _isActive = value;
                    if (_isActive)
                    {
                        // 如果激活状态为F=>T，就添加这个实体
                        EntityId = EngineAPI.EntityAPI.CreateGameEntity(this);
                        Debug.Assert(ID.IsValid(_entityId));
                    }
                    else if (ID.IsValid(_entityId))
                    {
                        // 激活状态T=>F且当前实体id非法，从引擎中移除这个实体，并将当前的C#中的实体对象所属EntityId设置成非法
                        EngineAPI.EntityAPI.RemoveGameEntity(this);
                        EntityId = ID.INVALID_ID;
                    }
                    OnPropertyChanged(nameof(IsActive));
                }
            }
        }

        /// <summary>
        /// 当前是否可操作
        /// </summary>
        private bool _isEnabled = true;
        [DataMember]
        public bool IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }

        /// <summary>
        /// 组件名称，一个组件当然得有一个名字
        /// </summary>
        private string _name;
        [DataMember]
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        /// <summary>
        /// 组件对应的父场景，同样的是一对多关系
        /// </summary>
        [DataMember]
        public Scene ParentScene { get; private set; }

        /// <summary>
        /// 实体对应的所有组件视图集合，这个要放下去序列化的
        /// </summary>
        [DataMember(Name = nameof(Components))]
        private readonly ObservableCollection<Component> _components = new ObservableCollection<Component>();
        public ReadOnlyObservableCollection<Component> Components { get; private set; }

        /// <summary>
        /// 获取组件Component接口，这里只获取了一个
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        public Component GetComponent(Type type) => Components.FirstOrDefault(c => c.GetType() == type);

        /// <summary>
        /// 相当于强制转换类型，将获取的Component进行类型转换
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public T GetComponent<T>() where T : Component => GetComponent(typeof(T)) as T;

        /// <summary>
        /// 实体中添加组件函数[注意在我们的模型中 一个游戏实体每种类型的组件component最多只有一个，如果出现了重复的，会跳过]
        /// </summary>
        /// <param name="component">The component.</param>
        /// <returns></returns>
        public bool AddComponent(Component component)
        {
            Debug.Assert(component != null);
            if (!Components.Any(x => x.GetType() == component.GetType()))
            {
                IsActive = false;
                _components.Add(component);
                IsActive = true;
                return true;
            }
            Logger.Log(MessageType.Warning, $"Entity {Name} already has a {component.GetType().Name} component.");
            return false;
        }

        /// <summary>
        /// 游戏实体中移除组件
        /// </summary>
        /// <param name="component">The component.</param>
        public void RemoveComponent(Component component)
        {
            Debug.Assert(component != null);
            if (component is Transform) return; //坐标变换类是每个都有的，不能删除
            if (_components.Contains(component))
            {
                IsActive = false;
                _components.Remove(component);
                IsActive = true;
            }
        }

        /// <summary>
        /// 在反序列化时需要进行的操作，初始化创建游戏实体的组件集合
        /// </summary>
        /// <param name="context"></param>
        [OnDeserialized]
        void OnDeserialized(StreamingContext context)
        {
            Debug.Assert(_components != null);
            if (_components != null)
            {
                Components = new ReadOnlyObservableCollection<Component>(_components);
                OnPropertyChanged(nameof(Components));
            }

        }

        public GameEntity(Scene scene)
        {
            Debug.Assert(scene != null);
            ParentScene = scene;
            _components.Add(new Transform(this));
            OnDeserialized(new StreamingContext());
        }

    }

    /// <summary>
    /// 多选实体类
    /// </summary>
    abstract class MSEntity : ViewModelBase
    {
        /// <summary>
        /// 是否允许更新，用来互斥更新模型数据
        /// </summary>
        private bool _enableUpdates = true;

        /// <summary>
        /// 是否可以操作
        /// 注意这里bool是可以为null的，这里这样选择是因为集合的原因，身为集合很可能下面的gameEntity._isEnable是不统一的，null就表示了这种不统一的特性
        /// </summary>
        private bool? _isEnabled = true;
        public bool? IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }

        /// <summary>
        /// 集合中所有游戏实体的共有名称，名称不同则为空
        /// </summary>
        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        /// <summary>
        /// 组件集合
        /// </summary>
        private readonly ObservableCollection<IMSComponent> _components = new ObservableCollection<IMSComponent>();
        /// <summary>
        /// 这个是和_components属性绑定的，相比于前者，Components可以在.xaml文件中使用，但前提是你得指明DataContext
        /// </summary>
        /// <value>
        /// The components.
        /// </value>
        public ReadOnlyObservableCollection<IMSComponent> Components { get; }

        /// <summary>
        /// MSEntity类获取Component的函数，注意这个T是继承于IMSComponent的，反正返回特定类型Component 一个
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public T GetMSComponent<T>() where T : IMSComponent
        {
            return (T)Components.FirstOrDefault(x => x.GetType() == typeof(T));
        }

        /// <summary>
        /// 已经选择的游戏实体列表
        /// </summary>
        public List<GameEntity> SelectedEntities { get; }

        private void MakeComponentList()
        {
            _components.Clear();
            var firstEntity = SelectedEntities.FirstOrDefault();
            if (firstEntity == null) return;

            foreach (var component in firstEntity.Components)
            {
                var type = component.GetType();
                // 如果对于集合中， 其中有游戏实体(它根据类型获取的组件为空)，即它不包含此种组件  【使用！表示集合中没有这种不合群的游戏实体】 
                if (!SelectedEntities.Skip(1).Any(entity => entity.GetComponent(type) == null))
                {
                    // 我们断言(组件集合中找不到这种类型的组件)
                    Debug.Assert(Components.FirstOrDefault(x => x.GetType() == type) == null);
                    _components.Add(component.GetMultiSelectionComponent(this));
                }
            }
        }

        /// <summary>
        /// 获取固定类型集合中的的混合属性 对于可null的float值获取 混合数据为null，否则就是集合单一值
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="objects"></param>
        /// <param name="getProperty"></param>
        /// <returns></returns>
        public static float? GetMixedValue<T>(List<T> objects, Func<T, float> getProperty)
        {
            var value = getProperty(objects.First());
            // List集合中跳过第一个，如果有任何的值不相似于第一个值，返回null， 否则所有值都是一样的，返回集合中的共有属性值
            return objects.Skip(1).Any(x => !getProperty(x).IsTheSameAs(value)) ? (float?)null : value;
        }

        /// <summary>
        /// 获取固定类型集合中的的混合属性 对于可null的bool值获取
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="objects"></param>
        /// <param name="getProperty"></param>
        /// <returns></returns>
        public static bool? GetMixedValue<T>(List<T> objects, Func<T, bool> getProperty)
        {
            var value = getProperty(objects.First());
            // List集合中跳过第一个，如果有任何的值不等于第一个值，返回null， 否则所有值都是一样的，返回集合中的共有属性值
            return objects.Skip(1).Any(x => getProperty(x) != value) ? (bool?)null : value;
        }

        /// <summary>
        /// 获取固定类型集合中的的混合属性 对于string[因为string本身就是可null的]的获取
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="objects"></param>
        /// <param name="getProperty"></param>
        /// <returns></returns>
        public static string GetMixedValue<T>(List<T> objects, Func<T, string> getProperty)
        {
            var value = getProperty(objects.First());
            // List集合中跳过第一个，如果有任何的值不等于第一个值，返回null， 否则所有值都是一样的，返回集合中的共有属性值
            return objects.Skip(1).Any(x => getProperty(x) != value) ? null : value;
        }

        /// <summary>
        /// 更新游戏实体集合属性的方法，将集合中的所有实体的对应属性设置成我们现在选择的值
        /// </summary>
        /// <param name="propertyName"></param>
        /// <returns></returns>
        protected virtual bool UpdateGameEntities(string propertyName)
        {
            // 我们将这个方法设置成虚函数，是因为这个是基类，下面派生的MSGameEntity有自己的属性，每个都不一样，得重写
            switch (propertyName)
            {
                case nameof(IsEnabled): SelectedEntities.ForEach(x => x.IsEnabled = IsEnabled.Value); return true;
                case nameof(Name): SelectedEntities.ForEach(x => x.Name = Name); return true;
            }
            return false;
        }

        /// <summary>
        /// 从集合中获取我们需要的属性值，注意包含不同值时可能返回null
        /// </summary>
        /// <returns></returns>
        protected virtual bool UpdateMSGameEntity()
        {
            IsEnabled = GetMixedValue(SelectedEntities, new Func<GameEntity, bool>(x => x.IsEnabled));
            Name = GetMixedValue(SelectedEntities, new Func<GameEntity, string>(x => x.Name));
            return true;
        }

        /// <summary>
        /// 互斥从视图中更新数据
        /// </summary>
        public void Refresh()
        {
            _enableUpdates = false;
            UpdateMSGameEntity();
            MakeComponentList();
            _enableUpdates = true;
        }

        public MSEntity(List<GameEntity> entities)
        {
            Debug.Assert(entities.Any() == true);
            Components = new ReadOnlyObservableCollection<IMSComponent>(_components);
            SelectedEntities = entities;
            PropertyChanged += (s, e) =>
            {
                if (_enableUpdates)
                {
                    UpdateGameEntities(e.PropertyName);
                }
            };
        }
        /// <summary>
        /// Initializes a new instance of the <see cref="MSEntity"/> class.
        /// MSEntity 无参构造函数，为了暂时解决注入时预览问题
        /// </summary>
    }

    /// <summary>
    /// 继承于MSEntity的多选游戏实体类
    /// </summary>
    class MSGameEntity : MSEntity
    {
        public MSGameEntity(List<GameEntity> entities) : base(entities)
        {
            Refresh();
        }
    }
}
