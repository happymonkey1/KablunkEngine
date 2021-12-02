using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Kablunk
{
	public abstract class Component
	{
		public Entity Entity { get; set; }
	}

	public class TagComponent : Component
	{
		public string Tag
		{
			get
			{
				return GetTag_Native(Entity.ID);
			}
			set
			{
				SetTag_Native(Entity.ID, value);
			}
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern string GetTag_Native(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void SetTag_Native(ulong entityID, string tag);

	}

	public class TransformComponent : Component
	{
		/// <summary>
		/// Transform relative to parent entity
		/// </summary>
		public Transform Transform
		{
			get
			{
				GetTransform_Native(Entity.ID, out Transform result);
				return result;
			}

			set
			{
				SetTransform_Native(Entity.ID, ref value);
			}
		}

		public Vector3 Translation
		{
			get
			{
				GetTranslation_Native(Entity.ID, out Vector3 result);
				return result;
			}

			set
			{
				SetTranslation_Native(Entity.ID, ref value);
			}
		}

		public Vector3 Rotation
		{
			get
			{
				GetRotation_Native(Entity.ID, out Vector3 result);
				return result;
			}

			set
			{
				SetRotation_Native(Entity.ID, ref value);
			}
		}

		public Vector3 Scale
		{
			get
			{
				GetScale_Native(Entity.ID, out Vector3 result);
				return result;
			}

			set
			{
				SetScale_Native(Entity.ID, ref value);
			}
		}


		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void GetTransform_Native(ulong entityID, out Transform outTransform);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SetTransform_Native(ulong entityID, ref Transform inTransform);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void GetTranslation_Native(ulong entityID, out Vector3 outTranslation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SetTranslation_Native(ulong entityID, ref Vector3 inTranslation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void GetRotation_Native(ulong entityID, out Vector3 outRotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SetRotation_Native(ulong entityID, ref Vector3 inRotation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void GetScale_Native(ulong entityID, out Vector3 outScale);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void SetScale_Native(ulong entityID, ref Vector3 inScale);
	}

	public class CameraComponent : Component
	{
		// #TODO implement
	}

	public class CSharpScriptComponent : Component
	{
		public object Instance
		{
			get => GetInstance_Native(Entity.ID);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern object GetInstance_Native(ulong entityID);
	}

	public class SpriteRendererComponent : Component
	{
		// #TODO implement
	}

	public enum RigidBody2DBodyType
	{
		Static, Dynamic, Kinematic
	}

	public class RigidBody2DComponent : Component
	{
		// #TODO implement
	}

	public class BoxCollider2DComponent : Component
	{
		// #TODO implement
	}
}
