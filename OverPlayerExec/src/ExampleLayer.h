#pragma once

#include "OverEngine/Core/Core.h"

class ExampleLayer : public OverEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
		m_VertexArray.reset(OverEngine::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		OverEngine::Ref<OverEngine::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(OverEngine::VertexBuffer::Create(vertices, sizeof(vertices)));

		OverEngine::BufferLayout layout = {
			{ OverEngine::ShaderDataType::Float3, "a_Position" },
			{ OverEngine::ShaderDataType::Float4, "a_Color" }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		OverEngine::Ref<OverEngine::IndexBuffer> indexBuffer;
		indexBuffer.reset(OverEngine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(OverEngine::VertexArray::Create());

		float squareVertices[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};

		OverEngine::Ref<OverEngine::VertexBuffer> squareVB;
		squareVB.reset(OverEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		squareVB->SetLayout({
			{ OverEngine::ShaderDataType::Float3, "a_Position" }
			});

		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		OverEngine::Ref<OverEngine::IndexBuffer> squareIB;
		squareIB.reset(OverEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		OverEngine::String vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = vec4(a_Position, 1.0);	
			}
		)";

		OverEngine::String fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				//color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

		m_Shader.reset(OverEngine::Shader::Create(vertexSrc, fragmentSrc));

		OverEngine::String blueShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);	
			}
		)";

		OverEngine::String blueShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		)";

		m_BlueShader.reset(OverEngine::Shader::Create(blueShaderVertexSrc, blueShaderFragmentSrc));
	}

	void OnAttach() override
	{
		std::stringstream ss = std::stringstream();
		ss << "Vendor: " << OverEngine::Application::Get().GetMainWindow().GetRendererContext()->GetInfoVendor();
		vendorInfo = ss.str();

		ss = std::stringstream();
		ss << "Renderer: " << OverEngine::Application::Get().GetMainWindow().GetRendererContext()->GetInfoRenderer();
		rendererInfo = ss.str();

		ss = std::stringstream();
		ss << "Version: " << OverEngine::Application::Get().GetMainWindow().GetRendererContext()->GetInfoVersion();
		versionInfo = ss.str();
	}

	void OnUpdate() override
	{
		OverEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		OverEngine::RenderCommand::Clear();

		OverEngine::Renderer::BeginScene();

		m_BlueShader->Bind();
		OverEngine::Renderer::Submit(m_SquareVA);

		m_Shader->Bind();
		OverEngine::Renderer::Submit(m_VertexArray);

		OverEngine::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		static bool show = true;
		if (show)
			ImGui::ShowDemoWindow(&show);

		ImGui::Begin("Renderer");

		ImGui::Text(vendorInfo.c_str());
		ImGui::Text(rendererInfo.c_str());
		ImGui::Text(versionInfo.c_str());

		ImGui::End();
	}

	void OnEvent(OverEngine::Event& event) override
	{
	}

private:
	OverEngine::String vendorInfo, rendererInfo, versionInfo;

	OverEngine::Ref<OverEngine::Shader> m_Shader;
	OverEngine::Ref<OverEngine::VertexArray> m_VertexArray;

	OverEngine::Ref<OverEngine::Shader> m_BlueShader;
	OverEngine::Ref<OverEngine::VertexArray> m_SquareVA;
};