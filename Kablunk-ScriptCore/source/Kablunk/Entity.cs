using System;
using System.Runtime.CompilerServices;

namespace Kablunk
{
	public class Entity
    {
		protected Entity() { ID = 0; }

		internal Entity(ulong id)
        {
			ID = id;
        }

		~Entity() { }

		public ulong ID { get; private set; }
		public string Tag => GetComponent<TagComponent>().Tag;
		public Vector3 Translation
        {
			get { return GetComponent<TransformComponent>().Translation; }
            set { GetComponent<TransformComponent>().Translation = value; }
        }

		public Vector3 Rotation
		{
			get { return GetComponent<TransformComponent>().Rotation; }
			set { GetComponent<TransformComponent>().Rotation = value; }
		}

		public Vector3 Scale
		{
			get { return GetComponent<TransformComponent>().Scale; }
			set { GetComponent<TransformComponent>().Scale = value; }
		}

		public T CreateComponent<T>() where T : Component, new()
        {
			CreateComponent_Native(ID, typeof(T));
			T component = new T();
			component.Entity = this;
			return component;
        }

		public bool HasComponent<T>() where T : Component, new()
        {
			return HasComponent_Native(ID, typeof(T));
        }

		public bool HasComponent(Type type)
        {
			return HasComponent_Native(ID, type);
        }

		public T GetComponent<T>() where T : Component, new()
        {
			if (HasComponent<T>())
            {
				T component = new T();
				component.Entity = this;
				return component;
            }

			return null;
        }

		public bool Is<T>() where T : Entity, new()
        {
			CSharpScriptComponent sc = GetComponent<CSharpScriptComponent>();
			object instance = sc != null ? sc.Instance : null;
			return instance is T ? true : false;
        }

		public T As<T>() where T : Entity, new()
        {
			CSharpScriptComponent sc = GetComponent<CSharpScriptComponent>();
			object instance = sc != null ? sc.Instance : null;
			return instance is T ? instance as T : null;
		}

		public Entity FindEntityByTag(string tag)
        {
			ulong entity_id = FindEntityByTag_Native(tag);
			if (entity_id == 0)
				return null;

			return new Entity(entity_id);
        }

		public Entity FindEntityByID(ulong entity_id)
        {
			return new Entity(entity_id);
        }

		public Entity Create()
        {
			return new Entity(CreateEntity_Native());
        }

		public void Destroy()
        {
			DestroyEntity_Native(ID);
        }

		public void Destroy(Entity entity)
        {
			DestroyEntity_Native(entity.ID);
        }

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CreateComponent_Native(ulong entity_id, Type type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool HasComponent_Native(ulong entity_id, Type type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern ulong FindEntityByTag_Native(string tag);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern ulong CreateEntity_Native();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern ulong DestroyEntity_Native(ulong entity_id);
	}
}
