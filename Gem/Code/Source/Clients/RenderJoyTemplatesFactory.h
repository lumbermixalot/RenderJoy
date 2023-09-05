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

    private:
        friend class RenderJoySystemComponent;

        static constexpr char LogName[] = "RenderJoyTemplatesFactory";
        static constexpr char BillboardPassTemplateBaseName[] = "RenderJoyBillboardPassTemplate";


        //! This method creates a pass template that renders a billboard with a texture that shows the user
        //! the pipeline is invalid. The template is automatically added to the pass system.
        AZ::RPI::PassTemplate CreateInvalidRenderJoyParentPassTemplate(AZ::RPI::PassSystemInterface* passSystem, const AZStd::string& name);

        void RemoveTemplate(AZ::RPI::PassSystemInterface* passSystem, const AZ::Name& templateName);

        // Wholesale removes all RenderJoy related templates from the pass system.
        void RemoveAllTemplates(AZ::RPI::PassSystemInterface* passSystem);

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

        // With this we can easily know what templates We created, so we can remove them.
        AZStd::vector<AZ::Name> m_createdPassTemplates;

        // Used to prevent a pass request from being created twice.
        AZStd::unordered_set<AZ::Name> m_passRequests;
    };

} //namespace RenderJoy

