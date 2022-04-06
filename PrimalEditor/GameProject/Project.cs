using PrimalEditor.GameDev;
using PrimalEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
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
        /// 我们Editor本身工程的主拓展名.primal
        /// </summary>
        public static string Extention { get; } = ".primal";

        /// <summary>
        /// 现在的项目名称
        /// </summary>
        [DataMember]
        public string Name { get; private set; } = "New Project";

        /// <summary>
        /// Editor项目本身工程项目.primal文件所在文件夹的路径
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


        [DataMember(Name = "Scenes")]
        private ObservableCollection<Scene> _scenes = new ObservableCollection<Scene>();
        public ReadOnlyCollection<Scene> Scenes
        {
            get;
            private set;
        }
        private Scene _activeScene;
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
        public static Project Current => Application.Current.MainWindow.DataContext as Project;
        public static UndoRedo UndoRedo { get; } = new UndoRedo();

        public ICommand UndoCommand { get; private set; }
        public ICommand RedoCommand { get; private set; }
        public ICommand AddSceneCommand { get; private set; }
        public ICommand RemoveSceneCommand { get; private set; }
        public ICommand SaveCommand { get; private set; }
        public ICommand BuildCommand { get; private set; }

        private static string GetConfigurationName(BuildConfiguration config) => _buildConfigurationNames[(int)config];

        private void AddScene(string sceneName)
        {
            Debug.Assert(!string.IsNullOrEmpty(sceneName.Trim()));
            _scenes.Add(new Scene(this, sceneName));
        }
        private void RemoveScene(Scene scene)
        {
            Debug.Assert(_scenes.Contains(scene));
            _scenes.Remove(scene);
        }
        public static Project Load(string file)
        {
            Debug.Assert(File.Exists(file));
            return Serializer.FromFile<Project>(file);
        }
        public void Unload()
        {
            // 退出主程序时要关闭VS
            VisualStudio.CloseVisualStudio();
            UndoRedo.Reset();
        }

        public static void Save(Project project)
        {
            Serializer.ToFile(project, project.FullPath);
            Logger.Log(MessageType.Info, $"Project saved to {project.FullPath}");
        }

        /// <summary>
        /// 用来形成Icommand的元素的 构建项目函数
        /// </summary>
        public void BuildGameCodeDll(bool showWindow = true)
        {
            try
            {
                UnloadGameCodeDll();
                //构建gameCode
                VisualStudio.BuildSolution(this, GetConfigurationName(DllBuildConfig), showWindow);
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

        public static void UnloadGameCodeDll()
        {

        }

        public static void LoadGameCodeDll()
        {

        }

        [OnDeserialized]
        private void OnDeserialized(StreamingContext contex)
        {
            if (_scenes != null)
            {
                Scenes = new ReadOnlyObservableCollection<Scene>(_scenes);
                OnPropertyChanged(nameof(Scenes));
            }
            ActiveScene = Scenes.FirstOrDefault(x => x.IsActive);

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
            BuildCommand = new RelayCommand<bool>(x => BuildGameCodeDll(x), x => !VisualStudio.IsDebugging() && VisualStudio.BuildDone);
        }
        public Project(string name, string path)
        {
            Name = name;
            Path = path;
            OnDeserialized(new StreamingContext());
        }
    }
}
