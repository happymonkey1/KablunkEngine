using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kablunk
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector2
	{
		public static Vector2 Zero = new Vector2(0, 0);
		public static Vector2 Right = new Vector2(1, 0);
		public static Vector2 Left = new Vector2(-1, 0);
		public static Vector2 Up = new Vector2(0, 1);
		public static Vector2 Down = new Vector2(0, -1);

		public float X;
		public float Y;

		public Vector2(float scalar)
		{
			X = Y = scalar;
		}

		public Vector2(float x, float y)
		{
			X = x;
			Y = y;
		}

		public Vector2(Vector3 v)
		{
			X = v.X;
			Y = v.Y;
		}

		public float Length()
		{
			return Mathf.Sqrt(X * X + Y * Y);
		}

		public Vector2 Normalized()
		{
			float L = Length();
			float x = X;
			float y = Y;

			if (L > 0.0f)
			{
				x /= L;
				y /= L;
			}

			return new Vector2(x, y);
		}

		public void Normalize()
		{
			float L = Length();

			if (L > 0.0f)
			{
				X /= L;
				Y /= L;
			}
		}

		public float Distance(Vector2 other)
		{
			return Mathf.Sqrt(Mathf.Pow(other.X - X, 2) + Mathf.Pow(other.Y - Y, 2));
		}

		public static float Distance(Vector2 p1, Vector2 p2)
		{
			return Mathf.Sqrt(Mathf.Pow(p2.X - p1.X, 2) + Mathf.Pow(p2.Y - p1.Y, 2));
		}

		public static Vector2 Lerp(Vector2 p1, Vector2 p2, float maxDistanceDelta)
		{
			if (maxDistanceDelta < 0.0f)
				return p1;
			if (maxDistanceDelta > 1.0f)
				return p2;

			return p1 + ((p2 - p1) * maxDistanceDelta);
		}

		public static Vector2 operator *(Vector2 left, float scalar)
		{
			return new Vector2(left.X * scalar, left.Y * scalar);
		}

		public static Vector2 operator *(float scalar, Vector2 right)
		{
			return new Vector2(scalar * right.X, scalar * right.Y);
		}

		public static Vector2 operator *(Vector2 left, Vector2 right)
		{
			return new Vector2(left.X * right.X, left.Y * right.Y);
		}

		public static Vector2 operator /(Vector2 left, Vector2 right)
		{
			return new Vector2(left.X / right.X, left.Y / right.Y);
		}

		public static Vector2 operator /(Vector2 left, float scalar)
		{
			return new Vector2(left.X / scalar, left.Y / scalar);
		}

		public static Vector2 operator /(float scalar, Vector2 right)
		{
			return new Vector2(scalar / right.X, scalar / right.Y);
		}

		public static Vector2 operator +(Vector2 left, Vector2 right)
		{
			return new Vector2(left.X + right.X, left.Y + right.Y);
		}

		public static Vector2 operator +(Vector2 left, float right)
		{
			return new Vector2(left.X + right, left.Y + right);
		}

		public static Vector2 operator -(Vector2 left, Vector2 right)
		{
			return new Vector2(left.X - right.X, left.Y - right.Y);
		}

		public static Vector2 operator -(Vector2 vector)
		{
			return new Vector2(-vector.X, -vector.Y);
		}

		public override string ToString()
		{
			return "Vector2[" + X + ", " + Y + "]";
		}

	}
}
