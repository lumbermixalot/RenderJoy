        void FullscreenTrianglePassData::Reflect(ReflectContext* context)
        {
            if (auto* serializeContext = azrtti_cast<SerializeContext*>(context))
            {
                serializeContext->Class<FullscreenTrianglePassData, RenderPassData>()
                    ->Version(0)
                    ->Field("ShaderAsset", &FullscreenTrianglePassData::m_shaderAsset)
                    ->Field("StencilRef", &FullscreenTrianglePassData::m_stencilRef);
            }
        }