/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

namespace RenderBoy
{
    // System Component TypeIds
    inline constexpr const char* RenderBoySystemComponentTypeId = "{C010EC8E-7313-4837-8B07-553D1AA8CE72}";
    inline constexpr const char* RenderBoyEditorSystemComponentTypeId = "{C47D63D5-97AD-459F-B727-3C00624F3D4A}";

    // Module derived classes TypeIds
    inline constexpr const char* RenderBoyModuleInterfaceTypeId = "{2AAF8D5A-F907-401E-A566-C99A3D1FA26A}";
    inline constexpr const char* RenderBoyModuleTypeId = "{63E965D4-780C-4914-AE2C-CA13685F88CF}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* RenderBoyEditorModuleTypeId = RenderBoyModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* RenderBoyRequestsTypeId = "{92A1781E-91CB-4B7B-8717-8F7A42823626}";
} // namespace RenderBoy
