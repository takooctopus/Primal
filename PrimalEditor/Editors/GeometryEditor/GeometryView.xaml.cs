using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;

namespace PrimalEditor.Editors
{
    /// <summary>
    /// Interaction logic for GeometryView.xaml
    /// </summary>
    public partial class GeometryView : UserControl
    {
        private static readonly GeometryView _geometryView = new GeometryView() { Background = (Brush)Application.Current.FindResource("Editor.Window.GrayBrush4")};

        private Point _clickedPosition;
        private bool _captureLeft;
        private bool _captureRight;

        private void SetGeometry(int index = -1)
        {
            if (!(DataContext is MeshRenderer vm)) return;
            if (vm.Meshes.Any() && viewport.Children.Count == 2)
            {
                // viewport第一个children是灯光，第二个是点集合，每次来新的，要把老的删了
                viewport.Children.RemoveAt(1);
            }
            var meshIndex = 0;
            var modelGroup = new Model3DGroup();
            foreach (var mesh in vm.Meshes)
            {
                // 跳过我们不想要的mesh
                if (index != -1 && meshIndex != index)
                {
                    ++meshIndex;
                    continue;
                }
                var mesh3D = new MeshGeometry3D()
                {
                    Positions = mesh.Positions,
                    Normals = mesh.Normals,
                    TriangleIndices = mesh.Indices,
                    TextureCoordinates = mesh.UVs
                };
                var diffuse = new DiffuseMaterial(mesh.Diffuse);
                var specular = new SpecularMaterial(mesh.Specular, 50); //镜面反射
                var matGroup = new MaterialGroup();
                matGroup.Children.Add(diffuse);
                matGroup.Children.Add(specular);

                var model = new GeometryModel3D(mesh3D, matGroup);
                modelGroup.Children.Add(model);

                var binding = new Binding(nameof(mesh.Diffuse)) { Source = mesh };
                BindingOperations.SetBinding(diffuse, DiffuseMaterial.BrushProperty, binding);

                if (meshIndex == index) break;
            }
            var visual = new ModelVisual3D() { Content = modelGroup };
            viewport.Children.Add(visual);
        }

        private void OnGrid_MouseMove(object sender, System.Windows.Input.MouseEventArgs e)
        {
            if (!_captureLeft && !_captureRight) return;
            var pos = e.GetPosition(this);
            var d = pos - _clickedPosition;

            if (_captureLeft && !_captureRight)
            {
                MoveCamera(d.X, d.Y, 0f);
            }
            else if (!_captureLeft && _captureRight)
            {
                var vm = DataContext as MeshRenderer;
                var cp = vm.CameraPosition;
                // 因为现在是xz平面，我们看y轴的偏移量，从上面看xz平面
                var yOffset = d.Y * 0.001 * Math.Sqrt(cp.X * cp.X + cp.Z * cp.Z);
                vm.CameraTarget = new Point3D(vm.CameraTarget.X, vm.CameraTarget.Y + yOffset, vm.CameraTarget.Z);
            }

            _clickedPosition = pos;
        }

        private void OnGrid_Mouse_LBD(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            _clickedPosition = e.GetPosition(this);
            _captureLeft = true;
            Mouse.Capture(sender as UIElement);
        }

        private void Grid_MouseWheel(object sender, System.Windows.Input.MouseWheelEventArgs e)
        {
            MoveCamera(0, 0, Math.Sign(e.Delta));
        }

        private void OnGrid_Mouse_LBU(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            _captureLeft = false;
            if (!_captureRight) Mouse.Capture(null);
        }

        private void OnGrid_Mouse_RBD(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            _clickedPosition = e.GetPosition(this);
            _captureRight = true;
            Mouse.Capture(sender as UIElement);
        }

        private void OnGrid_Mouse_RBU(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            _captureRight = false;
            if(!_captureLeft) Mouse.Capture(null);
        }

        /// <summary>
        /// 平移镜头
        /// </summary>
        /// <param name="dx"></param>
        /// <param name="dy"></param>
        /// <param name="dz"></param>
        private void MoveCamera(double dx, double dy, double dz)
        {
            var vm = DataContext as MeshRenderer;
            var v = new Vector3D(vm.CameraPosition.X, vm.CameraPosition.Y, vm.CameraPosition.Z);

            //  我们求在左手极坐标坐标系中的r,θ和φ角度【但是因为我们的标准坐标是右手坐标系的，这个会导致yz轴互换并使得z反号】
            var r = v.Length;   // 计算现在相机位置的半径r
            var theta = Math.Acos(v.Y / r); //θ角 竖向上的角
            var phi = Math.Atan2(-v.Z, v.X);  //Φ角 xz平面上的角 ()

            // 因为dxdy都很小
            theta -= dy * 0.01;
            phi -= dx * 0.01;
            r *= 1.0 - 0.1 * dz;

            theta = Math.Clamp(theta, 0.0001, Math.PI - 0.0001);

            v.X = r * Math.Sin(theta) * Math.Cos(phi);
            v.Z = -r * Math.Sin(theta) * Math.Sin(phi);
            v.Y = r * Math.Cos(theta);

            vm.CameraPosition = new Point3D(v.X, v.Y, v.Z);
        }

        /// <summary>
        /// 将这个view渲染成一张Icon【bitmap】
        /// </summary>
        /// <param name="meshRenderer"></param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <returns></returns>
        internal static BitmapSource RenderToBitmap(MeshRenderer meshRenderer, int width, int height)
        {
            var bmp = new RenderTargetBitmap(width, height, 96, 96, PixelFormats.Default);
            _geometryView.DataContext = meshRenderer;
            _geometryView.Width = width;
            _geometryView.Height = height;
            _geometryView.Measure(new Size(width, height));
            _geometryView.Arrange(new Rect(0, 0, width, height));
            _geometryView.UpdateLayout();

            bmp.Render(_geometryView);
            return bmp;
        }
        
        public GeometryView()
        {
            InitializeComponent();
            DataContextChanged += (s, e) => SetGeometry();
        }
    }
}
