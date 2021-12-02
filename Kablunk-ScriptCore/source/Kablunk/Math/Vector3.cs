using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace Kablunk
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3 : IEquatable<Vector3>
	{
		public static Vector3 Zero = new Vector3(0, 0, 0);
		public static Vector3 Forward = new Vector3(0, 0, -1);
		public static Vector3 Back = new Vector3(0, 0, 1);
		public static Vector3 Right = new Vector3(1, 0, 0);
		public static Vector3 Left = new Vector3(-1, 0, 0);
		public static Vector3 Up = new Vector3(0, 1, 0);
		public static Vector3 Down = new Vector3(0, -1, 0);

		public float X;
		public float Y;
		public float Z;
		public Vector3(float scalar)
		{
			X = Y = Z = scalar;
		}

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public Vector3(float x, Vector2 yz)
		{
			X = x;
			Y = yz.X;
			Z = yz.Y;
		}

		public Vector3(Vector2 vector)
		{
			X = vector.X;
			Y = vector.Y;
			Z = 0.0f;
		}

		public Vector3(Vector4 vector)
		{
			X = vector.X;
			Y = vector.Y;
			Z = vector.Z;
		}

		public void Clamp(Vector3 min, Vector3 max)
		{
			X = Mathf.Clamp(X, min.X, max.X);
			Y = Mathf.Clamp(Y, min.Y, max.Y);
			Z = Mathf.Clamp(Z, min.Z, max.Z);
		}

		public float Length()
		{
			return Mathf.Sqrt(X * X + Y * Y + Z * Z);
		}

		public Vector3 Normalized()
		{
			float L = Length();
			float x = X;
			float y = Y;
			float z = Z;

			if (L > 0.0f)
			{
				x /= L;
				y /= L;
				z /= L;
			}

			return new Vector3(x, y, z);
		}

		public void Normalize()
		{
			float L = Length();

			if (L > 0.0f)
			{
				X = X / L;
				Y = Y / L;
				Z = Z / L;
			}
		}

		public float Distance(Vector3 other)
		{
			return Mathf.Sqrt(Mathf.Pow(other.X - X, 2) + Mathf.Pow(other.Y - Y, 2) + Mathf.Pow(other.Z - Z, 2));
		}

		public static float Distance(Vector3 p1, Vector3 p2)
		{
			return Mathf.Sqrt(Mathf.Pow(p2.X - p1.X, 2) + Mathf.Pow(p2.Y - p1.Y, 2) + Mathf.Pow(p2.Z - p1.Z, 2));
		}

		public static Vector3 Lerp(Vector3 p1, Vector3 p2, float maxDistanceDelta)
		{
			if (maxDistanceDelta < 0.0f)
				return p1;
			if (maxDistanceDelta > 1.0f)
				return p2;

			return p1 + ((p2 - p1) * maxDistanceDelta);
		}

		public static Vector3 operator *(Vector3 left, float scalar)
		{
			return new Vector3(left.X * scalar, left.Y * scalar, left.Z * scalar);
		}

		public static Vector3 operator *(float scalar, Vector3 right)
		{
			return new Vector3(scalar * right.X, scalar * right.Y, scalar * right.Z);
		}

		public static Vector3 operator *(Vector3 left, Vector3 right)
		{
			return new Vector3(left.X * right.X, left.Y * right.Y, left.Z * right.Z);
		}

		public static Vector3 operator /(Vector3 left, Vector3 right)
		{
			return new Vector3(left.X / right.X, left.Y / right.Y, left.Z / right.Z);
		}

		public static Vector3 operator /(Vector3 left, float scalar)
		{
			return new Vector3(left.X / scalar, left.Y / scalar, left.Z / scalar);
		}

		public static Vector3 operator /(float scalar, Vector3 right)
		{
			return new Vector3(scalar / right.X, scalar / right.Y, scalar / right.Z);
		}

		public static Vector3 operator +(Vector3 left, Vector3 right)
		{
			return new Vector3(left.X + right.X, left.Y + right.Y, left.Z + right.Z);
		}

		public static Vector3 operator +(Vector3 left, float right)
		{
			return new Vector3(left.X + right, left.Y + right, left.Z + right);
		}

		public static Vector3 operator -(Vector3 left, Vector3 right)
		{
			return new Vector3(left.X - right.X, left.Y - right.Y, left.Z - right.Z);
		}

		public static Vector3 operator -(Vector3 vector)
		{
			return new Vector3(-vector.X, -vector.Y, -vector.Z);
		}

		public static Vector3 Cross(Vector3 x, Vector3 y)
		{
			return new Vector3(
				x.Y * y.Z - y.Y * x.Z,
				x.Z * y.X - y.Z * x.X,
				x.X * y.Y - y.X * x.Y
			);
		}

		public override bool Equals(object obj) => obj is Vector3 other && this.Equals(other);

		public bool Equals(Vector3 right) => X == right.X && Y == right.Y && Z == right.Z;

		public override int GetHashCode() => (X, Y, Z).GetHashCode();

		public static bool operator ==(Vector3 left, Vector3 right) => left.Equals(right);

		public static bool operator !=(Vector3 left, Vector3 right) => !(left == right);

		public static Vector3 Cos(Vector3 vector)
		{
			return new Vector3((float)Math.Cos(vector.X), (float)Math.Cos(vector.Y), (float)Math.Cos(vector.Z));
		}

		public static Vector3 Sin(Vector3 vector)
		{
			return new Vector3((float)Math.Sin(vector.X), (float)Math.Sin(vector.Y), (float)Math.Sin(vector.Z));
		}

		public override string ToString()
		{
			return "Vector3[" + X + ", " + Y + ", " + Z + "]";
		}

		public Vector2 XY
		{
			get { return new Vector2(X, Y); }
			set { X = value.X; Y = value.Y; }
		}
		public Vector2 XZ
		{
			get { return new Vector2(X, Z); }
			set { X = value.X; Z = value.Y; }
		}
		public Vector2 YZ
		{
			get { return new Vector2(Y, Z); }
			set { Y = value.X; Z = value.Y; }
		}

	}
}
