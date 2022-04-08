using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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
            while(parent != null)
            {
                if(parent is T type)
                {
                    return type;
                }
                parent = VisualTreeHelper.GetParent(parent);
            }
            return null;
        }
    }
}
