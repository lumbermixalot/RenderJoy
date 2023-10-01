"""
Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).

SPDX-License-Identifier: Apache-2.0 OR MIT
"""

# Functionality copied from:
# https://github.com/o3de/o3de/blob/development/AutomatedTesting/Gem/PythonTests/EditorPythonTestTools/editor_python_test_tools/editor_entity_utils.py

from __future__ import annotations
from typing import List, Tuple
from enum import Enum

import azlmbr
import azlmbr.bus as ebus
import azlmbr.editor as editor
import azlmbr.math as azmath
import azlmbr.legacy.general as azgeneral

class EditorEntityType(Enum):
    GAME = azlmbr.entity.EntityType().Game
    LEVEL = azlmbr.entity.EntityType().Level

### class EditorEntityType END
######################################################################################



class EditorComponent:
    """
    Modified version of this original: https://github.com/o3de/o3de/blob/development/AutomatedTesting/Gem/PythonTests/EditorPythonTestTools/editor_python_test_tools/editor_entity_utils.py
    EditorComponent class used to set and get the component property value using path
    EditorComponent object is returned from either of
        EditorEntity.add_component() or Entity.add_components() or EditorEntity.get_components_of_type()
    which also assigns self.id and self.type_id to the EditorComponent object.
    self.type_id is the UUID for the component type as provided by an ebus call.
    self.id is an azlmbr.entity.EntityComponentIdPair which contains both entity and component id's
    """

    def __init__(self, type_id: azmath.Uuid):
        self.type_id = type_id
        self.id = None
        self.property_tree_editor = None

    def get_component_name(self) -> str:
        """
        Used to get name of component
        :return: name of component
        """
        type_names = editor.EditorComponentAPIBus(ebus.Broadcast, "FindComponentTypeNames", [self.type_id])
        assert len(type_names) != 0, "Component object does not have type id"
        return type_names[0]

    @staticmethod
    def get_type_ids(component_names: list[str], entity_type: EditorEntityType = EditorEntityType.GAME) -> list:
        """
        Used to get type ids of given components list
        :param component_names: List of components to get type ids
        :param entity_type: Entity_Type enum value Entity_Type.GAME is the default
        :return: List of type ids of given components. Type id is a UUID as provided by the ebus call
        """
        type_ids = editor.EditorComponentAPIBus(
            ebus.Broadcast, "FindComponentTypeIdsByEntityType", component_names, entity_type.value)
        return type_ids
    
    def get_property_tree(self, force_get: bool = False):
        """
        Used to get and cache the property tree editor of component that has following functions associated with it:
            1. prop_tree.is_container(path)
            2. prop_tree.get_container_count(path)
            3. prop_tree.reset_container(path)
            4. prop_tree.add_container_item(path, key, item)
            5. prop_tree.remove_container_item(path, key)
            6. prop_tree.update_container_item(path, key, value)
            7. prop_tree.get_container_item(path, key)
        :param force_get: Force a fresh property tree editor rather than the cached self.property_tree_editor
        :return: Property tree editor of the component
        """
        if (not force_get) and (self.property_tree_editor is not None):
            return self.property_tree_editor

        build_prop_tree_outcome = editor.EditorComponentAPIBus(
            ebus.Broadcast, "BuildComponentPropertyTreeEditor", self.id
        )
        assert (
            build_prop_tree_outcome.IsSuccess()
        ), f"Failure: Could not build property tree editor of component: '{self.get_component_name()}'"
        prop_tree = build_prop_tree_outcome.GetValue()
        self.property_tree_editor = prop_tree
        return self.property_tree_editor
    
    def get_property_type_visibility(self):
        """
        Used to work with Property Tree Editor build_paths_list_with_types.
        Some component properties have unclear type or return to much information to contain within one Report.info.
        The iterable dictionary can be used to print each property path and type individually with a for loop.
        :return: Dictionary where key is property path and value is a tuple (property az_type, UI visible)
        """
        if self.property_tree_editor is None:
            self.get_property_tree()
        path_type_re = re.compile(r"([ A-z_|]+)(?=\s) \(([A-z0-9:<> ]+),([A-z]+)")
        result = {}
        path_types = self.property_tree_editor.build_paths_list_with_types()
        for path_type in path_types:
            match_line = path_type_re.search(path_type)
            if match_line is None:
                print(path_type)
                continue
            path, az_type, visible = match_line.groups()
            result[path] = (az_type, visible)
        return result
    
    def set_component_property_value(self, component_property_path: str, value: object):
        """
        Used to set component property value
        :param component_property_path: Path of property in the component to act on
        :param value: new value for the variable being changed in the component
        """
        outcome = editor.EditorComponentAPIBus(
            ebus.Broadcast, "SetComponentProperty", self.id, component_property_path, value
        )
        assert (
            outcome.IsSuccess()
        ), f"Failure: Could not set value to '{self.get_component_name()}' : '{component_property_path}'"
        azgeneral.idle_wait_frames(1)
        self.get_property_tree(True)
    
    def DumpProperties(self):
        """
            A helper function to print all properties of a component.
        """
        props = self.get_property_type_visibility()
        for key, value in props.items():
            print(f"{key}: {value}")

### class EditorComponent END
######################################################################################



class EditorEntity:
    """
    Modified version of this original: https://github.com/o3de/o3de/blob/development/AutomatedTesting/Gem/PythonTests/EditorPythonTestTools/editor_python_test_tools/editor_entity_utils.py
    Entity class is used to create and interact with Editor Entities.
    Example: To create Editor Entity, Use the code:
        test_entity = EditorEntity.create_editor_entity("TestEntity")
        # This creates a python object with 'test_entity' linked to entity name "TestEntity" in Editor.
        # To add component, use:
        test_entity.add_component(<COMPONENT_NAME>)
    """

    def __init__(self, id: azlmbr.entity.EntityId):
        self.id: azlmbr.entity.EntityId = id
        self.components: List[EditorComponent] = []

    @classmethod
    def create_editor_entity(cls, name: str = None, parent_id: azlmbr.entity.EntityId =None) -> EditorEntity:
        """
        Used to create entity at default position using 'CreateNewEntity' Bus
        :param name: Name of the Entity to be created
        :param parent_id: (optional) Used to create child entity under parent_id if specified
        :return: EditorEntity class object
        """
        if parent_id is None:
            parent_id = azlmbr.entity.EntityId()

        new_id = azlmbr.editor.ToolsApplicationRequestBus(ebus.Broadcast, "CreateNewEntity", parent_id)
        assert new_id.IsValid(), "Failure: Could not create Editor Entity"
        entity = cls(new_id)
        if name:
            entity.set_name(name)

        return entity
    
    # Methods
    def set_name(self, entity_name: str) -> None:
        """
        Given entity_name, sets name to Entity
        :param: entity_name: Name of the entity to set
        """
        editor.EditorEntityAPIBus(ebus.Event, "SetName", self.id, entity_name)

    def get_name(self) -> str:
        """
        Used to get the name of entity
        """
        return editor.EditorEntityInfoRequestBus(ebus.Event, "GetName", self.id)
    

    def add_component_by_uuid(self, component_uuid: azmath.Uuid) -> EditorComponent:
        """
        Used to add new component to Entity.
        :param component_name: String of component name to add.
        :return: Component object of newly added component.
        """
        components = self.add_components_by_uuid([component_uuid,])
        if len(components) > 0:
            return components[0]
        return None

    def add_components_by_uuid(self, component_uuids: list[azmath.Uuid]) -> List[EditorComponent]:
        """
        Used to add multiple components
        :param: component_names: List of components to add to entity
        :return: List of newly added components to the entity
        """
        components = []
        for type_id in component_uuids:
            new_comp = EditorComponent(type_id)
            add_component_outcome = editor.EditorComponentAPIBus(
                ebus.Broadcast, "AddComponentsOfType", self.id, [type_id]
            )
            assert (
                add_component_outcome.IsSuccess()
            ), f"Failure: Could not add component: '{new_comp.get_component_name()}' to entity: '{self.get_name()}'"
            new_comp.id = add_component_outcome.GetValue()[0]
            components.append(new_comp)
            self.components.append(new_comp)
        azgeneral.idle_wait_frames(1)
        return components

    def add_component_by_name(self, component_name: str) -> EditorComponent:
        """
        Used to add new component to Entity.
        :param component_name: String of component name to add.
        :return: Component object of newly added component.
        """
        component = self.add_components_by_name([component_name])[0]
        return component

    def add_components_by_name(self, component_names: list) -> List[EditorComponent]:
        """
        Used to add multiple components
        :param: component_names: List of components to add to entity
        :return: List of newly added components to the entity
        """
        components = []
        type_ids = EditorComponent.get_type_ids(component_names, EditorEntityType.GAME)
        for type_id in type_ids:
            new_comp = EditorComponent(type_id)
            add_component_outcome = editor.EditorComponentAPIBus(
                ebus.Broadcast, "AddComponentsOfType", self.id, [type_id]
            )
            assert (
                add_component_outcome.IsSuccess()
            ), f"Failure: Could not add component: '{new_comp.get_component_name()}' to entity: '{self.get_name()}'"
            new_comp.id = add_component_outcome.GetValue()[0]
            components.append(new_comp)
            self.components.append(new_comp)
        azgeneral.idle_wait_frames(1)
        return components

### class EditorEntity END
######################################################################################


if __name__ == "__main__":
    print("I'm just a humble module that offers the EditorComponent and EditorEntity classes!")