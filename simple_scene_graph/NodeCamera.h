#pragma once
#include "Node.h"
#include "../Camera.h"

namespace MelonRenderer
{
	class NodeCamera :
		public Node
	{
	public:
		mat4 CalculateWorldTransform(const mat4& parentMat);
		void SetCamera(Camera* camera);

	protected:
		Camera* m_camera;
	};
}
