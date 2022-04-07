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
        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            SurfaceId = EngineAPI.CreateRenderSurface(hwndParent.Handle, _width, _height);
            Debug.Assert(ID.IsValid(SurfaceId));
            _renderWindowHandle = EngineAPI.GetWindowHandle(SurfaceId);
            Debug.Assert(_renderWindowHandle != IntPtr.Zero);
            return new HandleRef(this, _renderWindowHandle);
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            EngineAPI.RemoveRenderSurface(SurfaceId);
            SurfaceId = ID.INVALID_ID;
            _renderWindowHandle = IntPtr.Zero;
        }
    }
}
