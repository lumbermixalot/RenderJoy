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
    //! This class creates the complete render pipeline definition for RenderJoy.
    //! It instantiates a ParentPass that connects all RenderJoy passes defined
    //! by the user in the Editor Entity Outliner.
    class RenderJoyMainPassTemplateCreator final
    {
    public:
        static constexpr char LogName[] = "RenderJoyMainPassTemplateCreator";
        static constexpr char PassTemplateName[] = "RenderJoyMainPassTemplate";

        //! The caller is responsible for adding/removing the template to/from the PassSystem.
        //! This method simply creates the template according to how the user defined the entities
        //! in the Editor. The returned template must be added to the PassSystem before
        //! the RenderJoy pipeline is instantiated.
        bool Create(AZ::RPI::PassSystemInterface* passSystem);
        const AZ::Name& GetOutputPassTemplateName() { return m_outputPassTemplateName; }
        void RemoveTemplates(AZ::RPI::PassSystemInterface* passSystem);

    private:
        //! A recursive function, creates all templates. The idea is that an entity that owns a RenderJoyPassEditorComponent
        //! will have its own pass template registered with its own, unique name. Later we will use each created template
        //! to issue pass requests and interconnect them based on the EntityId references.
        bool CreateRenderJoyPassTemplates(AZ::RPI::PassSystemInterface* passSystem, AZ::EntityId currentPassEntity);

        //! Recursive function that creates the interconnected pass requests.
        bool CreateRenderJoyPassRequests(AZ::RPI::PassTemplate& parentPassTemplate, AZ::EntityId currentPassEntity);

        AZStd::string GetPassNameFromEntityName(AZ::EntityId entityId);
        AZStd::string GetPassTemplateNameFromEntityName(AZ::EntityId entityId);

        AZ::EntityId m_outputPassEntity;
        AZ::Name m_outputPassName;
        AZ::Name m_outputPassTemplateName;

        // With this we can easily now what templates We created, so we can remove them.
        AZStd::vector<AZ::Name> m_createdPassTemplates;

        // Used to provent a pass request from being created twice.
        AZStd::unordered_set<AZ::Name> m_passRequests;
    };

} //namespace RenderJoy

