#pragma once
#include "Node.h"
#include "../Camera.h"

namespace MelonRenderer
{
	class NodeCamera :
		public Node
	{
	public:
		mat4 CalculateWorldTransform(const mat4& parentMat, uint32_t** handle) override;
		void SetCamera(Camera* camera);

	protected:
		Camera* m_camera;
	};
}
