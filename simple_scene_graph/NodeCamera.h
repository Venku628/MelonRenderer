#pragma once
#include "Node.h"
#include "../Camera.h"

namespace MelonRenderer
{
	class NodeCamera :
		public Node
	{
	public:
		void Tick(PipelineData* pipelineData, const mat4& parentMat) override;
		void Tick(PipelineData* pipelineData) override;

		void SetCamera(Camera* camera);

	protected:
		Camera* m_camera;
	};
}
