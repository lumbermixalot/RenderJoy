#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(FILES
    Include/RenderJoy/RenderJoyCommon.h
    Include/RenderJoy/RenderJoyTextureProviderBus.h
    Include/RenderJoy/RenderJoyPassBus.h
    Include/RenderJoy/IRenderJoySrgDataProvider.h
    Source/Tools/EditorRenderJoySystemComponent.cpp
    Source/Tools/EditorRenderJoySystemComponent.h
    Source/Tools/Components/EditorRenderJoyBillboardComponent.cpp
    Source/Tools/Components/EditorRenderJoyBillboardComponent.h
    Source/Tools/Components/EditorRenderJoyShaderComponent.cpp
    Source/Tools/Components/EditorRenderJoyShaderComponent.h
    Source/Tools/Components/EditorRenderJoyTextureComponent.cpp
    Source/Tools/Components/EditorRenderJoyTextureComponent.h
    Source/Tools/Components/EditorRenderJoyKeyboardComponent.cpp
    Source/Tools/Components/EditorRenderJoyKeyboardComponent.h
    Source/Tools/Utils/ITextureWriter.h
    Source/Tools/Utils/ITextureWriter.cpp
    Source/Tools/Utils/DdsTextureWriter.h
    Source/Tools/Utils/DdsTextureWriter.cpp
    Source/Tools/Utils/PngTextureWriter.h
    Source/Tools/Utils/PngTextureWriter.cpp
)
