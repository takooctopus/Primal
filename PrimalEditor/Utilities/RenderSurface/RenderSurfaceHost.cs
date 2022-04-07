using PrimalEditor.DllWrappers;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Interop;

namespace PrimalEditor.Utilities
{
    /// <summary>
    /// 窗口类,继承于【HwndHost】，用来生成窗口
    /// </summary>
    /// <seealso cref="System.Windows.Interop.HwndHost" />
    internal class RenderSurfaceHost : HwndHost
    {
        private IntPtr _renderWindowHandle = IntPtr.Zero;
        private readonly int _width = 800;
        private readonly int _height = 600;

        public int SurfaceId { get; private set; } = ID.INVALID_ID;

        public RenderSurfaceHost(double width, double height)
        {
            _width = (int)width;
            _height = (int)height;
        }

        /// <summary>
        /// When overridden in a derived class, creates the window to be hosted.
        /// 调用EngineAPI创建窗口，返回handle引用入口
        /// </summary>
        /// <param name="hwndParent">The window handle of the parent window.</param>
        /// <returns>
        /// The handle to the child Win32 window to create.
        /// </returns>
        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            SurfaceId = EngineAPI.CreateRenderSurface(hwndParent.Handle, _width, _height);
            Debug.Assert(ID.IsValid(SurfaceId));
            _renderWindowHandle = EngineAPI.GetWindowHandle(SurfaceId);
            Debug.Assert(_renderWindowHandle != IntPtr.Zero);
            return new HandleRef(this, _renderWindowHandle);
        }

        /// <summary>
        /// When overridden in a derived class, destroys the hosted window.
        /// 退出时调用EngineApi删除界面
        /// </summary>
        /// <param name="hwnd">A structure that contains the window handle.</param>
        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            EngineAPI.RemoveRenderSurface(SurfaceId);
            SurfaceId = ID.INVALID_ID;
            _renderWindowHandle = IntPtr.Zero;
        }
    }
}
