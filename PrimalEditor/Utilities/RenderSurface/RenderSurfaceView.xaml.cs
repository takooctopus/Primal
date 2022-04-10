using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;

namespace PrimalEditor.Utilities
{
    /// <summary>
    /// Interaction logic for RenderSurfaceView.xaml
    /// </summary>
    public partial class RenderSurfaceView : UserControl, IDisposable
    {
        /// <summary>
        /// The host
        /// </summary>
        private RenderSurfaceHost _host = null;


        /// <summary>
        /// 枚举类，这个应该与C++里面的传递消息相同，去WinUser.h中去找
        /// </summary>
        private enum Win32Msg
        {
            WM_SIZE = 0x0005,
            WM_SIZING = 0x0214,
            WM_ENTERSIZEMOVE = 0x0231,
            WM_EXITSIZEMOVE = 0x0232,
        }

        public RenderSurfaceView()
        {
            InitializeComponent();
            Loaded += OnRenderSurfaceViewLoaded;
        }

        private void OnRenderSurfaceViewLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnRenderSurfaceViewLoaded;
            _host = new RenderSurfaceHost(ActualWidth, ActualHeight);
            // 添加一个钩子来处理内部resize问题
            _host.MessageHook += new HwndSourceHook(HostMsgFilter);
            Content = _host;
        }




        private IntPtr HostMsgFilter(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            switch ((Win32Msg)msg)
            {
                case Win32Msg.WM_SIZE:
                    break;
                case Win32Msg.WM_SIZING:
                    throw new Exception();
                case Win32Msg.WM_ENTERSIZEMOVE:
                    throw new Exception();
                case Win32Msg.WM_EXITSIZEMOVE:
                    throw new Exception();
                default:
                    break;
            }
            return IntPtr.Zero;
        }

        #region IDisposable support
        private bool disposedValue;
        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // 暴露界面
                    _host.Dispose();
                }
                disposedValue = true;
            }
        }
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
        #endregion
    }
}
