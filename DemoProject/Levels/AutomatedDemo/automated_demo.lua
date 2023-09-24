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
        rotationAxis = { default = Vector3(1.0, 0.0, 1.0)
                            , description="Rotation vector"},
        rotationSpeed = { default = 1.0
                            , description="Rotation speed"
                            , suffix="deg/s"},
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

    self._rotationAxis = self.Properties.rotationAxis:GetNormalized()
    self._angle = 0.0
    self._rotationSpeed = self.Properties.rotationSpeed

    self._currentState = self._OnTickState_Idle
	self.tickBusHandler = TickBus.Connect(self)

end

function automated_demo:OnDeactivate()
    if self.tickBusHandler then
        self.tickBusHandler:Disconnect()
    end
end

function automated_demo:OnTick(deltaTime, timePoint)
	self._currentState(self, deltaTime, timePoint)
end

-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
-- The Tick States  START
-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
function automated_demo:_OnTickState_Idle(deltaTime, timePoint)
    self._angle = self._angle + self._rotationSpeed * deltaTime
    local quat = Quaternion.CreateFromAxisAngle(self._rotationAxis, self._angle)
    --local position = TransformBus.Event.GetWorldTranlation(self._cubeEntity)
    TransformBus.Event.SetLocalRotationQuaternion(self._cubeEntity, quat)
end
--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
-- The Tick States END
--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
-- Private methods  START
-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
-- Private methods END
--<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


return automated_demo
