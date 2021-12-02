using System.Runtime.InteropServices;

namespace Kablunk
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Transform
	{
		public Vector3 Position;
		public Vector3 Rotation;
		public Vector3 Scale;

		public Vector3 Up { get { return new Quaternion(Rotation) * Vector3.Up; } }

		public Vector3 Right { get { return new Quaternion(Rotation) * Vector3.Right; } }

		public Vector3 Forward { get { return new Quaternion(Rotation) * Vector3.Forward; } }

	}

}
