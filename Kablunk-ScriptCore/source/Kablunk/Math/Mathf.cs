using System;

namespace Kablunk
{
	public static class Mathf
	{
		public const float PI = (float)Math.PI;
		public const float TwoPI = (float)(Math.PI * 2.0);

		public const float Deg2Rad = PI / 180.0f;
		public const float Rad2Deg = 180.0f / PI;

		public static float Clamp(float value, float min, float max)
		{
			if (value < min)
				return min;
			if (value > max)
				return max;
			return value;
		}
		public static float Pow(float x, float y) { return (float)Math.Pow(x, y); }
		public static float Sin(float value) { return (float)Math.Sin(value); }
		public static float Cos(float value) { return (float)Math.Cos(value); }
		public static float Max(float v1, float v2) { return v1 > v2 ? v1 : v2; }
		public static float Min(float v1, float v2) { return v1 < v2 ? v1 : v2; }
		public static float Sqrt(float value) { return (float)Math.Sqrt(value); }
		public static float Abs(float value) { return Math.Abs(value); }

		public static float Lerp(float p1, float p2, float t)
		{
			if (t < 0.0f)
				return p1;
			else if (t > 1.0f)
				return p2;
			else
				return p1 + ((p2 - p1) * t);
		}

		public static float Modulo(float a, float b)
		{
			return (a - b) * (float)(Math.Floor(a / b)); 
		}
	}
}
