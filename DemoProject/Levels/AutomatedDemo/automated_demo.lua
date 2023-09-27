-- Just  basic template that you can quickly copy/paste
-- and start writing your component class
local automated_demo = {
    Properties = {
        cubeEntity = { default = EntityId()
                            , description="The parent entity of all RenderJoy billboards" },
        pacmanParentEntity = { default = EntityId()
                            , description="The parent entity that owns the Pacman billboard entity." },
        cameraEntity = { default = EntityId()
                            , description="The entity that owns the main camera"},
        viewTimePerShader = { default = 6.0
                            , description="How many seconds a particular shader effect will be in focus"
                            , suffix="s"},
        finalPositions = {
            default = { Vector3(-3.0, 1.5, 3.598),  Vector3(0.0, 1.5, 1.25),  Vector3(3.0, 1.5, 1.25),
                        Vector3(-3.0, 1.5, -1.25), Vector3(0.0, 1.5, -1.25), Vector3(3.0, 1.5, 1.098), }
            , description="Final positions for the billboards."},
    }
}

function automated_demo:OnActivate()    
    self._cubeEntity = self.entityId
    if EntityId.IsValid(self.Properties.cubeEntity) then
        self._cubeEntity = self.Properties.cubeEntity
    end
    
    self._pacmanParentEntity = self.entityId
    if EntityId.IsValid(self.Properties.pacmanParentEntity) then
        self._pacmanParentEntity = self.Properties.pacmanParentEntity
    end

    self._cameraEntity = self.entityId
    if EntityId.IsValid(self.Properties.cameraEntity) then
        self._cameraEntity = self.Properties.cameraEntity
    end

    self._tweener = require("Scripts.RenderJoy.ScriptedEntityTweener")
    self._tweener:OnActivate()

    self._totalTime = 0.0
    self._done = false
    --self._angle = 0.0
    --self._rotationSpeed = self.Properties.rotationSpeed

    self._currentState = self._OnTickState_WaitinForCompletion
	self.tickBusHandler = TickBus.Connect(self)

end

function automated_demo:OnDeactivate()
    if self.tickBusHandler then
        self.tickBusHandler:Disconnect()
    end

    if self._tweener then
        self._tweener:OnDeactivate()
    end
end

function automated_demo:OnTick(deltaTime, timePoint)
    self._totalTime = self._totalTime + deltaTime
	self._currentState(self, deltaTime, timePoint)
end

-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
-- The Tick States  START
-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
function automated_demo:_OnTickState_WaitinForCompletion(deltaTime, timePoint)
    if self._totalTime >= 1.0 then
        self:_DiscoverBillboardEntities()
        self:_DiscoverPacmanEntities()
        --self:_ForceBillboardMode()
        --self:_DumpBillboardPositions()
        self._currentState = self._OnTickState_FirstTimelineSetup
        --self._currentState = self._OnTickState_SecondTimelineSetup
        --self:_OnTweenComplete()
    end
end

-- function automated_demo:_OnTickState_TweenSetup(deltaTime, timePoint)
-- 
--     local onCompleteCB = function ()
--         self:_OnTweenComplete()
--     end
-- 
--     --[[
--     -- Need to create the final Quaternion.
--     local quat = Quaternion.CreateFromAxisAngle(self._rotationAxis, math.pi * 0.5)
--     local tweenArgs = {
--         easeMethod = ScriptedEntityTweenerEasingMethod_Elastic,
--         easeType = ScriptedEntityTweenerEasingType_Out,
--         ["3drotation"] = quat, --Vector3(0.0, 0.0, math.pi),
--         onComplete = onCompleteCB
--     }
--     ]]
-- 
--     local quat = Quaternion.CreateFromAxisAngle(self._rotationAxis, math.pi * 0.5)
--     local tweenArgs = {
--         easeMethod = ScriptedEntityTweenerEasingMethod_Quad,
--         easeType = ScriptedEntityTweenerEasingType_In,
--         ["3drotation"] = quat, --Vector3(0.0, 0.0, math.pi),
--         onComplete = onCompleteCB
--     }
-- 
--     self._tweener:StartAnimation(self._cubeEntity, 3.0, tweenArgs)
--     self._currentState = self._OnTickState_WaitingForCompletion
-- end

function automated_demo:_OnTickState_FirstTimelineSetup(deltaTime, timePoint)

    local timeline1 = self._tweener:TimelineCreate()

    local rotationAxis = Vector3(0.0, 0.0, 1.0)
    --rotationAxis = rotationAxis:GetNormalized()

    ----------------------------------------------------------------
    -- Add a wait here
    local quat1 = Quaternion.CreateFromAxisAngle(rotationAxis, 0.0)
    local tweenArgs1 = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Linear,
        easeType = ScriptedEntityTweenerEasingType_InOut,
        ["3drotation"] = quat1, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, self.Properties.viewTimePerShader, tweenArgs1)

    ----------------------------------------------------------------
    local quat2 = Quaternion.CreateFromAxisAngle(rotationAxis, math.pi * 0.5)
    local tweenArgs2 = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Elastic,
        easeType = ScriptedEntityTweenerEasingType_Out,
        ["3drotation"] = quat2, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, 3.0, tweenArgs2)

    -----------------------------------------------------------------
    -- Add a wait here
    local quat3 = quat2
    local tweenArgs3= {
        easeMethod = ScriptedEntityTweenerEasingMethod_Linear,
        easeType = ScriptedEntityTweenerEasingType_InOut,
        ["3drotation"] = quat3, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, self.Properties.viewTimePerShader, tweenArgs3)

    -----------------------------------------------------------------
    local quat4 = Quaternion.CreateFromAxisAngle(rotationAxis, math.pi * 1.0)
    local tweenArgs4 = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Elastic,
        easeType = ScriptedEntityTweenerEasingType_Out,
        ["3drotation"] = quat4, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, 3.0, tweenArgs4)
    
    -----------------------------------------------------------------
    -- Add a wait here
    local quat5 = quat4
    local tweenArgs5= {
        easeMethod = ScriptedEntityTweenerEasingMethod_Linear,
        easeType = ScriptedEntityTweenerEasingType_InOut,
        ["3drotation"] = quat5, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, self.Properties.viewTimePerShader, tweenArgs5)

    -----------------------------------------------------------------
    local quat6 = Quaternion.CreateFromAxisAngle(rotationAxis, math.pi * 1.5)
    local tweenArgs6 = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Elastic,
        easeType = ScriptedEntityTweenerEasingType_Out,
        ["3drotation"] = quat6, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, 3.0, tweenArgs6)

    -----------------------------------------------------------------
    -- Add a wait here
    local quat7 = quat6
    local tweenArgs7= {
        easeMethod = ScriptedEntityTweenerEasingMethod_Linear,
        easeType = ScriptedEntityTweenerEasingType_InOut,
        ["3drotation"] = quat7, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, self.Properties.viewTimePerShader, tweenArgs7)

    -----------------------------------------------------------------
    local quat8 = Quaternion.CreateFromAxisAngle(rotationAxis, math.pi * 2.0)
    local tweenArgs8 = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Elastic,
        easeType = ScriptedEntityTweenerEasingType_Out,
        ["3drotation"] = quat8, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, 3.0, tweenArgs8)

    -----------------------------------------------------------------
    -- Add a short wait here
    local quat9 = quat8
    local tweenArgs9= {
        easeMethod = ScriptedEntityTweenerEasingMethod_Linear,
        easeType = ScriptedEntityTweenerEasingType_InOut,
        ["3drotation"] = quat9, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, self.Properties.viewTimePerShader * 0.2, tweenArgs9)

    ------------------------------------------------------------------

    rotationAxis = Vector3(1.0, 0.0, 0.0)

    local quat10 = Quaternion.CreateFromAxisAngle(rotationAxis, math.pi * 0.5)
    local tweenArgs10 = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Elastic,
        easeType = ScriptedEntityTweenerEasingType_Out,
        ["3drotation"] = quat10, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, 3.0, tweenArgs10)

    -----------------------------------------------------------------
    -- Add a wait here
    local quat11 = quat10
    local tweenArgs11= {
        easeMethod = ScriptedEntityTweenerEasingMethod_Linear,
        easeType = ScriptedEntityTweenerEasingType_InOut,
        ["3drotation"] = quat11, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, self.Properties.viewTimePerShader, tweenArgs11)

    ------------------------------------------------------------------

    local quat12 = Quaternion.CreateFromAxisAngle(rotationAxis, math.pi)
    local tweenArgs12 = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Elastic,
        easeType = ScriptedEntityTweenerEasingType_Out,
        ["3drotation"] = quat12, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, 3.0, tweenArgs12)

    -----------------------------------------------------------------

    local quat13 = Quaternion.CreateFromAxisAngle(rotationAxis, math.pi * 1.5)
    local tweenArgs13 = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Elastic,
        easeType = ScriptedEntityTweenerEasingType_Out,
        ["3drotation"] = quat13, --Vector3(0.0, 0.0, math.pi),
       --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, 3.0, tweenArgs13)

    -----------------------------------------------------------------
    -- Add a wait here
    local quat14= quat13
    local tweenArgs14= {
        easeMethod = ScriptedEntityTweenerEasingMethod_Linear,
        easeType = ScriptedEntityTweenerEasingType_InOut,
        ["3drotation"] = quat14, --Vector3(0.0, 0.0, math.pi),
        --onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, self.Properties.viewTimePerShader, tweenArgs14)

    -----------------------------------------------------------------

    local quat15 = Quaternion.CreateFromAxisAngle(rotationAxis, math.pi * 2.0)
    local onCompleteCB = function ()
        self:_OnTweenComplete()
    end

    local tweenArgs15 = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Elastic,
        easeType = ScriptedEntityTweenerEasingType_Out,
        ["3drotation"] = quat15, --Vector3(0.0, 0.0, math.pi),
        onComplete = onCompleteCB
    }
    timeline1:Add(self._cubeEntity, 3.0, tweenArgs15)
    ------------------------------------------------------------------

    timeline1:Play()

    self._done = false
    self._currentState = self._OnTickState_WaitingForFirstTimeline
end

-- When this is called, it is assumed all billboards are in Billboard mode, facing the camera.
function automated_demo:_OnTickState_SecondTimelineSetup(deltaTime, timePoint)

    local positions = self.Properties.finalPositions
    -- local entities = self._billboardEntities
    local entities = self._parentEntities

    local timeline1 = self._tweener:TimelineCreate()

    local easeM = ScriptedEntityTweenerEasingMethod_Elastic
    local easeT = ScriptedEntityTweenerEasingType_Out
    local travelTime = 1.0

    local numEntities = #entities
    for i=1,numEntities do
        local tweenArgs = {
            easeMethod = easeM,
            easeType = easeT,
            ["3dposition"] = positions[i], --Vector3(0.0, 0.0, math.pi),
            --onComplete = onCompleteCB
        }
        --if i == numEntities then
        --    tweenArgs['onComplete'] = onCompleteCB
        --end
        timeline1:Add(entities[i], travelTime, tweenArgs)
    end

    -- Finally add the pacman position animation
    local onCompleteCB = function ()
        self:_OnTweenComplete()
        -- self:_DumpBillboardPositions()
    end

    local finalPos = self._pacmanCurrentPosition:Clone()
    finalPos.y = -2.0

    local tweenArgsPacman = {
        easeMethod = ScriptedEntityTweenerEasingMethod_Cubic,
        easeType = ScriptedEntityTweenerEasingType_Out,
        ["3dposition"] = finalPos, --Vector3(0.0, 0.0, math.pi),
        onComplete = onCompleteCB
    }
    timeline1:Add(self._pacmanParentEntity, 3.0, tweenArgsPacman)


    timeline1:Play()

    self._done = false
    self._currentState = self._OnTickState_WaitingForSecondTimeline

end

function automated_demo:_OnTickState_WaitingForFirstTimeline(deltaTime, timePoint)
    if self._done then
        self._currentState = self._OnTickState_SecondTimelineSetup
        --Debug.Log("Disconnecting from ticks")
        --self.tickBusHandler:Disconnect()
        --self.tickBusHandler = nil
        self:_ForceBillboardMode()
    end
end

function automated_demo:_OnTickState_WaitingForSecondTimeline(deltaTime, timePoint)
    if self._done then
        Debug.Log("Disconnecting from ticks")
        self.tickBusHandler:Disconnect()
        self.tickBusHandler = nil
        --self:_ForceBillboardMode()
    end
end

--[[
function automated_demo:_OnTickState_Idle(deltaTime, timePoint)
    self._angle = self._angle + self._rotationSpeed * deltaTime
    local quat = Quaternion.CreateFromAxisAngle(self._rotationAxis, self._angle)
    --local position = TransformBus.Event.GetWorldTranlation(self._cubeEntity)
    TransformBus.Event.SetLocalRotationQuaternion(self._cubeEntity, quat)
end
]]

--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
-- The Tick States END
--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
-- Private methods  START
-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
function automated_demo:_OnTweenComplete()
    Debug.Log("automated_demo:_OnTweenComplete")
    self._done = true
end

function automated_demo:_DiscoverBillboardEntities()
    -- Get the list of 1st level children from the Cube parent entity.
    -- These are typically the parent entity of the Spawnable.

    local billboardEntities = {}
    local parentEntities = {}
    local billboardIndex = 0

    -- vector_EntityId
    local vectorOfEntities =  TransformBus.Event.GetChildren(self._cubeEntity)
    local size = vectorOfEntities:Size()
    for i = 1, size, 1  do
        local entity = vectorOfEntities:At(i)
        local entityName = GameEntityContextRequestBus.Broadcast.GetEntityName(entity)
        Debug.Log("automated_demo:_DiscoverBillboardEntities Spawnable Root with name=" .. entityName)
        -- vector_EntityId
        local spawnableChildren = TransformBus.Event.GetChildren(entity)
        if  spawnableChildren:Size() == 1 then
            local billboardEntity = spawnableChildren:At(1)
            local billboardName = GameEntityContextRequestBus.Broadcast.GetEntityName(billboardEntity)
            Debug.Log("automated_demo:_DiscoverBillboardEntities billboard with name=" .. billboardName)
            billboardIndex = billboardIndex + 1
            billboardEntities[billboardIndex] = billboardEntity
            parentEntities[billboardIndex] = entity
        else
            Debug.Warning(false, "automated_demo:_DiscoverBillboardEntities Spawnable Root with name=" .. entityName .. " has unexpected amount of children=" .. tostring(spawnableChildren:Size()))
        end
    end

    self._billboardEntities = billboardEntities
    self._parentEntities = parentEntities
end


function automated_demo:_ForceBillboardMode()
    for _, billboardEntity in ipairs(self._billboardEntities) do
        RenderJoyBillboardBus.Event.SetBillboardMode(billboardEntity, true);
    end
end

function automated_demo:_DumpBillboardPositions()
    for idx, billboardEntity in ipairs(self._billboardEntities) do
        local name = GameEntityContextRequestBus.Broadcast.GetEntityName(billboardEntity)
        local localPos = TransformBus.Event.GetLocalTranslation(billboardEntity)
        local worldPos = TransformBus.Event.GetWorldTranslation(billboardEntity)
        Debug.Log("[" .. tostring(idx) .. "]" .. name .. ", local=" .. tostring(localPos) .. ", world=" .. tostring(worldPos))
    end
end

function automated_demo:_DiscoverPacmanEntities()
    local children = TransformBus.Event.GetChildren(self._pacmanParentEntity)
    assert(children:Size() == 1, "Unexpected size for vector of entities.")
    local pacmanBillboardEntity = children:At(1)
    local currentPacmanPosition = TransformBus.Event.GetWorldTranslation(self._pacmanParentEntity)
    currentPacmanPosition.y = 20.0
    TransformBus.Event.SetWorldTranslation(self._pacmanParentEntity, currentPacmanPosition)

    self._pacmanBillboardEntity = pacmanBillboardEntity
    self._pacmanCurrentPosition = currentPacmanPosition
end

--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
-- Private methods END
--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


return automated_demo
