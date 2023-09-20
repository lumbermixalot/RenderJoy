/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Atom/RPI.Reflect/Pass/PassTemplate.h>

namespace AZ
{
    namespace RPI
    {
        class PassSystemInterface;
    }
}

namespace RenderJoy
{
    //! This class provides services to create RenderJoy RPI::PassTemplates at runtime.
    class RenderJoyTemplatesFactory final
    {
    public:
        RenderJoyTemplatesFactory() = default;
        ~RenderJoyTemplatesFactory();

        static constexpr char LogName[] = "RenderJoyTemplatesFactory";

        // The parent pass name can be extracted from m_passRequest.
        struct ParentEntityTemplates
        {
            // At the very least a pipeline EntityId owns a Parent Pass, that has always one child pass
            // that renders something visible in the screen, typically a RenderJoyBillboardPass.
            AZStd::vector<AZ::Name> m_createdPassTemplates;
            AZStd::shared_ptr<AZ::RPI::PassRequest> m_passRequest;
            AZ::Name m_billboardPassName; 
        };

    private:
        friend class RenderJoySystemComponent;

        bool CreateRenderJoyParentPassRequest(AZ::RPI::PassSystemInterface* passSystem, AZ::EntityId parentPassEntityId, AZ::EntityId passBusEntity);
        
        // Removes all templates created on behalf of a pipeline EntityId
        void RemoveTemplates(AZ::RPI::PassSystemInterface* passSystem, AZ::EntityId parentPassEntityId);

        // Wholesale removes all RenderJoy related templates from the pass system.
        void RemoveAllTemplates(AZ::RPI::PassSystemInterface* passSystem);

        size_t GetParentPassCount() const;

        AZStd::vector<AZ::EntityId> GetParentPassEntities() const;

        AZStd::shared_ptr<AZ::RPI::PassRequest> GetParentPassRequest(AZ::EntityId parentPassEntityId) const;

        AZ::Name GetBillboardPassName(AZ::EntityId parentPassEntityId) const;

        AZ::Name GetShaderPassName(AZ::EntityId shaderPassEntityId) const;

        bool EntityHasActivePasses(AZ::RPI::PassSystemInterface* passSystem, AZ::EntityId parentPassEntityId) const;

        //////////////////////////////////////////////////////////////////////////
        // Helper Functions Start ...



        // //! A recursive function, creates all templates. The idea is that an entity that owns a RenderJoyPassEditorComponent
        // //! will have its own pass template registered with its own, unique name. Later we will use each created template
        // //! to issue pass requests and interconnect them based on the EntityId references.
        // bool CreateRenderJoyPassTemplates(AZ::RPI::PassSystemInterface* passSystem, AZ::EntityId currentPassEntity);
        // 
        // //! Recursive function that creates the interconnected pass requests.
        // bool CreateRenderJoyPassRequests(AZ::RPI::PassTemplate& parentPassTemplate, AZ::EntityId currentPassEntity);
        // 
        // AZStd::string GetPassNameFromEntityName(AZ::EntityId entityId);
        // AZStd::string GetPassTemplateNameFromEntityName(AZ::EntityId entityId);
        // 
        // AZ::EntityId m_outputPassEntity;
        // AZ::Name m_outputPassName;
        // AZ::Name m_outputPassTemplateName;


        // For each RenderJoy parent pass, we keep a list of the names of the children templates. 
        AZStd::unordered_map<AZ::EntityId, ParentEntityTemplates> m_parentEntities;
    };

} //namespace RenderJoy

