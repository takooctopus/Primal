using System;
using System.Diagnostics;

namespace PrimalEditor.Components
{
    /// <summary>
    /// 枚举类， 是组件类型，以后每次添加新类，都要在里面添加
    /// </summary>
    enum ComponentType
    {
        Transform,
        Script
    }
    internal static class ComponentFactory
    {
        /// <summary>
        /// 构建方法列表，其顺序和ComponentType中枚举顺序应该一致，添加新方法的时候要注意
        /// </summary>
        private static readonly Func<GameEntity, object, Component>[] _function =
            new Func<GameEntity, object, Component>[]
        {
            (entity, data) => new Transform(entity),
            (entity, data) => new Script(entity){Name = (string) data},
        };

        /// <summary>
        /// 对外的工厂方法，根据传入的组件类型[枚举元素]来获取对应的创造方法
        /// </summary>
        /// <param name="componentType">Type of the component.</param>
        /// <returns></returns>
        public static Func<GameEntity, object, Component> GetCreationFunction(ComponentType componentType)
        {
            Debug.Assert((int)componentType < _function.Length);
            return _function[(int)componentType];
        }

        /// <summary>
        /// 根据组件类来返回其在枚举类中对应的index
        /// </summary>
        /// <param name="component">The component.</param>
        /// <returns></returns>
        public static ComponentType toEnumType(this Component component)
        {
            return component switch
            {
                Transform => ComponentType.Transform,
                Script => ComponentType.Script,
                _ => throw new ArgumentException("Unknown component type.")
            };
        }
    }
}
