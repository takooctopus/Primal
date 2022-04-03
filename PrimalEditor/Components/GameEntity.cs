using PrimalEditor.DllWrappers;
using PrimalEditor.GameProject;
using PrimalEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace PrimalEditor.Components
{
    [DataContract]
    [KnownType(typeof(Transform))]
    class GameEntity : ViewModelBase
    {
        // 这里的EntityId设置是从c++引擎中传回的，对应的是实体在C++引擎创建的实体entity，但是传回的其实只有entity._id，毕竟现在里面只有一个_id属性.
        private int _entityId = ID.INVALID_ID;
        public int EntityId { 
            get => _entityId; 
            set
            {
                if(_entityId != value)
                {
                    _entityId = value;
                    OnPropertyChanged(nameof(EntityId));
                }
            } 
        }
        private bool _isActive;

        public bool IsActive
        {
            get => _isActive;
            set {
                if(_isActive != value)
                {
                    _isActive = value;
                    if (_isActive)
                    {
                        EntityId = EngineAPI.CreateGameEntity(this);
                        Debug.Assert(ID.IsValid(_entityId));
                    }
                    else
                    {
                        EngineAPI.RemoveGameEntity(this);
                    }
                    OnPropertyChanged(nameof(IsActive));
                }                
            }
        }

        private bool _isEnabled = true;
        [DataMember]
        public bool IsEnabled
        {
            get => _isEnabled;
            set
            {
                if(_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }
        private string _name;
        [DataMember]
        public string Name
        {
            get => _name;
            set
            {
                if(_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        [DataMember]
        public Scene ParentScene { get; private set; }
        [DataMember(Name = nameof(Components))]
        private readonly ObservableCollection<Component> _components = new ObservableCollection<Component>();
        public ReadOnlyObservableCollection<Component> Components { get; private set; }

        // 获取组件Component接口
        public Component GetComponent(Type type) => Components.FirstOrDefault(c => c.GetType() == type);
        // 相当于强制转换类型，将获取的Component进行类型转换
        public T GetComponent<T>() where T : Component => GetComponent(typeof(T)) as T;

        // 同样的， 这两个ICommand只适用于单体的更改，到集合后就不适用了
        //public ICommand RenameCommand { get; private set; }
        //public ICommand IsEnableCommand { get; private set; }
        [OnDeserialized]
        void OnDeserialized(StreamingContext context)
        {
            Debug.Assert(_components != null);
            if(_components != null)
            {
                Components = new ReadOnlyObservableCollection<Component>(_components);
                OnPropertyChanged(nameof(Components));
            }

            // 同样的， 这两个ICommand只适用于单体的更改，到集合后就不适用了
            //RenameCommand = new RelayCommand<string>(x =>
            //{
            //    var oldName = _name;
            //    Name = x;
            //    Project.UndoRedo.Add(new UndoRedoAction(
            //        nameof(Name),
            //        this,
            //        oldName,
            //        x,
            //        $"Rename entity '{oldName}' to '{x}'"
            //        )); 
            //}, x => x != _name);

            //IsEnableCommand = new RelayCommand<bool>(x =>
            //{
            //    var oldValue = _isEnabled;
            //    IsEnabled = x;
            //    Project.UndoRedo.Add(new UndoRedoAction(
            //        nameof(IsEnabled),
            //        this,
            //        oldValue,
            //        x,
            //        x ? $"Enable {Name}" : $"Disable {Name}"
            //        ));
            //});
        }
        public GameEntity(Scene scene)
        {
            Debug.Assert(scene != null);
            ParentScene = scene;
            _components.Add(new Transform(this));
            OnDeserialized(new StreamingContext());
        }
    }

    abstract class MSEntity : ViewModelBase
    {
        private bool _enableUpdates = true;
        // 注意这里bool是可以为null的，这里这样选择是因为集合的原因，身为集合很可能下面的gameEntity._isEnable是不统一的，null就表示了这种不统一的特性
        private bool? _isEnabled = true;
        public bool? IsEnabled
        {
            get => _isEnabled;
            set
            {
                if(_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }

        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if(_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        private readonly ObservableCollection<IMSComponent> _components = new ObservableCollection<IMSComponent>();
        public ReadOnlyObservableCollection<IMSComponent> Components { get; }
        public List<GameEntity> SelectedEntities { get; }
        public static float? GetMixedValue(List<GameEntity> entities, Func<GameEntity,float> getProperty)
        {
            var value = getProperty(entities.First());
            foreach(var entity in entities.Skip(1))
            {
                // 这个函数是建立于Utilities.cs->MathUtil类中的
                if (!value.IsTheSameAs(getProperty(entity)))
                {
                    return null;
                }
            }
            return value;
        }
        public static bool? GetMixedValue(List<GameEntity> entities, Func<GameEntity, bool> getProperty)
        {
            var value = getProperty(entities.First());
            foreach (var entity in entities.Skip(1))
            {
                if (value != getProperty(entity))
                {
                    return null;
                }
            }
            return value;
        }

        public static string GetMixedValue(List<GameEntity> entities, Func<GameEntity, string> getProperty)
        {
            var value = getProperty(entities.First());
            foreach (var entity in entities.Skip(1))
            {
                if (value != getProperty(entity))
                {
                    return null;
                }
            }
            return value;
        }
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
        protected virtual bool UpdateMSGameEntity()
        {  
            IsEnabled = GetMixedValue(SelectedEntities, new Func<GameEntity, bool>(x => x.IsEnabled));
            Name = GetMixedValue(SelectedEntities, new Func<GameEntity, string>(x => x.Name));
            return true;
        }
        public void Refresh()
        {
            _enableUpdates = false;
            UpdateMSGameEntity();
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
    }

    class MSGameEntity : MSEntity
    {
        public MSGameEntity(List<GameEntity> entities) : base(entities)
        {
            Refresh();
        }
    }
}
