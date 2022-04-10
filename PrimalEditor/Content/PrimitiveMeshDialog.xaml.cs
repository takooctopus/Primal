using PrimalEditor.ContentToolsAPIStruct;
using PrimalEditor.DllWrappers;
using PrimalEditor.Editors;
using PrimalEditor.Utilities.Controls;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace PrimalEditor.Content
{
    /// <summary>
    /// Interaction logic for PrimitiveMeshDialog.xaml [建立几何图形的dialog]
    /// </summary>
    public partial class PrimitiveMeshDialog : Window
    {

        private static readonly List<ImageBrush> _textures = new List<ImageBrush>();

        private void OnPrimitiveMeshType_ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e) => UpdatePrimitive();

        private void OnSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e) => UpdatePrimitive();

        private void OnScalarBox_ValueChanged(object sender, RoutedEventArgs e) => UpdatePrimitive();

        private float Value(ScalarBox scalarBox, float min)
        {
            float.TryParse(scalarBox.Value, out var result);
            return Math.Max(result, min);
        }
        private void UpdatePrimitive()
        {
            if (!IsInitialized) return;
            var primitiveType = (PrimitiveMeshType)PrimitiveMeshTypeComboBox.SelectedItem;
            var smoothingAnle = 0;
            var info = new PrimitiveInitInfo()
            {
                Type = primitiveType,
            };

            switch (primitiveType)
            {
                case PrimitiveMeshType.Plane:
                    {
                        info.SegmentX = (int)xSliderPlane.Value;
                        info.SegmentZ = (int)zSliderPlane.Value;
                        info.Size.X = Value(widthScalarBoxPlane, 0.001f);
                        info.Size.Z = Value(lengthScalarBoxPlane, 0.001f);
                        break;
                    }
                case PrimitiveMeshType.Cube:
                    return;
                case PrimitiveMeshType.UvSphere:
                    {
                        info.SegmentX = (int)xSliderUvSphere.Value;
                        info.SegmentY = (int)ySliderUvSphere.Value;
                        info.Size.X = Value(xScalarBoxUvSphere, 0.001f);
                        info.Size.Y = Value(yScalarBoxUvSphere, 0.001f);
                        info.Size.Z = Value(zScalarBoxUvSphere, 0.001f);
                        smoothingAnle = (int)angleSliderUvSphere.Value;
                        break;
                    }
                case PrimitiveMeshType.IcoSphere:
                    return;
                case PrimitiveMeshType.Cyclinder:
                    return;
                case PrimitiveMeshType.Capsule:
                    return;
                default:
                    return;/*throw new ArgumentOutOfRangeException(nameof(primitiveType));*/
            }
            var geometry = new Geometry();
            geometry.ImportSettings.SmoothingAngle = smoothingAnle;
            ContentToolsAPI.CreatePrimitiveMesh(geometry, info);
            (DataContext as GeometryEditor).SetAsset(geometry);
            OnTexture_CheckBox_Click(textureCheckBox, null);
        }

        private static void LoadTextures()
        {
            var uris = new List<Uri>
            {
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/PlaneTexture.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/A.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/UvTexture.png"),
            };
            _textures.Clear();
            foreach (var uri in uris)
            {
                var resource = Application.GetResourceStream(uri);
                using var reader = new BinaryReader(resource.Stream);
                var data = reader.ReadBytes((int)resource.Stream.Length);
                var imageSource = (BitmapSource)new ImageSourceConverter().ConvertFrom(data);
                imageSource.Freeze();
                var brush = new ImageBrush(imageSource);
                brush.Transform = new ScaleTransform(1, -1, 0.5, 0.5);
                brush.ViewportUnits = BrushMappingMode.Absolute;
                brush.Freeze();
                _textures.Add(brush);
            }
        }

        static PrimitiveMeshDialog()
        {
            LoadTextures();
        }

        public PrimitiveMeshDialog()
        {
            InitializeComponent();
            Loaded += (sender, args) => UpdatePrimitive();
        }

        private void OnTexture_CheckBox_Click(object sender, RoutedEventArgs e)
        {
            Brush brush = Brushes.White;
            if ((sender as CheckBox).IsChecked == true)
            {
                brush = _textures[(int)PrimitiveMeshTypeComboBox.SelectedItem];
            }

            var vm = DataContext as GeometryEditor;

            foreach (var mesh in vm.MeshRenderer.Meshes)
            {
                mesh.Diffuse = brush;
            }
        }
    }
}