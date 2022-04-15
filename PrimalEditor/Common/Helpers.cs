using System;
using System.Diagnostics;
using System.IO;
using System.Security.Cryptography;
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

        public static bool IsDirectory(string path)
        {
            try
            {
                return File.GetAttributes(path).HasFlag(FileAttributes.Directory);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
            return false;

        }

        public static bool IsOlder(this DateTime date, DateTime other) => date < other;

        /// <summary>
        /// 辅助函数，去掉文件名中的非法字符
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public static string SanitizeFileName(string name)
        {
            var path = new StringBuilder(name.Substring(0, name.LastIndexOf(Path.DirectorySeparatorChar) + 1));
            var file = new StringBuilder(name[(name.LastIndexOf(Path.DirectorySeparatorChar) + 1)..]);
            foreach(var c in Path.GetInvalidPathChars())
            {
                path.Replace(c, '_');
            }
            foreach (var c in Path.GetInvalidFileNameChars())
            {
                file.Replace(c, '_');
            }
            return path.Append(file).ToString();
        }

        /// <summary>
        /// 辅助函数，将缓冲区的数据计算hash
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="meshDataBegin"></param>
        /// <param name="meshDataSize"></param>
        /// <returns></returns>
        /// <exception cref="System.NotImplementedException"></exception>
        public static byte[] ComputeHash(byte[] data, int offset = 0, int count = 0)
        {
            if(data?.Length > 0)
            {
                using var sha256 = SHA256.Create();
                return sha256.ComputeHash(data, offset, count > 0 ? count : data.Length);
            }
            return null;
        }
    }
}
