using PrimalEditor.GameProject;
using PrimalEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Text;
using System.Threading.Tasks;

namespace PrimalEditor.GameDev
{
    /// <summary>
    /// VisualStudio类，包装了打开vs实例的方法，说白了我们的界面是做操作的，编译的事情就交给VS去做吧
    /// </summary>
    static class VisualStudio
    {
        /// <summary>
        /// 打开的VS实例，一个程序全局只有一个
        /// </summary>
        private static EnvDTE80.DTE2 _vsInstance = null;
        /// <summary>
        /// VS我们要的版本，这个就是VS2022，要是2019就是16.0，不考虑版本就删掉版本就是了
        /// </summary>
        private static readonly string _progID = "VisualStudio.DTE.17.0";

        /// <summary>
        /// 用来判断构建GameCode成功与否的标志量
        /// </summary>
        public static bool BuildSuceeded { get; private set; } = true;

        /// <summary>
        /// 用来判断构建GameCode与否结束的标志量
        /// </summary>
        public static bool BuildDone { get; private set; } = true;

        /// <summary>
        /// 从ole32.dll导入的方法：返回运行实例表
        /// </summary>
        /// <param name="reserved"></param>
        /// <param name="pprot"></param>
        /// <returns></returns>
        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable pprot);

        /// <summary>
        /// 从ole32.dll导入的方法：绑定实例上下文
        /// </summary>
        /// <param name="reserved"></param>
        /// <param name="ppbc"></param>
        /// <returns></returns>
        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);

        /// <summary>
        /// 怎么说呢，这个方法打开VS实例并绑定到我们的单例私有变量_vsInstance中
        /// </summary>
        /// <param name="solutionPath"></param>
        public static void OpenVisualStudio(string solutionPath)
        {
            IRunningObjectTable rot = null;
            IEnumMoniker monikerTable = null;
            IBindCtx bindCtx = null;
            try
            {
                if (_vsInstance == null)
                {
                    // find and open visual studio
                    var hResult = GetRunningObjectTable(0, out rot);
                    if (hResult < 0 || rot == null) throw new COMException($"GetRunningObjectTable() returned HRESULT: {hResult:X8}");

                    rot.EnumRunning(out monikerTable);
                    monikerTable.Reset();

                    hResult = CreateBindCtx(0, out bindCtx);
                    if (hResult < 0 || bindCtx == null) throw new COMException($"CreateBindCtx() returned HRESULT: {hResult:X8}");

                    IMoniker[] currentMoniker = new IMoniker[1];
                    // 每次只获取一个
                    while (monikerTable.Next(1, currentMoniker, IntPtr.Zero) == 0)
                    {
                        string name = string.Empty;
                        currentMoniker[0]?.GetDisplayName(bindCtx, null, out name);

                        // 如果获取的实例名字里有VisualStudio.DTE.17.0:19336 [:后面的是进程ID]这样的字段，那么就进去看看是不是我们已经打开过了
                        if (name.Contains(_progID))
                        {
                            // [BUG警告：] 我在这里找了好久BUG不知道为什么到这里就崩溃，不返回信息，找了好久才发现是之前打开的VS僵尸进程导致的，解决方法就是到任务管理器中找到对应僵尸进程的VS进程kill掉就好，我是怎么发现的呢，是因为我发现上面name有时会出现两个VisualStudio.DTE.17.0:xxxx的字样，有时第一个崩有时第二个崩，而我只开了一个VS窗口。我只能说简单的嘴臭，极致的享受。
                            hResult = rot.GetObject(currentMoniker[0], out object obj);
                            if (hResult < 0 || obj == null)
                                throw new COMException($"Running object table's GetObject() returned HRESULT: {hResult:X8}");
                            // 把进程object转化成对应的DTE2对象
                            EnvDTE80.DTE2 dte = obj as EnvDTE80.DTE2;
                            var solutionName = dte.Solution.FullName;
                            // 我们对比一下这进程打开的解决方案的名称和我们新建脚本所属的解决方案是不是同一个，是的话就直接添加，不用再开一个VS窗口进程了
                            if (solutionName == solutionPath)
                            {
                                _vsInstance = dte;
                                break;
                            }
                        }
                    }
                    // 否则我们就新开一个VS进程，将其绑定在_vsInstance上
                    if (_vsInstance == null)
                    {
                        Type visuastudioType = Type.GetTypeFromProgID(_progID, true);
                        _vsInstance = Activator.CreateInstance(visuastudioType) as EnvDTE80.DTE2;
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.Write(ex.Message);
                Logger.Log(MessageType.Error, "failed to open Visual Studio");
            }
            finally
            {
                // 不管有没有成功打开，最后都要释放资源，如果中途break或者crash了，就会因为引计数的增加导致僵尸进程
                if (monikerTable != null) Marshal.ReleaseComObject(monikerTable);
                if (rot != null) Marshal.ReleaseComObject(rot);
                if (bindCtx != null) Marshal.ReleaseComObject(bindCtx);
            }
        }

        /// <summary>
        /// 关闭VS进程
        /// </summary>
        public static void CloseVisualStudio()
        {
            if (_vsInstance?.Solution.IsOpen == true)
            {
                // 先将工作区的所有文件保存了，然后关闭解决方案
                _vsInstance.ExecuteCommand("File.SaveAll");
                _vsInstance.Solution.Close(true);
            }
            // 退出进程
            _vsInstance?.Quit();
        }

        /// <summary>
        /// 添加模板文件到解决方案的函数
        /// </summary>
        /// <param name="solution">解决方案的具体路径，就是.sln文件的路径</param>
        /// <param name="projectName">游戏项目名称</param>
        /// <param name="files">要放进去的文件名列表[是文件的全路径]</param>
        /// <returns></returns>
        public static bool AddFilesToSolution(string solution, string projectName, string[] files)
        {
            Debug.Assert(files?.Length > 0);
            OpenVisualStudio(solution);
            try
            {
                Debug.Assert(_vsInstance != null);
                if (_vsInstance != null)
                {
                    if (!_vsInstance.Solution.IsOpen) _vsInstance.Solution.Open(solution);
                    else _vsInstance.ExecuteCommand("File.SaveAll");

                    // 对_vsInstance解决方案的所有项目进行遍历
                    foreach (EnvDTE.Project project in _vsInstance.Solution.Projects)
                    {
                        // 找到对应的项目文件名xxx.vcxproj，要是包含我们要的的项目名，就直接添加进project里
                        if (project.UniqueName.Contains(projectName))
                        {
                            foreach (var file in files)
                            {
                                //向项目中添加文件
                                project.ProjectItems.AddFromFile(file);
                            }
                        }
                    }

                    // 为了方便查看，将打开加进去的cpp文件方便操作
                    var cpp = files.FirstOrDefault(x => Path.GetExtension(x) == ".cpp");
                    if (!string.IsNullOrEmpty(cpp))
                    {
                        // 注意这里因为没有interface界面，我们要将项目的Reference->EnvDTE->Embed Interop Type属性设置成false
                        _vsInstance.ItemOperations.OpenFile(cpp, EnvDTE.Constants.vsViewKindTextView).Visible = true;
                    }
                    _vsInstance.MainWindow.Activate();
                    _vsInstance.MainWindow.Visible = true;
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, "failed to add files to visual studio project");
                return false;
            }
            return true;
        }

        /// <summary>
        /// 在VS构建Solution开始时的触发事件函数
        /// </summary>
        /// <param name="project"></param>
        /// <param name="projectConfig"></param>
        /// <param name="platform"></param>
        /// <param name="solutionConfig"></param>
        private static void OnBuildSolutionBegin(string project, string projectConfig, string platform, string solutionConfig)
        {
            Logger.Log(MessageType.Info, $"Building {project}, {projectConfig}, {platform}, {solutionConfig}.");
        }

        /// <summary>
        /// 在VS构建Solution结束时的触发事件函数
        /// </summary>
        /// <param name="project"></param>
        /// <param name="projectConfig"></param>
        /// <param name="platform"></param>
        /// <param name="solutionConfig"></param>
        /// <param name="Success"></param>
        private static void OnBuildSolutionDone(string project, string projectConfig, string platform, string solutionConfig, bool Success)
        {
            if (BuildDone) return;
            if (Success) Logger.Log(MessageType.Info, $"Building {projectConfig} configuration succeeded.");
            else Logger.Log(MessageType.Error, $"Building {projectConfig} configuration failed.");

            BuildDone = true;
            BuildSuceeded = Success;
        }


        /// <summary>
        /// 判断现在VS是否正在Debug运行实例
        /// </summary>
        /// <returns></returns>
        public static bool IsDebugging()
        {
            bool result = false;
            bool tryAgain = true;
            for (int i = 0; i < 3 && tryAgain; i++)
            {
                try
                {
                    result = _vsInstance != null
                        && (_vsInstance.Debugger.CurrentProgram != null || _vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);
                    tryAgain = false;
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    System.Threading.Thread.Sleep(1000);
                }
                if (result) break;
            }
            return result;
        }

        /// <summary>
        /// 构建GameCode函数
        /// </summary>
        /// <param name="project"></param>
        /// <param name="configName"></param>
        /// <param name="showWindow">[可选]是否结束后显示窗口</param>
        /// <exception cref="NotImplementedException"></exception>
        public static void BuildSolution(Project project, string configName, bool showWindow = true)
        {
            // 先判断VS现在是否在DEBUG模式
            if (IsDebugging())
            {
                Logger.Log(MessageType.Error, "Visual Studio is currently running a process.");
                return;
            }
            OpenVisualStudio(project.Solution);
            BuildDone = BuildSuceeded = false;

            // TODO: 现在这个只是简易实现，最好还是用信息过滤器来实现忙碌信息
            for (int i = 0; i < 3 && !BuildDone; i++)
            {
                try
                {
                    if (!_vsInstance.Solution.IsOpen) _vsInstance.Solution.Open(project.Solution);
                    _vsInstance.MainWindow.Visible = showWindow;

                    _vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBuildSolutionBegin;
                    _vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBuildSolutionDone;

                    try
                    {
                        foreach (var PdbFile in Directory.GetFiles(Path.Combine($"{project.Path}", $@"x64\{configName}"), "*.pdb"))
                        {
                            File.Delete(PdbFile);
                        }
                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine(ex.Message);
                    }

                    _vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(configName).Activate();
                    _vsInstance.ExecuteCommand("Build.BuildSolution");
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                    Debug.WriteLine($"Attempt {i}: failed to build {project.Name} game code in VS");
                    Logger.Log(MessageType.Error, "Building game code in Vs failed");
                    System.Threading.Thread.Sleep(1000);
                }
            }
        }

        public static void Run(Project project, string configName, bool debug)
        {
            if (_vsInstance != null && !IsDebugging() && BuildDone && BuildSuceeded)
            {
                _vsInstance.ExecuteCommand(debug ? "Debug.Start" : "Debug.StartWithoutDebugging");
            }

        }

        public static void Stop()
        {
            if(_vsInstance != null && IsDebugging())
            {
                _vsInstance.ExecuteCommand("Debug.StopDebugging");
            }
        }
    }
}
