"""
Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).

SPDX-License-Identifier: Apache-2.0 OR MIT
"""

from __future__ import annotations
from typing import List, Tuple
import importlib
import os

import azlmbr
import azlmbr.legacy.general as azgeneral
import azlmbr.bus as ebus
import azlmbr.math as azmath
import azlmbr.asset as azasset

import pyside_utils

import my_entity_utils
import shader_creator
importlib.reload(my_entity_utils)
importlib.reload(shader_creator)

from my_entity_utils import EditorComponent
from my_entity_utils import EditorEntity
from shader_creator import RenderJoyShaderCreator


class ShaderWizard:
    """
    This class is the brains. Creates entities, components and poeprly links them
    to instantiate a default RenderJoy Shader effect, rendered as a billboard.
    """
    TAB="    "

    # @param reportCB An optional callback with method .ReportMessage(msg: str)
    def __init__(self, reportCB=None):
        self._reportCB = reportCB

    # Returns the full path of the AZSL file
    def Run(self, shaderName: str) -> str:
        reportCB = self._reportCB
        # First thing let's create the parent entity that will own the billboard.
        billboardEntity, billboardComponent = self._CreateBillboardEntity(shaderName)
        #Controller|Configuration|RenderJoy Shader: ('EntityId', 'Visible')
        #Controller|Configuration|Display Mode: ('unsigned int', 'Visible')
        #Controller|Configuration|Billboard Options: ('BillboardOptions', 'Visible')
        #Controller|Configuration|Billboard Options|Always Face Camera: ('bool', 'Visible')
        #Controller|Configuration|Flatscreen Options: ('FlatscreenOptions', 'Visible')
        #Controller|Configuration|Flatscreen Options|Scale X: ('float', 'Visible')
        #Controller|Configuration|Flatscreen Options|Scale Y: ('float', 'Visible')
        #Controller|Configuration|Flatscreen Options|Pos X: ('float', 'Visible')
        #Controller|Configuration|Flatscreen Options|Pos Y: ('float', 'Visible')
        #Controller|Configuration|Flatscreen Options|Width: ('float', 'Visible')
        #Controller|Configuration|Flatscreen Options|Height: ('float', 'Visible')
        #billboardComponent.DumpProperties()
        
        shaderEntity, shaderComponent = self._CreateShaderEntity(shaderName, billboardEntity)
        #Controller|Configuration|Shader: ('Asset<ShaderAsset>', 'Visible')
        #Controller|Configuration|Input Channels (AZStd::array<EntityId, 4>,Visible)
        #]: ('EntityId', 'Visible')
        #Controller|Configuration|Render Target Width: ('unsigned int', 'Visible')
        #Controller|Configuration|Render Target Height: ('unsigned int', 'Visible')
        #Controller|Configuration|Render Target Format: ('Format', 'Visible')
        #Save To Disk|Configuration|Output Path: ('AZ::IO::Path', 'Visible')
        #Save To Disk|Configuration: ('SaveToDiskConfig', 'Visible')
        #shaderComponent.DumpProperties()

        # Time to create the azsl and the shader file.
        shaderSourcePath, azshaderPath, azslSourcePath= self._CreateShaderFiles(shaderName)
        shaderAssetId = self._WaitForShaderAssetReady(azshaderPath)
        shaderComponent.set_component_property_value('Controller|Configuration|Shader', shaderAssetId)
        if reportCB:
            reportCB.ReportMessage(f"{ShaderWizard.TAB}Set the shader asset to {azshaderPath}")

        # Finally set the shader entity to the billboard.
        billboardComponent.set_component_property_value('Controller|Configuration|RenderJoy Shader', shaderEntity.id)
        if reportCB:
            reportCB.ReportMessage(f"Set shader entity for RenderJoy Billboard component to entity with id={shaderEntity.id}, name [{shaderEntity.get_name()}]")
        return azslSourcePath


    def _CreateBillboardEntity(self, shaderName: str) -> Tuple[EditorEntity, EditorComponent]:
        reportCB = self._reportCB
        # First thing let's create the parent entity that will own the billboard.
        billboardName = f"{shaderName} Billboard"
        billboardEntity = EditorEntity.create_editor_entity(billboardName)
        if reportCB:
            reportCB.ReportMessage(f"Created billboard parent entity={billboardName}")
        # Add the billboard component.
        componentUuid = azlmbr.globals.property.EditorRenderJoyBillboardComponentTypeId
        #print(f"uuid = {componentUuid}")
        billboardComponent = billboardEntity.add_component_by_uuid(componentUuid)
        if billboardComponent is None:
            print(f"Failed to create billboard component with Uuid={componentUuid}")
        if reportCB:
            if billboardComponent:
                reportCB.ReportMessage(f"{ShaderWizard.TAB}Added [{billboardComponent.get_component_name()}] component")
            else:
                reportCB.ReportMessage(f"{ShaderWizard.TAB}Failed to add billboard component with uuid: {componentUuid}")
        return billboardEntity, billboardComponent


    def _CreateShaderEntity(self, shaderName: str, parentEntity: EditorEntity) -> Tuple[EditorEntity, EditorComponent]:
        reportCB = self._reportCB
        # First thing let's create the parent entity that will own the billboard.
        entityName = f"{shaderName} Shader"
        shaderEntity = EditorEntity.create_editor_entity(entityName, parentEntity.id)
        if reportCB:
            reportCB.ReportMessage(f"Created shader entity={entityName}")
        # Add the shader component.
        componentUuid = azlmbr.globals.property.EditorRenderJoyShaderComponentTypeId
        #print(f"uuid = {componentUuid}")
        shaderComponent = shaderEntity.add_component_by_uuid(componentUuid)
        if shaderComponent is None:
            print(f"Failed to create shader component with Uuid={componentUuid}")
        if reportCB:
            if shaderComponent:
                reportCB.ReportMessage(f"{ShaderWizard.TAB}Added [{shaderComponent.get_component_name()}] component")
            else:
                reportCB.ReportMessage(f"{ShaderWizard.TAB}Failed to add shader component with uuid: {componentUuid}")
        return shaderEntity, shaderComponent


    #Returns the full path of the source .shader file
    def _CreateShaderFiles(self, shaderStemName: str) -> Tuple[str, str, str]:
        # Let's find the current level path
        outputDir = azgeneral.get_current_level_path()
        print(f"level path {outputDir}")
        shaderCreator = RenderJoyShaderCreator(self._reportCB)
        shaderSourcePath, azslSourcePath = shaderCreator.CreateShaderFiles(shaderStemName, outputDir)
        # Let's deduce the file path of the azshader
        fullLevelsDir, levelName = os.path.split(outputDir)
        fullGameDir, levelsFolderName = os.path.split(fullLevelsDir)
        levelSubfolder = f"{levelsFolderName}/{levelName}"
        azshaderPath =  f"{levelSubfolder}/{shaderStemName}.azshader"
        return shaderSourcePath, azshaderPath, azslSourcePath
    
    # @param azshaderPath Can be Cache relative or Cache absolute.
    def _WaitForShaderAssetReady(self, azshaderPath:str) -> azlmbr.asset.AssetId:
        RegisterType = False
        def myCondition():
            asset_id = azasset.AssetCatalogRequestBus(ebus.Broadcast, "GetAssetIdByPath", azshaderPath, azmath.Uuid(), RegisterType)
            if asset_id:
                return asset_id.is_valid()
            return None
        futureObj = pyside_utils.wait_for_condition(myCondition, timeout=10.0)
        result = pyside_utils.run_async(futureObj)
        if result:
            assetId = azasset.AssetCatalogRequestBus(ebus.Broadcast, "GetAssetIdByPath", azshaderPath, azmath.Uuid(), RegisterType)
            return assetId
        return None
    

if __name__ == "__main__":
    print("I'm just a humble module that offers the ShaderWizard class!")