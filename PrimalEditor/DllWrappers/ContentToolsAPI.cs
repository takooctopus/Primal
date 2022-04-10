using PrimalEditor.ContentToolsAPIStruct;
using PrimalEditor.Utilities;
using System;
using System.Diagnostics;
using System.Numerics;
using System.Runtime.InteropServices;

namespace PrimalEditor.ContentToolsAPIStruct
{
    [StructLayout(LayoutKind.Sequential)]
    class GeometryImportSettings
    {
        public float SmoothingAngle = 178f; // 平滑角度
        public byte CalculateNormals = 0;   // 计算法线?(bool)
        public byte CalculateTangents = 1;  //计算切线?(bool)
        public byte ReverseHandedness = 0;  // 左右手互换？(bool)
        public byte ImportEmbededTextures = 1; // 导入嵌入纹理?(bool)
        public byte ImportAnimations = 1;	// 导入动画?(bool)
    }

    [StructLayout(LayoutKind.Sequential)]
    class SceneData : IDisposable
    {
        public IntPtr Data;
        public int DataSize;
        public GeometryImportSettings settings = new GeometryImportSettings();

        public void Dispose()
        {
            // 因为这块内存是c++中创建的，我们应该自己进行垃圾回收
            Marshal.FreeCoTaskMem(Data);
            // 手动垃圾回收，就不应该让GC再进行一次
            GC.SuppressFinalize(this);
        }

        ~SceneData()
        {
            Dispose();
        }
    }

    /// <summary>
    /// PrimitiveInitInfo应当与c++中的格式对应
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    class PrimitiveInitInfo
    {
        public Content.PrimitiveMeshType Type;
        public int SegmentX = 1;
        public int SegmentY = 1;
        public int SegmentZ = 1;
        public Vector3 Size = new Vector3(1f);
        public int Lod = 0;
    }

}


namespace PrimalEditor.DllWrappers
{

    static class ContentToolsAPI
    {
        private const string _contentToolsDll = "ContentTools.dll";

        [DllImport(_contentToolsDll)]
        private static extern void CreatePrimitiveMesh([In, Out] SceneData data, PrimitiveInitInfo info);
        public static void CreatePrimitiveMesh(Content.Geometry geometry, PrimitiveInitInfo info)
        {
            Debug.Assert(geometry != null);
            //!!!使用using保证出范围时会自动调用SceneData.Dispose()
            using var sceneData = new SceneData();
            try
            {
                CreatePrimitiveMesh(sceneData, info);
                Debug.Assert(sceneData.Data != IntPtr.Zero && sceneData.DataSize > 0);
                var data = new byte[sceneData.DataSize];
                Marshal.Copy(sceneData.Data, data, 0, sceneData.DataSize);
                geometry.FromRawData(data);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, $"failed to create {info.Type} primitive mesh.");
            }
        }
    }
}
