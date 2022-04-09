using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

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

        private bool _canResize = true;
        private bool _moved = false;


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

            // 因为对主程序窗口进行拖动缩放时会出现里面hosted window缩放事件，这里是为了解决逻辑不一致问题的
            var window = this.FindVisualParent<Window>();
            Debug.Assert(window != null);
            var helper = new WindowInteropHelper(window);
            if(helper.Handle != null)
            {
                HwndSource.FromHwnd(helper.Handle)?.AddHook(HwndMessageHook);
            }
        }

        private IntPtr HwndMessageHook(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            switch ((Win32Msg)msg)
            {
                case Win32Msg.WM_SIZING:
                    _canResize = false;
                    _moved = false;
                    break;
                case Win32Msg.WM_ENTERSIZEMOVE:
                    _moved = true;
                    break;
                case Win32Msg.WM_EXITSIZEMOVE:
                    _canResize = true;
                    if (!_moved)
                    {
                        _host.Resize();
                    }
                    break;
                default:
                    break;
            }
            return IntPtr.Zero;
        }
    

        private IntPtr HostMsgFilter(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            switch ((Win32Msg)msg)
            {
                case Win32Msg.WM_SIZE:
                    if(_canResize)
                    {
                        _host.Resize();
                    }
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
