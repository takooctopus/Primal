using PrimalEditor.Components;
using PrimalEditor.DllWrappers;
using PrimalEditor.GameDev;
using PrimalEditor.Utilities;
using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace PrimalEditor.GameProject
{
    enum BuildConfiguration
    {
        Debug,
        DebugEditor,
        Release,
        ReleaseEditor
    }

    [DataContract(Name = "Game")]
    class Project : ViewModelBase
    {
        /// <summary>
        /// 我们Game本身工程的主拓展名.primal
        /// </summary>
        public static string Extention { get; } = ".primal";

        /// <summary>
        /// 现在的项目名称
        /// </summary>
        [DataMember]
        public string Name { get; private set; } = "New Project";

        /// <summary>
        /// Game项目本身工程项目.primal文件所在文件夹的路径，你那个游戏项目的主目录，包含项目名称那个
        /// </summary>
        [DataMember]
        public string Path { get; private set; }

        /// <summary>
        /// Editor项目本身工程项目.primal文件的全路径
        /// </summary>
        public string FullPath => $@"{Path}{Name}{Extention}";

        /// <summary>
        /// 生成的Game的MSVC工程的.sln文件全路径
        /// </summary>
        public string Solution => $@"{Path}{Name}.sln";

        public string ContentPath => $@"{Path}Content\";

        /// <summary>
        /// 项目的参数
        /// </summary>
        private static readonly string[] _buildConfigurationNames = new string[]
        {
            "Debug",
            "DebugEditor",
            "Release",
            "ReleaseEditor"
        };

        /// <summary>
        /// 绑定于BuildConfig的属性私有变量
        /// </summary>
        private int _buildConfig;
        /// <summary>
        /// GameCode的编译配置名，我们希望这个被序列化，方便下次打开时还用这个配置
        /// </summary>
        [DataMember]
        public int BuildConfig
        {
            get => _buildConfig;
            set
            {
                if (_buildConfig != value)
                {
                    _buildConfig = value;
                    OnPropertyChanged(nameof(BuildConfig));
                }
            }
        }

        /// <summary>
        /// 编译成EXE的配置名
        /// </summary>
        public BuildConfiguration standAloneBuildConfig => BuildConfig == 0 ? BuildConfiguration.Debug : BuildConfiguration.Release;

        /// <summary>
        /// 编译成Dll的配置名
        /// </summary>
        public BuildConfiguration DllBuildConfig => BuildConfig == 0 ? BuildConfiguration.DebugEditor : BuildConfiguration.ReleaseEditor;

        /// <summary>
        /// 可用脚本名称数组
        /// </summary>
        private string[] _availableScriptes;
        /// <summary>
        /// Gets or sets 可用的脚本名称数组
        /// </summary>
        /// <value>
        /// The available scripts.
        /// </value>
        public string[] AvailableScripts
        {
            get => _availableScriptes;
            set
            {
                if (_availableScriptes != value)
                {
                    _availableScriptes = value;
                    OnPropertyChanged(nameof(AvailableScripts));
                }
            }
        }


        /// <summary>
        /// 所有的场景实例列表，这个我们要序列化的
        /// </summary>
        [DataMember(Name = "Scenes")]
        private ObservableCollection<Scene> _scenes = new ObservableCollection<Scene>();
        public ReadOnlyCollection<Scene> Scenes
        {
            get;
            private set;
        }

        /// <summary>
        /// The active scene 当前激活的场景实例
        /// </summary>
        private Scene _activeScene;
        /// <summary>
        /// Gets or sets the active scene.
        /// 当前激活的场景实例
        /// </summary>
        /// <value>
        /// The active scene.
        /// </value>
        public Scene ActiveScene
        {
            get => _activeScene;
            set
            {
                if (_activeScene != value)
                {
                    _activeScene = value;
                    OnPropertyChanged(nameof(ActiveScene));
                }
            }
        }

        /// <summary>
        /// 当前项目Project，全局唯一实例
        /// </summary>
        /// <value>
        /// The current.
        /// </value>
        public static Project Current => Application.Current.MainWindow.DataContext as Project;

        /// <summary>
        /// 撤销重做实例UndoRedo，里面有俩列表放着撤销和重做的命令
        /// </summary>
        /// <value>
        /// The undo redo.
        /// </value>
        public static UndoRedo UndoRedo { get; } = new UndoRedo();

        public ICommand UndoCommand { get; private set; }
        public ICommand RedoCommand { get; private set; }
        public ICommand AddSceneCommand { get; private set; }
        public ICommand RemoveSceneCommand { get; private set; }
        public ICommand SaveCommand { get; private set; }
        public ICommand BuildCommand { get; private set; }
        public ICommand DebugStartCommand { get; private set; }
        public ICommand DebugStartWithoutDebuggingCommand { get; private set; }
        public ICommand DebugStopCommand { get; private set; }


        /// <summary>
        /// 对于Command的设定函数
        /// 特别的buildCommand是异步的
        /// </summary>
        private void SetCommands()
        {
            // 添加场景的Command
            AddSceneCommand = new RelayCommand<object>(x =>
            {
                AddScene($"New Scene {_scenes.Count}");
                var newScene = _scenes.Last();
                var sceneIndex = _scenes.Count - 1;
                UndoRedo.Add(new UndoRedoAction(
                    () => RemoveScene(newScene),
                    () => _scenes.Insert(sceneIndex, newScene),
                    $"Add {newScene.Name}"
                    ));
            });

            // 移除场景的Command
            RemoveSceneCommand = new RelayCommand<Scene>(x =>
            {
                var sceneIndex = _scenes.IndexOf(x);
                RemoveScene(x);
                UndoRedo.Add(new UndoRedoAction(
                    () => _scenes.Insert(sceneIndex, x),
                    () => RemoveScene(x),
                    $"Remove {x.Name}"
                    ));
            }, x => !x.IsActive);

            // 第一个参数是要调用的方法， 第二个是推断[推断为假这个button就按不下去]
            UndoCommand = new RelayCommand<object>(x => UndoRedo.Undo(), x => UndoRedo.UndoList.Any());
            RedoCommand = new RelayCommand<object>(x => UndoRedo.Redo(), x => UndoRedo.RedoList.Any());
            SaveCommand = new RelayCommand<object>(x => Save(this));
            BuildCommand = new RelayCommand<bool>(async x => await BuildGameCodeDll(x), x => !VisualStudio.IsDebugging() && VisualStudio.BuildDone);

            DebugStartCommand = new RelayCommand<object>(async x => await RunGame(true), x => !VisualStudio.IsDebugging() && VisualStudio.BuildDone);
            DebugStartWithoutDebuggingCommand = new RelayCommand<object>(async x => await RunGame(false), x => !VisualStudio.IsDebugging() && VisualStudio.BuildDone);
            DebugStopCommand = new RelayCommand<object>(async x => await StopGame(), x => VisualStudio.IsDebugging());


            OnPropertyChanged(nameof(AddSceneCommand));
            OnPropertyChanged(nameof(RemoveSceneCommand));
            OnPropertyChanged(nameof(UndoCommand));
            OnPropertyChanged(nameof(RedoCommand));
            OnPropertyChanged(nameof(SaveCommand));
            OnPropertyChanged(nameof(BuildCommand));
            OnPropertyChanged(nameof(DebugStartCommand));
            OnPropertyChanged(nameof(DebugStartWithoutDebuggingCommand));
            OnPropertyChanged(nameof(DebugStopCommand));
        }

        /// <summary>
        /// 类函数用来获取编译配置
        /// </summary>
        /// <param name="config">The configuration.[enum]</param>
        /// <returns></returns>
        private static string GetConfigurationName(BuildConfiguration config) => _buildConfigurationNames[(int)config];

        /// <summary>
        /// 根据场景名称添加场景
        /// </summary>
        /// <param name="sceneName">Name of the scene.</param>
        private void AddScene(string sceneName)
        {
            Debug.Assert(!string.IsNullOrEmpty(sceneName.Trim()));
            _scenes.Add(new Scene(this, sceneName));
        }

        /// <summary>
        /// 根据场景实例移除场景
        /// </summary>
        /// <param name="scene">The scene.</param>
        private void RemoveScene(Scene scene)
        {
            Debug.Assert(_scenes.Contains(scene));
            _scenes.Remove(scene);
        }

        /// <summary>
        /// 反序列化读取项目配置文件
        /// </summary>
        /// <param name="file">The file.</param>
        /// <returns></returns>
        public static Project Load(string file)
        {
            Debug.Assert(File.Exists(file));
            return Serializer.FromFile<Project>(file);
        }
        /// <summary>
        /// 退出程序时要做的事情
        /// </summary>
        public void Unload()
        {
            UnloadGameCodeDll();
            // 退出主程序时要关闭VS
            VisualStudio.CloseVisualStudio();
            UndoRedo.Reset();
        }

        /// <summary>
        /// 调用序列化器进行持久化
        /// </summary>
        /// <param name="project">The project.</param>
        public static void Save(Project project)
        {
            Serializer.ToFile(project, project.FullPath);
            Logger.Log(MessageType.Info, $"Project saved to {project.FullPath}");
        }

        /// <summary>
        /// 将游戏实体存放成二进制
        /// </summary>
        private void SaveToBinary()
        {
            var configName = GetConfigurationName(standAloneBuildConfig);
            var bin = $@"{Path}x64\{configName}\game.bin";
            using (var bw = new BinaryWriter(File.Open(bin, FileMode.Create, FileAccess.Write)))
            {
                bw.Write(ActiveScene.GameEntities.Count);
                foreach (var entity in ActiveScene.GameEntities)
                {
                    bw.Write(0); //实体类别 entity type TODO:
                    bw.Write(entity.Components.Count);
                    foreach (var component in entity.Components)
                    {
                        bw.Write((int)component.toEnumType());
                        component.WriteToBinary(bw);
                    }
                }
            }
        }

        /// <summary>
        /// 异步task：开始debug或者非debug运行
        /// </summary>
        /// <param name="debug">if set to <c>true</c> [debug].</param>
        private async Task RunGame(bool debug)
        {
            var configName = GetConfigurationName(standAloneBuildConfig);
            await Task.Run(() => VisualStudio.BuildSolution(this, configName, debug));
            if (VisualStudio.BuildSuceeded)
            {
                SaveToBinary();
                await Task.Run(() => VisualStudio.Run(this, configName, debug));
            }
        }

        /// <summary>
        /// 异步task，停止game的运行
        /// </summary>
        private async Task StopGame() => await Task.Run(() => VisualStudio.Stop());

        /// <summary>
        /// 用来形成Icommand的元素的 构建项目函数
        /// </summary>
        public async Task BuildGameCodeDll(bool showWindow = true)
        {
            try
            {
                UnloadGameCodeDll();
                //构建gameCode
                await Task.Run(() => VisualStudio.BuildSolution(this, GetConfigurationName(DllBuildConfig), showWindow));
                if (VisualStudio.BuildSuceeded)
                {
                    LoadGameCodeDll();
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.ToString());
                Logger.Log(MessageType.Error, "Build game code failed.");
                throw;
            }

        }

        /// <summary>
        /// 加载GameCode的dll文件 
        /// </summary>
        private void LoadGameCodeDll()
        {
            // 现在选择的编译配置文件名
            var configName = GetConfigurationName(DllBuildConfig);
            var dll = $@"{Path}x64\{configName}\{Name}.dll";
            AvailableScripts = null;
            if (File.Exists(dll) && EngineAPI.LoadGameCodeDll(dll) != 0)
            {
                AvailableScripts = EngineAPI.GetScriptNames();
                // 当前激活的场景里[一个].所有的游戏实体.找到里面只要有脚本文件的，把它们激活状态全部设置为true
                // TODEBUG: 这里有问题，初始化时创建的gameentity并没有脚本属性[现在再将每个entity的IsActive都设置为True]
                ActiveScene.GameEntities.Where(x => x.GetComponent<Script>() != null).ToList().ForEach(x => x.IsActive = true);
                Logger.Log(MessageType.Info, "Game Code DLL loaded successfully.");
            }
            else
            {
                Logger.Log(MessageType.Warning, "Failed to load game code dll file, try to build the project first.");
            }
        }

        /// <summary>
        /// 移除GameCode的dll文件
        /// </summary>
        private void UnloadGameCodeDll()
        {
            // 当前激活的场景里[一个].所有的游戏实体.找到里面只要有脚本文件的，把它们激活状态全部设置为false
            ActiveScene.GameEntities.Where(x => x.GetComponent<Script>() != null).ToList().ForEach(x => x.IsActive = false);
            if (EngineAPI.UnloadGameCodeDll() != 0)
            {
                AvailableScripts = null;
                Logger.Log(MessageType.Info, "Game Code DLL unloaded successfully.");
            }
        }


        [OnDeserialized]
        private async void OnDeserialized(StreamingContext contex)
        {
            // 初始化场景集合
            if (_scenes != null)
            {
                Scenes = new ReadOnlyObservableCollection<Scene>(_scenes);
                OnPropertyChanged(nameof(Scenes));
            }
            ActiveScene = Scenes.FirstOrDefault(x => x.IsActive);

            Debug.Assert(ActiveScene != null);

            // [异步] 加载时要导入gamecode生成的dll文件
            {
                await BuildGameCodeDll(false);
            }

            SetCommands();
        }
        public Project(string name, string path)
        {
            Name = name;
            Path = path;
            OnDeserialized(new StreamingContext());
        }
    }
}
