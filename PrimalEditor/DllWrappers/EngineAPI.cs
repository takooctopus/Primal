using PrimalEditor.Components;
using PrimalEditor.EngineAPIStructs;
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
    class GameEntityDescriptor
    {
        public TransformComponent Transform = new TransformComponent();
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
