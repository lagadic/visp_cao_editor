import bpy
import bmesh
from random import *
from bpy.props import *
from mathutils import Vector
from bpy.types import Panel, UIList

# #########################################
# ViSP Property Panel
# #########################################

def update_after_enum(self, context):
    print('self.vp_model_types ---->', self.vp_model_types)

class IgnitProperties(bpy.types.PropertyGroup):
    vp_model_types = bpy.props.EnumProperty(
        name = "Type",
        description = "Model export types",
        items = [
            ("3D Faces" , "3D Faces" , "Export as 3d points"),
            ("3D Lines", "3D Lines", "Export as 3d lines"),
            ("3D Cylinders", "3D Cylinders", "Export as 3d cylinders"),
            ("3D Circles", "3D Circles", "Export as 3d circles")],
        update=update_after_enum
        )

    vp_export_enable = BoolProperty(name = "Enable For Export", description = "True or False?", default = True)

    vp_obj_Point1 = FloatVectorProperty(name = "Point 1 coordinate", description = "Point 1 coordinate", size=3, default=[0.00,0.00,0.00])
    vp_obj_Point2 = FloatVectorProperty(name = "Point 2 coordinate", description = "Point 2 coordinate", size=3, default=[0.00,0.00,0.00])
    vp_obj_Point3 = FloatVectorProperty(name = "Point 3 coordinate", description = "Point 3 coordinate", size=3, default=[0.00,0.00,0.00])

    vp_radius = FloatProperty(name = "", default = 0,description = "Set radius")

class CustomProp_vertices(bpy.types.PropertyGroup):
    '''name = StringProperty() '''
    id = IntProperty()
    coord = FloatVectorProperty(description = "coordinate", size=3, default=[0.00,0.00,0.00])

class UL_items_vertices(UIList):

    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):
        split = layout.split(0.3)
        split.label("%d" % (index))
        split.prop(item, "name", text="", emboss=False, translate=True)

    def invoke(self, context, event):
        pass  

class UIPanel(bpy.types.Panel):
    bl_label = "ViSP CAD Properites Panel"
    bl_space_type = "VIEW_3D"
    bl_region_type = "TOOLS"

    def __init__(self):
        self._ob_select = None

    def draw(self, context):
        layout = self.layout
        scn = context.scene
        col = layout.column()

        if not len(context.selected_objects):
            col.label("Select Object(s) in scene to add properties")

        else:
            self._ob_select = context.selected_objects[0]
            try:
                self._ob_select["vp_model_types"]
            except:
                col.label("Add new property")
                col.operator("my.button", text="+ New").number=5

            else:
                col.operator("refresh.button", text="load previous property") # Load prev. set properties. Button click required to write to panel
                col.prop(scn.ignit_panel, "vp_model_types", expand=False)
                col1 = col.column()
                col1.enabled = False

                if scn.ignit_panel.vp_model_types in ["3D Faces","3D Lines"]:
                    if context.active_object.mode == 'EDIT': # enable only in edit mode
                        col1.enabled = True
                    else:
                        col.label("Switch to EDIT MODE to get coordinates")
                        col.label("Note: Only needed if model is complex")

                    col1.template_list("UL_items_vertices", "", scn, "custom_vertices", scn, "custom_vertices_index", rows=2)
                    col1.operator("my.button", text="Get Vertices").number=6
                    col1.operator("my.button", text="Clear List").number=7
                    bpy.app.debug = True

                elif scn.ignit_panel.vp_model_types in ["3D Cylinders","3D Circles"]:
                    if context.active_object.mode == 'EDIT': # enable only in edit mode
                        col1.enabled = True
                    else:
                        col.label("Switch to EDIT MODE to set radius and coordinates")

                    col1.prop(scn.ignit_panel, "vp_obj_Point1")
                    col1.operator("my.button", text="Get Point 1").number=1
                    col1.prop(scn.ignit_panel, "vp_obj_Point2")
                    col1.operator("my.button", text="Get Point 2").number=2
                    if scn.ignit_panel.vp_model_types == "3D Circles":
                        col1.prop(scn.ignit_panel, "vp_obj_Point3")
                        col1.operator("my.button", text="Get Point 3").number=3

                    row = col1.row()
                    row.operator("my.button", text="Calculate Radius").number=4
                    row.prop(scn.ignit_panel, "vp_radius")
                col.prop(scn.ignit_panel, "vp_export_enable")
                col.label(" ")
                layout.operator("model_types.selection")
 
# #########################################
# BUTTON CALLS
# #########################################

new_mesh = ""
seed = ["b","l","e","n","d","r","v","t","i","s","p","a","z","y"]
def clear_vertices_list(scn):
    lst = scn.custom_vertices
    if len(lst) > 0:
        for i in range(len(lst)-1,-1,-1):
            scn.custom_vertices.remove(i)

class OBJECT_OT_AddPropsButton(bpy.types.Operator):
    bl_idname = "model_types.selection"
    bl_label = "Add Properites"

    def execute(self, context):
        global new_mesh, seed
        scn = context.scene
        hasCircle = False
        hasCylinder = False
        ob = context.selected_objects[0]

        if scn.ignit_panel.vp_model_types == "3D Faces":
            ob["vp_model_types"] = scn.ignit_panel.vp_model_types
            ob["vp_export_enable"] = scn.ignit_panel.vp_export_enable
            attr=(o.name for o in scn.custom_faces)
            if ob.name not in attr:
                item = scn.custom_faces.add()
                item.id = len(scn.custom_faces)
                if len(scn.custom_vertices) > 0:
                    item.name = new_mesh
                else:
                    item.name = ob.name
                scn.custom_faces_index = (len(scn.custom_faces)-1)
            scn.custom_faces[scn.custom_faces_index].enabled = ob["vp_export_enable"]
            clear_vertices_list(scn)

        elif scn.ignit_panel.vp_model_types == "3D Lines":
            ob["vp_model_types"] = scn.ignit_panel.vp_model_types
            ob["vp_export_enable"] = scn.ignit_panel.vp_export_enable
            attr=(o.name for o in scn.custom_lines)
            if ob.name not in attr:
                item = scn.custom_lines.add()
                item.id = len(scn.custom_lines)
                if len(scn.custom_vertices) > 0:
                    item.name = new_mesh
                else:
                    item.name = ob.name
                scn.custom_lines_index = (len(scn.custom_lines)-1)
            scn.custom_lines[scn.custom_lines_index].enabled = ob["vp_export_enable"]
            clear_vertices_list(scn)

        elif scn.ignit_panel.vp_model_types in ["3D Cylinders","3D Circles"]:
            attr_circle = (o.name for o in scn.custom_circle)
            attr_cylinder = (o.name for o in scn.custom_cylinder)
            if ob.name not in attr_cylinder:
                hasCylinder = True
            if ob.name not in attr_circle:
                hasCircle = True

            if hasCircle and hasCylinder:
                shuffle(seed)
                me = bpy.data.meshes.new(scn.ignit_panel.vp_model_types+"".join(seed))
                ob = bpy.data.objects.new(scn.ignit_panel.vp_model_types+"".join(seed), me)
                scn.objects.link(ob)
                # scn.objects.active = ob
                # ob.select = True

            ob["vp_model_types"] = scn.ignit_panel.vp_model_types
            ob["vp_export_enable"] = scn.ignit_panel.vp_export_enable
            ob["vp_obj_Point1"] = scn.ignit_panel.vp_obj_Point1
            ob["vp_obj_Point2"] = scn.ignit_panel.vp_obj_Point2
            ob["vp_radius"] = scn.ignit_panel.vp_radius

            if scn.ignit_panel.vp_model_types == "3D Circles":
                ob["vp_obj_Point3"] = scn.ignit_panel.vp_obj_Point3
                if hasCircle:
                    item = scn.custom_circle.add()
                    item.id = len(scn.custom_circle)
                    scn.custom_circle_index = (len(scn.custom_circle)-1)
                    item.name = ob.name
                scn.custom_circle[scn.custom_circle_index].enabled = ob["vp_export_enable"]
            else:
                if hasCylinder:
                    item = scn.custom_cylinder.add()
                    item.id = len(scn.custom_cylinder)
                    scn.custom_cylinder_index = (len(scn.custom_cylinder)-1)
                    item.name = ob.name
                scn.custom_cylinder[scn.custom_cylinder_index].enabled = ob["vp_export_enable"]

        return{'FINISHED'}

class OBJECT_OT_RefreshButton(bpy.types.Operator):
    bl_idname = "refresh.button"
    bl_label = "Button"

    def __init__(self):
        self._ob_select = None

    def execute(self, context):
        scn = context.scene
        self._ob_select = context.selected_objects[0]
        scn.ignit_panel.vp_model_types = self._ob_select["vp_model_types"]
        scn.ignit_panel.vp_export_enable = self._ob_select["vp_export_enable"]
        if self._ob_select["vp_model_types"] in ["3D Cylinders","3D Circles"]:
            try:
                self._ob_select["vp_obj_Point1"]
            except:
                pass
            else:
                scn.ignit_panel.vp_obj_Point1 = self._ob_select["vp_obj_Point1"]
                scn.ignit_panel.vp_obj_Point2 = self._ob_select["vp_obj_Point2"]
                if self._ob_select["vp_model_types"] == "3D Circles":
                    scn.ignit_panel.vp_obj_Point3 = self._ob_select["vp_obj_Point3"]

                scn.ignit_panel.vp_radius = self._ob_select["vp_radius"]

        return{'FINISHED'}

class OBJECT_OT_Button(bpy.types.Operator):
    bl_idname = "my.button"
    bl_label = "Button"
    number = bpy.props.IntProperty()

    def __init__(self):
        self._ob_select = None

    # @classmethod
    # def poll(cls, context):
    #     return (True)

    def execute(self, context):
        global new_mesh, seed
        scn = context.scene

        if self.number == 7:
            clear_vertices_list(scn)

        elif self.number == 5:
            self._ob_select = context.selected_objects[0]
            self._ob_select["vp_model_types"] = "3D Faces"

        else:
            for ob in context.selected_objects:
                ob_edit = context.edit_object # check if in edit mode
                me = ob_edit.data
                bm = bmesh.from_edit_mesh(me)
                selected = [v for v in bm.verts if v.select]
                #Get selected vertices
                if self.number == 6:
                    #TODO: how to undo/ update points change?
                    for v in selected:
                        item = scn.custom_vertices.add()
                        item.id = len(scn.custom_vertices)
                        item.coord = [round(i,4) for i in v.co]
                        item.name = ",".join(map(str,[x for x in item.coord]))
                        scn.custom_vertices_index = (len(scn.custom_vertices)-1)
                    bpy.ops.mesh.edge_face_add()    
                    bpy.ops.mesh.separate(type='SELECTED')
                    shuffle(seed)
                    scn.objects[0].name += scn.ignit_panel.vp_model_types + "".join(seed)
                    bpy.ops.object.mode_set(mode='OBJECT')
                    bpy.ops.object.select_all(action='DESELECT')
                    scn.objects[0].select = True
                    bpy.ops.object.mode_set(mode='EDIT')
                    new_mesh = scn.objects[0].name

                # Calculate Radius
                if self.number == 4:    
                    vsum = Vector()
                    for v in selected:
                        vsum += v.co
                    midPoint = vsum/len(selected)
                    distances = [(v.co-midPoint).length for v in selected]
                    radius = sum(distances)/len(distances)
                    ob["vp_radius"] = radius
                    scn.ignit_panel.vp_radius = radius
                #Get coordinates of selected vertex
                else:
                    v = selected[0]
                    if self.number == 1:
                        scn.ignit_panel.vp_obj_Point1 = v.co
                    elif self.number == 2:
                        scn.ignit_panel.vp_obj_Point2 = v.co
                    elif self.number == 3:
                        scn.ignit_panel.vp_obj_Point3 = v.co
        return{'FINISHED'}

classes = (
    IgnitProperties,
    CustomProp_vertices,
    UIPanel,
    OBJECT_OT_Button,
    OBJECT_OT_RefreshButton,
    OBJECT_OT_AddPropsButton,
    UL_items_vertices
)

if __name__ == "__main__":
    from bpy.utils import register_class
    for cls in classes:
        register_class(cls)
