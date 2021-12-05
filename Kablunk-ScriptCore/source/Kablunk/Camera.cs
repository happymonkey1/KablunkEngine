using System;
using System.Runtime.CompilerServices;

namespace Kablunk
{
	public class Camera
	{
		public static Vector3 ScreenToWorldPosition(Vector2 screen_pos)
		{
			ScreenToWorldPosition_Native(ref screen_pos, out Vector3 world_position);
			return world_position;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void ScreenToWorldPosition_Native(ref Vector2 screen_pos, out Vector3 world_position);
	}
}
