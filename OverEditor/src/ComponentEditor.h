#pragma once

#include "UIElements.h"
#include "EditorLayer.h"

#include <OverEngine/Scene/Components.h>
#include <imgui/imgui.h>

namespace OverEditor
{
	// Base
	template <class T>
	void ComponentEditor(Entity entity, uint32_t typeID)
	{
		if (UIElements::BeginComponentEditor<T>(entity, entity.GetComponent<T>().GetName(), typeID))
			ImGui::TextUnformatted("No Overloaded function for this component found!");
	}

	// Overloads
	template <>
	void ComponentEditor<TransformComponent>(Entity entity, uint32_t typeID)
	{
		if (UIElements::BeginComponentEditor<TransformComponent>(entity, "Transform Component", typeID))
		{
			UIElements::BeginFieldGroup();

			static Vector3 positionDelta(0.0f);
			static Vector3 rotationDelta(0.0f);
			static Vector3 scaleDelta(0.0f);

			auto& tc = entity.GetComponent<TransformComponent>();
			Vector3 position = tc.GetLocalPosition();
			Vector3 rotation = tc.GetLocalEulerAngles();
			Vector3 scale = tc.GetLocalScale();

			if (UIElements::DragFloat3Field("Position", "##Position", glm::value_ptr(position), 0.2f))
				positionDelta += position - tc.GetLocalPosition();

			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				auto action = CreateRef<Vector3EditAction>(positionDelta, [entity]() mutable {
					return entity.GetComponent<TransformComponent>().GetLocalPosition();
				}, [entity](const auto& pos) mutable {
					return entity.GetComponent<TransformComponent>().SetLocalPosition(pos);
				});

				EditorLayer::Get().GetActionStack().Do(action, false);
				positionDelta = Vector3(0.0f);
			}

			tc.SetLocalPosition(position);

			if (UIElements::DragFloat3Field("Rotation", "##Rotation", glm::value_ptr(rotation), 0.2f))
				rotationDelta += rotation - tc.GetLocalEulerAngles();

			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				auto action = CreateRef<Vector3EditAction>(rotationDelta, [entity]() mutable {
					return entity.GetComponent<TransformComponent>().GetEulerAngles();
				}, [entity](const auto& rot) mutable {
					return entity.GetComponent<TransformComponent>().SetEulerAngles(rot);
				});

				EditorLayer::Get().GetActionStack().Do(action, false);
				rotationDelta = Vector3(0.0f);
			}

			tc.SetLocalEulerAngles(rotation);

			if (UIElements::DragFloat3Field("Scale", "##Scale", glm::value_ptr(scale), 0.2f))
				scaleDelta += scale - tc.GetLocalScale();

			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				auto action = CreateRef<Vector3EditAction>(scaleDelta, [entity]() mutable {
					return entity.GetComponent<TransformComponent>().GetLocalScale();
				}, [entity](const auto& scale) mutable {
					return entity.GetComponent<TransformComponent>().SetLocalScale(scale);
				});

				EditorLayer::Get().GetActionStack().Do(action, false);
				scaleDelta = Vector3(0.0f);
			}

			tc.SetLocalScale(scale);

			UIElements::EndFieldGroup();

			#if 0
			ImGui::Text("%i", transform.GetChangedFlags());

			ImGui::PushItemWidth(-1);
			ImGui::InputFloat4("", (float*)&tc.GetLocalToWorld()[0].x, "%.3f", ImGuiInputTextFlags_ReadOnly);
			ImGui::InputFloat4("", (float*)&tc.GetLocalToWorld()[1].x, "%.3f", ImGuiInputTextFlags_ReadOnly);
			ImGui::InputFloat4("", (float*)&tc.GetLocalToWorld()[2].x, "%.3f", ImGuiInputTextFlags_ReadOnly);
			ImGui::InputFloat4("", (float*)&tc.GetLocalToWorld()[3].x, "%.3f", ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemWidth();
			#endif
		}
	}

	template <>
	void ComponentEditor<SpriteRendererComponent>(Entity entity, uint32_t typeID)
	{
		if (UIElements::BeginComponentEditor<SpriteRendererComponent>(entity, "SpriteRenderer Component", typeID))
		{
			UIElements::BeginFieldGroup();

			auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

			UIElements::Color4Field("Tint", "##Tint", glm::value_ptr(spriteRenderer.Tint));
			UIElements::DragFloatField("AlphaClipThreshold", "##AlphaClipThreshold", &spriteRenderer.AlphaClipThreshold, 0.02f, 0.0f, 1.0f);
			UIElements::Texture2DField("Sprite", "##Sprite", spriteRenderer.Sprite);

			if (spriteRenderer.Sprite && spriteRenderer.Sprite->GetType() != TextureType::Placeholder)
			{
				UIElements::CheckboxField("Flip.x", "##Flip.x", &spriteRenderer.Flip.x);
				UIElements::CheckboxField("Flip.y", "##Flip.y", &spriteRenderer.Flip.y);

				UIElements::DragFloat2Field("Tiling", "##Tiling", glm::value_ptr(spriteRenderer.Tiling), 0.02f);
				UIElements::DragFloat2Field("Offset", "##Offset", glm::value_ptr(spriteRenderer.Offset), 0.02f);

				static UIElements::EnumValues wrappingValues = {
					{ 0, "None (Use texture default value)" }, { 1, "Repeat" },
					{ 2, "MirroredRepeat" }, { 3, "ClampToEdge" },{ 4, "ClampToBorder" }
				};
				UIElements::BasicEnum("Wrapping.x", "##Wrapping.x", wrappingValues, (int8_t*)&spriteRenderer.Wrapping.x);
				UIElements::BasicEnum("Wrapping.y", "##Wrapping.y", wrappingValues, (int8_t*)&spriteRenderer.Wrapping.y);

				static UIElements::EnumValues filteringValues = {
					{ 0, "None (Use texture default value)" }, { 1, "Point" }, { 2, "Linear" }
				};
				UIElements::BasicEnum("Filtering", "##Filtering", filteringValues, (int8_t*)&spriteRenderer.Filtering);

				UIElements::CheckboxField("OverrideTextureBorderColor (for ClampToBorder wrapping)",
					"##OverrideTextureBorderColor", &spriteRenderer.TextureBorderColor.first);

				if (spriteRenderer.TextureBorderColor.first)
					UIElements::Color4Field("TextureBorderColor (for ClampToBorder wrapping)",
						"##BorderColor", glm::value_ptr(spriteRenderer.TextureBorderColor.second));
			}

			UIElements::EndFieldGroup();
		}
	}

	template <>
	void ComponentEditor<CameraComponent>(Entity entity, uint32_t typeID)
	{
		if (UIElements::BeginComponentEditor<CameraComponent>(entity, "Camera Component", typeID))
		{
			UIElements::BeginFieldGroup();

			auto& camera = entity.GetComponent<CameraComponent>();

			float orthoSize = camera.Camera.GetOrthographicSize();

			Color clearColor = camera.Camera.GetClearColor();

			float OrthographicNearClip = camera.Camera.GetOrthographicNearClip();
			float OrthographicFarClip = camera.Camera.GetOrthographicFarClip();

			// TODO: Perspective Camera
			if (UIElements::DragFloatField("Orthographic Size", "##Orthographic Size", &orthoSize, 0.5f, 0.0001f, FLT_MAX))
				camera.Camera.SetOrthographicSize(orthoSize);

			if (UIElements::DragFloatField("OrthographicNearClip", "##OrthographicNearClip", &OrthographicNearClip, 0.5f))
				camera.Camera.SetOrthographicNearClip(OrthographicNearClip);
			if (UIElements::DragFloatField("OrthographicFarClip", "##OrthographicFarClip", &OrthographicFarClip, 0.5f))
				camera.Camera.SetOrthographicFarClip(OrthographicFarClip);

			if (UIElements::Color4Field("Clear Color", "##Clear Color", glm::value_ptr(clearColor)))
				camera.Camera.SetClearColor(clearColor);

			UIElements::CheckboxFlagsField<uint8_t>("Is Clearing Color", "##Is Clearing Color", (uint8_t*)&camera.Camera.GetClearFlags(), ClearFlags_ClearColor);
			UIElements::CheckboxFlagsField<uint8_t>("Is Clearing Depth", "##Is Clearing Depth", (uint8_t*)&camera.Camera.GetClearFlags(), ClearFlags_ClearDepth);

			UIElements::EndFieldGroup();
		}
	}
}
