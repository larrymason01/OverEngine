#pragma once

#include "OverEngine/Core/Time/TimeStep.h"
#include "OverEngine/Core/Random.h"
#include "OverEngine/Physics/PhysicsWorld2D.h"
#include "OverEngine/Core/AssetManagement/Asset.h"

#include <entt.hpp>

namespace OverEngine
{
	class Entity;

	struct Physics2DSettings
	{
		Vector2 gravity = Vector2(0.0f, -9.8f);
	};

	struct SceneSettings
	{
		Physics2DSettings physics2DSettings;
	};

	class SceneSerializer;

	class Scene : public Asset
	{
		OE_CLASS_NO_REFLECT(Scene, Asset)

	public:
		Scene(const SceneSettings& settings = SceneSettings());
		Scene(Scene& other);
		~Scene();

		inline virtual bool IsReference() const override { return false; }

		Entity CreateEntity(const String& name = String(), uint64_t uuid = Random::UInt64());
		Entity CreateEntity(Entity& parent, const String& name = String(), uint64_t uuid = Random::UInt64());

		void OnUpdate(TimeStep deltaTime);
		void OnPhysicsUpdate(TimeStep deltaTime);
		void OnScriptsUpdate(TimeStep deltaTime);

		void OnScenePlay();
		void InitializePhysics();
		void InitializeScripts();

		// Rendering
		bool OnRender();
		void RenderSprites();
		void SetViewportSize(uint32_t width, uint32_t height);

		inline PhysicsWorld2D& GetPhysicsWorld2D() { return *m_PhysicsWorld2D; }
		inline const PhysicsWorld2D& GetPhysicsWorld2D() const { return *m_PhysicsWorld2D; }

		inline const Vector<entt::entity>& GetRootHandles() const { return m_RootHandles; }
		inline Vector<entt::entity>& GetRootHandles() { return m_RootHandles; }

		inline uint32_t GetEntityCount() const { return (uint32_t)m_Registry.alive(); }

		inline bool Exists(const entt::entity& entity) { return m_Registry.valid(entity); }
		
		void HandleCollision(const Collision2D& collision, bool enter);
		void OnCollisionEnter(const Collision2D& collision);
		void OnCollisionExit(const Collision2D& collision);

	private:
		template<typename T>
		void CopyComponentsFrom(Scene& src)
		{
			auto view = src.m_Registry.view<T>();
			m_Registry.insert<T>(view.data(), view.data() + view.size(), view.raw(), view.raw() + view.size());

			m_Registry.view<T>().each([this](entt::entity entity, T& component)
			{
				component.AttachedEntity = { entity, this };
			});
		}

	private:
		entt::registry m_Registry;
		PhysicsWorld2D* m_PhysicsWorld2D = nullptr;

		// To set viewport size for new camera components
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		/**
		 * To hold entities with entt::null parent.
		 * Useful for drawing graphs / trees.
		 */
		Vector<entt::entity> m_RootHandles;
		UnorderedMap<entt::entity, Vector<entt::id_type>> m_ComponentList;

		friend class Entity;
		friend class SceneSerializer;
	};
}
