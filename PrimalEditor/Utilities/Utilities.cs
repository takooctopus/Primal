﻿using System;
using System.Windows.Threading;

namespace PrimalEditor.Utilities
{
    /// <summary>
    /// Utilities
    /// ID工具，注册非法ID值-1
    /// </summary>
    public static class ID
    {
        // TODO: 这里的数据类型可能会与C++中的不匹配
        public static int INVALID_ID => -1;
        public static bool IsValid(int id) => id != INVALID_ID;
    }
    /// <summary>
    /// 数学工具类
    /// </summary>
    public static class MathUtil
    {
        public static float Epsilon => 0.00001f;
        /// <summary>
        /// 提供低精度浮点数的等于比较方法
        /// </summary>
        /// <param name="value">The value.</param>
        /// <param name="other">The other.</param>
        /// <returns>
        ///   <c>true</c> if [is the same as] [the specified other]; otherwise, <c>false</c>.
        /// </returns>
        public static bool IsTheSameAs(this float value, float other)
        {
            return Math.Abs(value - other) < Epsilon;
        }
        /// <summary>
        /// 提供低精度浮点数[可null]的比较方法
        /// </summary>
        /// <param name="value">The value.</param>
        /// <param name="other">The other.</param>
        /// <returns>
        ///   <c>true</c> if [is the same as] [the specified other]; otherwise, <c>false</c>.
        /// </returns>
        public static bool IsTheSameAs(this float? value, float? other)
        {
            if (!value.HasValue || !other.HasValue) return false;
            return Math.Abs(value.Value - other.Value) < Epsilon;
        }
    }

    internal class DelayEventTimerArgs : EventArgs
    {
        public bool RepeatEvent { get; set; }
        public object Data { get; set; }
        public DelayEventTimerArgs(object data)
        {
            Data = data;
        }
    }
    internal class DelayEventTimer
    {
        private readonly DispatcherTimer _timer;
        private readonly TimeSpan _delay;
        private DateTime _lastEventTime = DateTime.Now;
        private object _data;

        public event EventHandler<DelayEventTimerArgs> Triggered;

        public void Trigger(object data = null)
        {
            _data = data;
            _lastEventTime = DateTime.Now;
            _timer.IsEnabled = true;
        }

        public void Disable()
        {
            _timer.IsEnabled = false;
        }
        private void OnTimerTick(object sender, EventArgs e)
        {
            if ((DateTime.Now - _lastEventTime) < _delay) return;
            var eventArgs = new DelayEventTimerArgs(_data);
            Triggered?.Invoke(this, eventArgs);
            _timer.IsEnabled = eventArgs.RepeatEvent;
        }

        public DelayEventTimer(TimeSpan delay, DispatcherPriority priority = DispatcherPriority.Normal)
        {
            _delay = delay;
            _timer = new DispatcherTimer(priority)
            {
                Interval = TimeSpan.FromMilliseconds(delay.TotalMilliseconds * 0.5)
            };
            _timer.Tick += OnTimerTick;
        }

    }
}
