-- Just  basic template that you can quickly copy/paste
-- and start writing your component class
local automated_demo = {
    Properties = {
        cubeEntity = { default = EntityId()
                            , description="The parent entity of all RenderJoy billboards" },
        pacmanEntity = { default = EntityId()
                            , description="The entity that owns the Pacman billboard" },
        cameraEntity = { default = EntityId()
                            , description="The entity that owns the main camera"},
        viewTimePerShader = { default = 6.0
                            , description="How many seconds a particular shader effect will be in focus"
                            , suffix="s"},
    }
}

function automated_demo:OnActivate()    
    self._cubeEntity = self.entityId
    if EntityId.IsValid(self.Properties.cubeEntity) then
        self._cubeEntity = self.Properties.cubeEntity
    end
    
    self._pacmanEntity = self.entityId
    if EntityId.IsValid(self.Properties.pacmanEntity) then
        self._pacmanEntity = self.Properties.pacmanEntity
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
        self:_ForceBillboardMode()
        --self._currentState = self._OnTickState_TweenSetup
        self._currentState = self._OnTickState_WaitingForCompletion
        self:_OnTweenComplete()
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

function automated_demo:_OnTickState_TweenSetup(deltaTime, timePoint)

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

    self._currentState = self._OnTickState_WaitingForCompletion
end

function automated_demo:_OnTickState_WaitingForCompletion(deltaTime, timePoint)
    if self._done then
        Debug.Log("Disconnecting from ticks")
        self.tickBusHandler:Disconnect()
        self.tickBusHandler = nil
        self:_ForceBillboardMode()
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
        else
            Debug.Warning(false, "automated_demo:_DiscoverBillboardEntities Spawnable Root with name=" .. entityName .. " has unexpected amount of children=" .. tostring(spawnableChildren:Size()))
        end
    end

    self._billboardEntities = billboardEntities
end


function automated_demo:_ForceBillboardMode()
    for _, billboardEntity in ipairs(self._billboardEntities) do
        RenderJoyBillboardBus.Event.SetBillboardMode(billboardEntity, true);
    end
end

--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
-- Private methods END
--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


return automated_demo
