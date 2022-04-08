using PrimalEditor.Components;
using PrimalEditor.EngineAPIStructs;
using PrimalEditor.GameProject;
using PrimalEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;


namespace PrimalEditor.EngineAPIStructs
{
    [StructLayout(LayoutKind.Sequential)]
    class TransformComponent
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale = new Vector3(1, 1, 1);
    }

    [StructLayout(LayoutKind.Sequential)]
    class ScriptComponent
    {
        public IntPtr ScriptCreator;
    }

    /// <summary>
    /// 游戏实体描述信息的结构体
    /// 用于的描述我们获得数据的格式化结构体类
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    class GameEntityDescriptor
    {
        public TransformComponent Transform = new TransformComponent();
        public ScriptComponent Script = new ScriptComponent();
    }
}

namespace PrimalEditor.DllWrappers
{
    static class EngineAPI
    {
        private const string _engineDll = "EngineDLL.dll";

        [DllImport(_engineDll, CharSet = CharSet.Ansi)]
        public static extern int LoadGameCodeDll(string dllPath);


        [DllImport(_engineDll, CharSet = CharSet.Ansi)]
        public static extern int UnloadGameCodeDll();

        /// <summary>
        /// Gets the script creator.c#接口
        /// 传一个string【脚本名称】进去，返回这个脚本生成的函数对应的地址Intptr
        /// 因为我们实际上并不会用这个，只是将这个传递给engine而已
        /// </summary>
        /// <param name="name">The name.</param>
        /// <returns></returns>
        [DllImport(_engineDll)]
        public static extern IntPtr GetScriptCreator(string name);

        /// <summary>
        /// Gets the script names. C#接口
        /// </summary>
        /// <returns></returns>
        [DllImport(_engineDll)]
        [return: MarshalAs(UnmanagedType.SafeArray)]
        public static extern string[] GetScriptNames();

        [DllImport(_engineDll)]
        public static extern int CreateRenderSurface(IntPtr host, int width, int height);

        [DllImport(_engineDll)]
        public static extern void RemoveRenderSurface(int surfaceId);

        [DllImport(_engineDll)]
        public static extern void ResizeRenderSurface(int surfaceId);

        [DllImport(_engineDll)]
        public static extern IntPtr GetWindowHandle(int surfaceId); 

        internal static class EntityAPI
        {
            /// <summary>
            /// 从dll中引入的创建游戏实体函数
            /// </summary>
            /// <param name="desc"></param>
            /// <returns></returns>
            [DllImport(_engineDll)]
            private static extern int CreateGameEntity(GameEntityDescriptor desc);
            /// <summary>
            /// 创建函数实体的C#侧接口函数
            /// </summary>
            /// <param name="entity"></param>
            /// <returns></returns>
            public static int CreateGameEntity(GameEntity entity)
            {
                GameEntityDescriptor desc = new GameEntityDescriptor();
                // 将CS中的GameEntity类数据过滤放入接口API的数据结构中[即GameEntityDescriptor]
                // transformComponent的数据逻辑
                {
                    var c = entity.GetComponent<Transform>();
                    desc.Transform.Position = c.Position;
                    desc.Transform.Rotation = c.Rotation;
                    desc.Transform.Scale = c.Scale;
                }

                // scriptComponent的数据逻辑
                {
                    // 获取游戏实体对象拥有的脚本对象，我们其实只需要传入对象的名称就好
                    var c = entity.GetComponent<Script>();
                    // 这里我们要判断当前项目是否为null， 如果为null则 GameCode DLL还未被载入，这个时候脚本组件的创建将会被推迟【但我们仍旧可以建立一个游戏实体(没有脚本属性)】
                    if( c != null && Project.Current != null)
                    {
                        if (Project.Current.AvailableScripts.Contains(c.Name))
                        {
                            // 把api从tag中返回的函数地址放到我们的脚本类结构体中
                            desc.Script.ScriptCreator = GetScriptCreator(c.Name);
                        }
                        else
                        {
                            Logger.Log(MessageType.Error, $"Unable to find script with name {c.Name}, game entity will be created without script component! ");
                        }
                    }
                }


                return CreateGameEntity(desc);
            }

            /// <summary>
            /// 从dll引入的移除游戏实体函数
            /// </summary>
            /// <param name="id"></param>
            [DllImport(_engineDll)]
            private static extern void RemoveGameEntity(int id);
            /// <summary>
            /// c#侧移除游戏实体函数接口
            /// </summary>
            /// <param name="entity"></param>
            public static void RemoveGameEntity(GameEntity entity)
            {
                RemoveGameEntity(entity.EntityId);
            }
        }
    }
}
