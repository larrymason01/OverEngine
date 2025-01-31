#pragma once

#include "Entity.h"
#include "Scene.h"

#include "OverEngine/Core/Core.h"
#include "OverEngine/Core/Object.h"
#include "OverEngine/Core/Math/Math.h"
#include "OverEngine/Core/Random.h"

#include "OverEngine/Renderer/Texture.h"

#include "OverEngine/Physics/RigidBody2D.h"
#include "OverEngine/Physics/Collider2D.h"

#include "OverEngine/Scene/SceneCamera.h"
#include "OverEngine/Scene/ScriptableEntity.h"

namespace OverEngine
{
	struct Component : public Object
	{
		OE_CLASS_NO_REFLECT_PUBLIC(Component, Object)

		Entity AttachedEntity;
		bool Enabled = true;

		Component(const Entity& attachedEntity, bool enabled = true)
			: AttachedEntity(attachedEntity), Enabled(enabled) {}
	};

	////////////////////////////////////////////////////////
	/// Common Components //////////////////////////////////
	////////////////////////////////////////////////////////

	struct NameComponent : public Component
	{
		OE_CLASS_NO_REFLECT_PUBLIC(NameComponent, Component)

		String Name;

		NameComponent(const NameComponent&) = default;
		NameComponent(const Entity& entity, const String& name)
			: Component(entity), Name(name) {}
	};

	struct IDComponent : public Component
	{
		OE_CLASS_NO_REFLECT_PUBLIC(IDComponent, Component)

		uint64_t ID = Random::UInt64();

		IDComponent(const IDComponent&) = default;
		IDComponent(const Entity& entity, const uint64_t& id)
			: Component(entity), ID(id) {}
	};

	////////////////////////////////////////////////////////
	/// Renderer Components ////////////////////////////////
	////////////////////////////////////////////////////////

	struct CameraComponent : public Component
	{
		OE_CLASS_NO_REFLECT_PUBLIC(CameraComponent, Component)

		SceneCamera Camera;
		bool FixedAspectRatio = false;

		CameraComponent(const CameraComponent&) = default;

		CameraComponent(const Entity& entity, const SceneCamera& camera)
			: Component(entity), Camera(camera) {}

		CameraComponent(const Entity& entity)
			: Component(entity) {}
	};

	struct SpriteRendererComponent : public Component
	{
		OE_CLASS_PUBLIC(SpriteRendererComponent, Component)

		Color Tint = Color(1.0f);

		Ref<Texture2D> Sprite;

		Vector2 Tiling = Vector2(1.0f);
		Vector2 Offset = Vector2(0.0f);
		TextureFlip Flip = TextureFlip_None;

		// Useful for SubTextures
		bool ForceTile = false;

		SpriteRendererComponent(const SpriteRendererComponent&) = default;

		SpriteRendererComponent(const Entity& entity, Ref<Texture2D> sprite = nullptr)
			: Component(entity), Sprite(sprite) {}

		SpriteRendererComponent(const Entity& entity, Ref<Texture2D> sprite, const Color& tint)
			: Component(entity), Sprite(sprite), Tint(tint) {}
	};

	////////////////////////////////////////////////////////
	/// Physics Components /////////////////////////////////
	////////////////////////////////////////////////////////

	struct RigidBody2DComponent : public Component
	{
		OE_CLASS_NO_REFLECT_PUBLIC(RigidBody2DComponent, Component)

		Ref<RigidBody2D> RigidBody = nullptr;

		RigidBody2DComponent(const RigidBody2DComponent& other)
			: Component(other.AttachedEntity), RigidBody(other.RigidBody) {}

		RigidBody2DComponent(RigidBody2DComponent&& other)
			: Component(other.AttachedEntity)
		{
			if (other.RigidBody && !other.RigidBody->IsDeployed())
				RigidBody = other.RigidBody;
		}

		void operator=(RigidBody2DComponent&& other)
		{
			AttachedEntity = other.AttachedEntity;
			if (other.RigidBody && !other.RigidBody->IsDeployed())
				RigidBody = other.RigidBody;
		}

		void operator=(const RigidBody2DComponent& other)
		{
			AttachedEntity = other.AttachedEntity;
			RigidBody = RigidBody2D::Create(other.RigidBody->GetProps());
		}

		RigidBody2DComponent(const Entity& entity, const RigidBody2DProps& props = RigidBody2DProps())
			: Component(entity), RigidBody(RigidBody2D::Create(props)) {}

		~RigidBody2DComponent()
		{
			if (RigidBody && RigidBody->IsDeployed())
				RigidBody->UnDeploy();
		}
	};

	// Store's all colliders attached to an Entity
	struct Colliders2DComponent : public Component
	{
		OE_CLASS_NO_REFLECT_PUBLIC(Colliders2DComponent, Component)

		Vector<Ref<Collider2D>> Colliders;

		Colliders2DComponent(const Colliders2DComponent& other)
			: Component(other.AttachedEntity)
		{
			for (const auto& collider : other.Colliders)
			{
				Colliders.push_back(Collider2D::Create(collider->GetProps()));
			}
		}

		Colliders2DComponent(const Entity& entity)
			: Component(entity) {}
	};

	////////////////////////////////////////////////////////
	/// Script Components //////////////////////////////////
	////////////////////////////////////////////////////////

	struct NativeScriptsComponent : public Component
	{
		OE_CLASS_NO_REFLECT_PUBLIC(NativeScriptsComponent, Component)

		struct ScriptData
		{
			ScriptableEntity* Instance = nullptr;

			std::function<ScriptableEntity*()> InstantiateScript;
			void (*DestroyScript)(ScriptData*);

			~ScriptData()
			{
				if (Instance)
					DestroyScript(this);
			}
		};

		bool Runtime = false;
		UnorderedMap<size_t, ScriptData> Scripts;

		// Don't copy 'Scripts' to other instance when `Runtime` is true; since it will lead to double free on entt::registry::destroy
		NativeScriptsComponent(const NativeScriptsComponent& other)
			: Component(other.AttachedEntity)
		{
			if (!other.Runtime)
				Scripts = other.Scripts;
		}

		NativeScriptsComponent(const Entity& entity)
			: Component(entity) {}

		template<typename T, typename... Args>
		void AddScript(Args&&... args)
		{
			auto hash = typeid(T).hash_code();

			if (HasScript(hash))
			{
				OE_CORE_ASSERT(false, "Script of `typeid().hash_code() = {}` is already attached to Entity {0:x} (named `{}`)!", hash, AttachedEntity.GetComponent<IDComponent>().ID, AttachedEntity.GetComponent<NameComponent>().Name);
				return;
			}

			Scripts[hash] = ScriptData{
				nullptr,

				// TODO: Use C++20 features here https://stackoverflow.com/a/49902823/11814750
				[args = std::make_tuple(std::forward<Args>(args)...)]() mutable
				{
					return std::apply([](auto&& ... args) {
						return static_cast<ScriptableEntity*>(new T(std::forward<Args>(args)...));
					}, std::move(args));
				},

				[](ScriptData* s) { delete s->Instance; s->Instance = nullptr; }
			};

			if (Runtime)
			{
				auto& script = Scripts.at(hash);
				script.Instance = script.InstantiateScript();
			}
		}

		template<typename T>
		inline T& GetScript() { return *((T*)Scripts[typeid(T).hash_code()].Instance); }

		template<typename T>
		inline bool HasScript() { return HasScript(typeid(T).hash_code()); }

		template<typename T>
		inline void RemoveScript() { RemoveScript(typeid(T).hash_code()); }

		inline bool HasScript(size_t hash) const { return Scripts.count(hash); }
		void RemoveScript(size_t hash);
	};
}
