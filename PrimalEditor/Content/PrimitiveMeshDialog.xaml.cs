using PrimalEditor.ContentToolsAPIStruct;
using PrimalEditor.DllWrappers;
using PrimalEditor.Editors;
using PrimalEditor.Utilities.Controls;
using System;
using System.Windows;
using System.Windows.Controls;

namespace PrimalEditor.Content
{
    /// <summary>
    /// Interaction logic for PrimitiveMeshDialog.xaml [建立几何图形的dialog]
    /// </summary>
    public partial class PrimitiveMeshDialog : Window
    {

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
            var info = new PrimitiveInitInfo()
            {
                Type = primitiveType,
            };

            switch (primitiveType)
            {
                case PrimitiveMeshType.Plane:
                    {
                        info.SegmentX = (int)xSliderPlane.Value;
                        info.SegmentZ = (int)xSliderPlane.Value;
                        info.Size.X = Value(widthScalarBoxPlane, 0.001f);
                        info.Size.Z = Value(lengthScalarBoxPlane, 0.001f);
                        break;
                    }
                case PrimitiveMeshType.Cube:
                    break;
                case PrimitiveMeshType.UvSphere:
                    break;
                case PrimitiveMeshType.IcoSphere:
                    break;
                case PrimitiveMeshType.Cyclinder:
                    break;
                case PrimitiveMeshType.Capsule:
                    break;
                default:
                    throw new ArgumentOutOfRangeException(nameof(primitiveType));
            }
            var geometry = new Geometry();
            ContentToolsAPI.CreatePrimitiveMesh(geometry, info);
            (DataContext as GeometryEditor).SetAsset(geometry);
        }

        public PrimitiveMeshDialog()
        {
            InitializeComponent();
            Loaded += (sender, args) => UpdatePrimitive();
        }
    }
}