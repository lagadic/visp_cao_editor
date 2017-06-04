
bl_info = {
    "name": "ViSP CAO",
    "author": "Vikas Thamizharasan",
    "blender": (2, 6, 9),
    "location": "File > Export",
    "description": "Export CAO",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Export"}


if "bpy" in locals():
    import imp
    if "export_cao" in locals():
        imp.reload(export_cao)

import bpy
import bmesh
from bpy.props import *
from bpy_extras.io_utils import (ImportHelper,
                                 ExportHelper,
                                 path_reference_mode,
                                 axis_conversion,
                                 )
from mathutils import Vector

# #####################################################
# UI Panel
# #####################################################

def update_after_enum(self, context):
    print('self.vp_model_types ---->', self.vp_model_types)
    # draw_float_properties(self, context)

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

    vp_heirarchy_export = BoolProperty(
        name = "Heirarchy Export", 
        description = "True or False?")

    vp_obj_Point1 = FloatVectorProperty(name = "Point 1 coordinate", description = "Point 1 coordinate", size=3, default=[0.00,0.00,0.00])
    vp_obj_Point2 = FloatVectorProperty(name = "Point 2 coordinate", description = "Point 2 coordinate", size=3, default=[0.00,0.00,0.00])
    vp_obj_Point3 = FloatVectorProperty(name = "Point 3 coordinate", description = "Point 3 coordinate", size=3, default=[0.00,0.00,0.00])

    vp_radius = FloatProperty(name = "", default = 0,description = "Set radius")

class UIPanel(bpy.types.Panel):
    bl_label = "ViSP CAD Properites Panel"
    bl_space_type = "VIEW_3D"
    bl_region_type = "TOOLS"

    # @classmethod
    # def poll(cls, context):
        # return ()

    def draw(self, context):
        layout = self.layout
        scn = context.scene
        col = layout.column()
        col.prop(scn.ignit_panel, "vp_model_types", expand=False)
        
        col1 = col.column()
        col1.enabled = False

        if scn.ignit_panel.vp_model_types in ["3D Cylinders","3D Circles"]:
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

        # col.prop(scn.ignit_panel, "vp_heirarchy_export")
        col.label(" ")
        layout.operator("model_types.selection")
 
# ##########################################################
# Buttons to Set Properites, Calculate Radius, Get Vertices
# ##########################################################

class OBJECT_OT_PrintPropsButton(bpy.types.Operator):
    bl_idname = "model_types.selection"
    bl_label = "Set Properites"

    def execute(self, context):
        scn = context.scene
        for ob in context.selected_objects:
            ob["vp_model_types"] = scn.ignit_panel.vp_model_types

            if scn.ignit_panel.vp_model_types in ["3D Cylinders","3D Circles"]:
                ob["vp_obj_Point1"] = scn.ignit_panel.vp_obj_Point1
                ob["vp_obj_Point2"] = scn.ignit_panel.vp_obj_Point2
                if scn.ignit_panel.vp_model_types == "3D Circles":
                    ob["vp_obj_Point3"] = scn.ignit_panel.vp_obj_Point3

                ob["vp_radius"] = scn.ignit_panel.vp_radius

        return{'FINISHED'}

class OBJECT_OT_Button(bpy.types.Operator):
    bl_idname = "my.button"
    bl_label = "Button"
    number = bpy.props.IntProperty()

    def execute(self, context):
        scn = context.scene
        for ob in context.selected_objects:
            if scn.ignit_panel.vp_model_types in ["3D Cylinders","3D Circles"]:
                ob_edit = context.edit_object # check if in edit mode
                me = ob_edit.data
                bm = bmesh.from_edit_mesh(me)
                selected = [v for v in bm.verts if v.select]
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
                else: #Get coordinates of selected vertex
                    for v in selected:
                        if self.number == 1:
                            scn.ignit_panel.vp_obj_Point1 = v.co
                        elif self.number == 2:
                            scn.ignit_panel.vp_obj_Point2 = v.co
                        elif self.number == 3:
                            scn.ignit_panel.vp_obj_Point3 = v.co
                        print(v.co)
                        break
                    break
        return{'FINISHED'} 


# #####################################################
# ExportCAO
# #####################################################

class ExportCAO(bpy.types.Operator, ExportHelper):

    bl_idname = "export_scene.cao"
    bl_label = 'Export .cao'
    bl_options = {'PRESET'}

    filename_ext = ".cao"
    filter_glob = StringProperty(
            default="*.cao",
            options={'HIDDEN'},
            )

    # context group
    use_selection = BoolProperty(
            name="Selection Only",
            description="Export selected objects only",
            default=False,
            )

    # object group
    use_mesh_modifiers = BoolProperty(
            name="Apply Modifiers",
            description="Apply modifiers (preview resolution)",
            default=True,
            )

    # extra data group
    use_edges = BoolProperty(
            name="Include Edges",
            description="",
            default=True,
            )

    use_normals = BoolProperty(
            name="Include Normals",
            description="",
            default=False,
            )

    use_triangles = BoolProperty(
            name="Triangulate Faces",
            description="Convert all faces to triangles",
            default=False,
            )

    axis_forward = EnumProperty(
            name="Forward",
            items=(('X', "X Forward", ""),
                   ('Y', "Y Forward", ""),
                   ('Z', "Z Forward", ""),
                   ('-X', "-X Forward", ""),
                   ('-Y', "-Y Forward", ""),
                   ('-Z', "-Z Forward", ""),
                   ),
            default='-Z',
            )
    axis_up = EnumProperty(
            name="Up",
            items=(('X', "X Up", ""),
                   ('Y', "Y Up", ""),
                   ('Z', "Z Up", ""),
                   ('-X', "-X Up", ""),
                   ('-Y', "-Y Up", ""),
                   ('-Z', "-Z Up", ""),
                   ),
            default='Y',
            )
    global_scale = FloatProperty(
            name="Scale",
            min=0.01, max=1000.0,
            default=1.0,
            )


    check_extension = True

    def execute(self, context):
        from . import export_cao

        from mathutils import Matrix
        keywords = self.as_keywords(ignore=("axis_forward",
                                            "axis_up",
                                            "global_scale",
                                            "check_existing",
                                            "filter_glob",
                                            ))

        global_matrix = (Matrix.Scale(self.global_scale, 4) *
                         axis_conversion(to_forward=self.axis_forward,
                                         to_up=self.axis_up,
                                         ).to_4x4())
        keywords["global_matrix"] = global_matrix

        return export_cao.save(self, context, **keywords)

def menu_func_export(self, context):
    self.layout.operator(ExportCAO.bl_idname, text="ViSP .cao")


def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_export.append(menu_func_export)
    bpy.types.Scene.ignit_panel = bpy.props.PointerProperty(type=IgnitProperties)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)
    del bpy.types.Scene.ignit_panel

if __name__ == "__main__":
    register()
