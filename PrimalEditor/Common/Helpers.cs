using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace PrimalEditor
{
    static class VisualExtensions
    {
        /// <summary>
        /// Finds the visual parent.找到视觉树中的父级
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="depObject">The dep object.</param>
        /// <returns></returns>
        public static T FindVisualParent<T>(this DependencyObject depObject) where T : DependencyObject
        {
            if (!(depObject is Visual)) return null;
            var parent = VisualTreeHelper.GetParent(depObject);
            while (parent != null)
            {
                if (parent is T type)
                {
                    return type;
                }
                parent = VisualTreeHelper.GetParent(parent);
            }
            return null;
        }
    }

    public static class ContentHelper
    {
        /// <summary>
        /// 根据长度返回名称，最低8位
        /// </summary>
        /// <param name="length">The length.</param>
        /// <returns></returns>
        public static string GetRandomString(int length = 8)
        {
            if (length <= 0) length = 8;
            var n = length / 11;
            var sb = new StringBuilder(n);
            for (int i = 0; i <= n; i++)
            {
                sb.Append(Path.GetRandomFileName().Replace(".", "")); //因为GetRandomFileName()返回的字符串去掉小数点是11位的
            }
            return sb.ToString(0, length);
        }
    }
}
