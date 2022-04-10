using PrimalEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;

namespace PrimalEditor.GameProject
{
    /// <summary>
    /// 项目信息类[我们要打开的项目的基础路径图片信息被放在这里面]
    /// </summary>
    [DataContract]
    public class ProjectData
    {
        /// <summary>
        /// 项目名称
        /// </summary>
        /// <value>
        /// The name of the project.
        /// </value>
        [DataMember]
        public string ProjectName { get; set; }
        /// <summary>
        /// 项目所在文件夹路径[不包括项目名称]
        /// </summary>
        /// <value>
        /// The project path.
        /// </value>
        [DataMember]
        public string ProjectPath { get; set; }
        /// <summary>
        /// 项目修改时间
        /// </summary>
        /// <value>
        /// The date.
        /// </value>
        [DataMember]
        public DateTime Date { get; set; }

        /// <summary>
        /// 项目的主描述结构文件位置
        /// </summary>
        /// <value>
        /// The full path.
        /// </value>
        public string FullPath { get => $"{ProjectPath}{ProjectName}{Project.Extention}"; }

        /// <summary>
        /// 图标二进制文件
        /// </summary>
        /// <value>
        /// The icon.
        /// </value>
        public byte[] Icon { get; set; }
        /// <summary>
        /// 截图二进制文件
        /// </summary>
        /// <value>
        /// The screenshot.
        /// </value>
        public byte[] Screenshot { get; set; }
    }

    /// <summary>
    /// 项目信息类列表类【里面就是一个列表，全是要打开的项目信息】
    /// </summary>
    [DataContract]
    public class ProjectDataList
    {
        [DataMember]
        public List<ProjectData> Projects { get; set; }
    }

    /// <summary>
    /// 打开项目类
    /// </summary>
    class OpenProject
    {
        /// <summary>
        /// 【只读】编辑器路径
        /// </summary>
        private static readonly string _applicationDataPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}\PrimalEditor\";
        /// <summary>
        /// 项目文件【.primal文件】路径
        /// </summary>
        private static readonly string _projectDataPath;

        /// <summary>
        /// 【prop】用来装所有项目信息类ProjectData的列表
        /// </summary>
        private static readonly ObservableCollection<ProjectData> _projects = new ObservableCollection<ProjectData>();
        public static ReadOnlyObservableCollection<ProjectData> Projects { get; }

        private static void ReadProjectData()
        {
            if (File.Exists(_projectDataPath))
            {
                var projects = Serializer.FromFile<ProjectDataList>(_projectDataPath).Projects.OrderByDescending(x => x.Date);
                _projects.Clear();
                foreach (var project in projects)
                {
                    if (File.Exists(project.FullPath))
                    {
                        project.Icon = File.ReadAllBytes($@"{project.ProjectPath}\.Primal\icon.png");
                        project.Screenshot = File.ReadAllBytes($@"{project.ProjectPath}\.Primal\screenshot.png");
                        _projects.Add(project);
                    }
                }
            }
        }
        /// <summary>
        /// 持久化项目信息
        /// </summary>
        private static void WriteProjectData()
        {
            // TOERROR: 创建项目时这个地方直接卡死了，搞什么啊
            var projects = _projects.OrderBy(x => x.Date).ToList();
            Serializer.ToFile(new ProjectDataList() { Projects = projects }, _projectDataPath);
        }

        /// <summary>
        /// 【函数】根据项目信息ProjectData打开项目
        /// </summary>
        /// <param name="data">The data.</param>
        /// <returns></returns>
        public static Project Open(ProjectData data)
        {
            ReadProjectData();
            var project = _projects.FirstOrDefault(x => x.FullPath == data.FullPath);
            if (project != null)
            {
                project.Date = DateTime.Now;
            }
            else
            {
                project = data;
                project.Date = DateTime.Now;
                _projects.Add(project);
            }
            WriteProjectData();
            return Project.Load(project.FullPath);
        }

        static OpenProject()
        {
            try
            {
                if (!Directory.Exists(_applicationDataPath))
                {
                    Directory.CreateDirectory(_applicationDataPath);
                }
                _projectDataPath = $@"{_applicationDataPath}ProjectData.xml";
                Projects = new ReadOnlyObservableCollection<ProjectData>(_projects);
                ReadProjectData();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, $"Failed to read project data of: {_projectDataPath}");
                throw;
            }
        }
    }
}
