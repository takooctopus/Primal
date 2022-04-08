using PrimalEditor.DllWrappers;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Interop;

namespace PrimalEditor.Utilities
{
    /// <summary>
    /// 窗口类,继承于【HwndHost】，用来生成窗口[实际是宿主窗口，里面保存一个win32窗口的句柄]
    /// </summary>
    /// <seealso cref="System.Windows.Interop.HwndHost" />
    internal class RenderSurfaceHost : HwndHost
    {
        private IntPtr _renderWindowHandle = IntPtr.Zero;
        private readonly int _width = 800;
        private readonly int _height = 600;
        private DelayEventTimer _resizeTimer;

        public int SurfaceId { get; private set; } = ID.INVALID_ID;

        public void Resize()
        {
            //Logger.Log(MessageType.Info, "Resizing Host Window");
            _resizeTimer.Trigger();
        }

        private void Resize(object sender, DelayEventTimerArgs e)
        {
            e.RepeatEvent = Mouse.LeftButton == MouseButtonState.Pressed;
            if (!e.RepeatEvent)
            {
                Logger.Log(MessageType.Info, "Host Window Resized");
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="RenderSurfaceHost"/> class.
        /// 初始化时设定宿主窗口的宽高
        /// </summary>
        /// <param name="width">The width.</param>
        /// <param name="height">The height.</param>
        public RenderSurfaceHost(double width, double height)
        {
            _width = (int)width;
            _height = (int)height;
            _resizeTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250.0));
            _resizeTimer.Triggered += Resize;
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
