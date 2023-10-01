"""
Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).

SPDX-License-Identifier: Apache-2.0 OR MIT
"""

from __future__ import annotations
from typing import List, Tuple

import os
import string

class RenderJoyShaderCreator:
    TAB="    "

    # @param reportCB A callback with method .ReportMessage(msg: str)
    def __init__(self, reportCB=None):
        self._reportCB = reportCB

    def CreateShaderFiles(self, stemName: str, parentDir: str):
        reportCB = self._reportCB

        azslName, azslfilePath = self._CreateAzsl(stemName, parentDir)
        if reportCB:
            reportCB.ReportMessage(f"{RenderJoyShaderCreator.TAB}Created .azsl: {azslName} at {azslfilePath}")
        
        shaderName, shaderfilePath = self._CreateShader(stemName, parentDir, azslName)
        if reportCB:
            reportCB.ReportMessage(f"{RenderJoyShaderCreator.TAB}Created .shader: {shaderName} at {shaderfilePath}")

        return shaderfilePath, azslfilePath

    def _CreateAzsl(self, stemName: str, parentDir: str) -> Tuple[str, str]:
        defaultAzslStr = """
/*
 * Add your copyright
*/

// All RenderJoy shaders must start by including this file
// or a copy of this file.
#include <RenderJoy/RenderJoyPassSrg.azsli>


// Same behavior as ShaderToy default shader:
// https://www.shadertoy.com/new
PSOutput MainPS(VSOutput IN)
{
    PSOutput OUT;
    // Normalized pixel coordinates (from 0 to 1)
    float2 uv = IN.m_position.xy / iResolution.xy;
    // Time varying pixel color
    float3 color = 0.5 + 0.5*cos(iTime + uv.xyx + float3(0,2,4));
    OUT.m_color = float4(color,1);
    return OUT;
}

        """
        azslName = f"{stemName}.azsl"
        fileName = os.path.join(parentDir, azslName)
        try:
            fileObj = open(fileName, "wt")
            fileObj.write(defaultAzslStr)
            fileObj.close()
        except:
            return None
        return azslName, fileName

    def _CreateShader(self, stemName: str, parentDir: str, azslName: str):
        defaultShaderTemplateStr = """
{ 
    "Source" : "$azsl",

    "AddBuildArguments": {
      "debug": false
    },
    
    "DepthStencilState" : 
    {
        "Depth" : 
        { 
            "Enable" : false 
        },
        "Stencil" :
        {
            "Enable" : false
        }
    },

    "ProgramSettings":
    {
      "EntryPoints":
      [
        {
          "name": "MainVS",
          "type": "Vertex"
        },
        {
          "name": "MainPS",
          "type": "Fragment"
        }
      ]
    }   
}
"""
        defaultShaderTemplate = string.Template(defaultShaderTemplateStr)
        shaderName = f"{stemName}.shader"
        fileName = os.path.join(parentDir, shaderName)
        try:
            fileObj = open(fileName, "wt")
            fileObj.write(defaultShaderTemplate.substitute({'azsl' : azslName}))
            fileObj.close()
        except:
            return None
        return shaderName, fileName
    
if __name__ == "__main__":
    print("I'm just a humble module that offers the RenderJoyShaderCreator class!")