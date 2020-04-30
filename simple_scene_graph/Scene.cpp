#include "Scene.h"

namespace MelonRenderer
{
	Scene::~Scene()
	{
		for (auto& drawable : m_drawables)
		{
			drawable.Fini();
		}
	}

	void Scene::UpdateInstanceTransforms()
	{
		typedef std::pair<Node*, mat4> NodeCall;

		std::stack<NodeCall> nodeCallStack;

		for (auto rootChild : m_rootChildren)
		{
			nodeCallStack.emplace(std::make_pair(rootChild, mat4(1.f)));
		}

		while (!nodeCallStack.empty())
		{
			NodeCall nodeCall = nodeCallStack.top();
			nodeCallStack.pop();

			uint32_t* drawableHandle = nullptr;
			auto parentMat = nodeCall.first->CalculateWorldTransform(nodeCall.second, &drawableHandle);
			if (drawableHandle != nullptr)
			{
				m_drawableInstances[*drawableHandle].m_transformation = parentMat;
				m_drawableInstances[*drawableHandle].m_transformationInverseTranspose = glm::inverse(glm::transpose(parentMat));
			}
			for (auto child : nodeCall.first->m_children)
			{
				nodeCallStack.emplace(std::make_pair(child, parentMat));
			}
		}
	}

	uint32_t Scene::CreateDrawableInstance(uint32_t drawableHandle, bool isStatic)
	{
		m_drawableInstances.emplace_back(DrawableInstance{ drawableHandle, 0 });
		m_drawableInstanceIsStatic.emplace_back(isStatic);

		return m_drawableInstances.size() - 1;
	}
}