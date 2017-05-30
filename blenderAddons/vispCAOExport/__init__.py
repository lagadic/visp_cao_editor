
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

class IgnitProperties(bpy.types.PropertyGroup):
    vp_model_types = bpy.props.EnumProperty(
        name = "Type",
        description = "Model export types",
        items = [
            ("3D Points" , "3D Points" , "Export as 3d points"),
            ("3D Lines", "3D Lines", "Export as 3d lines"),
            ("3D Cylinders", "3D Cylinders", "Export as 3d cylinders"),
            ("3D Circles", "3D Circles", "Export as 3d circles")])

    vp_heirarchy_export = BoolProperty(
        name = "Heirarchy Export", 
        description = "True or False?")

    vp_obj_Point1x = FloatProperty(name = "X", description = "Point 1 x coordinate", default = 0.000)
    vp_obj_Point1y = FloatProperty(name = "Y", description = "Point 1 y coordinate", default = 0.000)
    vp_obj_Point1z = FloatProperty(name = "Z", description = "Point 1 z coordinate", default = 0.000)

    vp_obj_Point2x = FloatProperty(name = "X", description = "Point 2 x coordinate", default = 0.000)
    vp_obj_Point2y = FloatProperty(name = "Y", description = "Point 2 y coordinate", default = 0.000)
    vp_obj_Point2z = FloatProperty(name = "Z", description = "Point 2 z coordinate", default = 0.000)

    vp_obj_Point3x = FloatProperty(name = "X", description = "Point 3 x coordinate", default = 0.000)
    vp_obj_Point3y = FloatProperty(name = "Y", description = "Point 3 y coordinate", default = 0.000)
    vp_obj_Point3z = FloatProperty(name = "Z", description = "Point 3 z coordinate", default = 0.000)
    vp_radius = FloatProperty(name = "", default = 0,description = "radius")


class UIPanel(bpy.types.Panel):
    bl_label = "ViSP CAD Properites Panel"
    bl_space_type = "VIEW_3D"
    bl_region_type = "TOOLS"
 
    def draw(self, context):
        layout = self.layout
        scn = context.scene
        col = layout.column()
        col.prop(scn.ignit_panel, "vp_model_types", expand=False)
        
        col.label("Point 1 coordinate (cylinder/ circle) :", icon='TEXT')
        row1 = col.row()
        row1.prop(scn.ignit_panel, "vp_obj_Point1x")
        row1.prop(scn.ignit_panel, "vp_obj_Point1y")
        row1.prop(scn.ignit_panel, "vp_obj_Point1z")

        col.label("Point 2 coordinate (cylinder/ circle) :", icon='TEXT')
        row2 = col.row()
        row2.prop(scn.ignit_panel, "vp_obj_Point2x")
        row2.prop(scn.ignit_panel, "vp_obj_Point2y")
        row2.prop(scn.ignit_panel, "vp_obj_Point2z")

        col.label("Point 3 coordinate (circle):", icon='TEXT')
        row3 = col.row()
        row3.prop(scn.ignit_panel, "vp_obj_Point3x")
        row3.prop(scn.ignit_panel, "vp_obj_Point3y")
        row3.prop(scn.ignit_panel, "vp_obj_Point3z")

        col.label("\n")
        row4 = col.row()
        row4.operator("my.button", text="Calculate Radius")
        row4.prop(scn.ignit_panel, "vp_radius")

        # col.prop(scn.ignit_panel, "vp_heirarchy_export")

        layout.operator("model_types.selection")
 
# #####################################################
# Button to Set Properites
# #####################################################

class OBJECT_OT_PrintPropsButton(bpy.types.Operator):
    bl_idname = "model_types.selection"
    bl_label = "Set Properites"

    def execute(self, context):
        scn = context.scene
        for ob in context.selected_objects:
            ob["vp_model_types"] = scn.ignit_panel.vp_model_types
            ob["vp_obj_Point1"] = [scn.ignit_panel.vp_obj_Point1x, scn.ignit_panel.vp_obj_Point1y, scn.ignit_panel.vp_obj_Point1z]
            ob["vp_obj_Point2"] = [scn.ignit_panel.vp_obj_Point2x, scn.ignit_panel.vp_obj_Point2y, scn.ignit_panel.vp_obj_Point2z]
            ob["vp_obj_Point3"] = [scn.ignit_panel.vp_obj_Point3x, scn.ignit_panel.vp_obj_Point3y, scn.ignit_panel.vp_obj_Point3z] # Will be ignored for cylinder
            ob["vp_radius"] = scn.ignit_panel.vp_radius

        return{'FINISHED'}

class OBJECT_OT_Button(bpy.types.Operator):
    bl_idname = "my.button"
    bl_label = "Button"
 
    def execute(self, context):
        scn = context.scene
        for ob in context.selected_objects:
            if scn.ignit_panel.vp_model_types in ["3D Cylinders","3D Circles"]:
                # Calculate Radius
                ob_edit = context.edit_object
                me = ob_edit.data
                bm = bmesh.from_edit_mesh(me)
                selected = [v for v in bm.verts if v.select]
                vsum = Vector()
                for v in selected:
                    vsum += v.co
                midPoint = vsum/len(selected)
                distances = [(v.co-midPoint).length for v in selected]
                radius = sum(distances)/len(distances)
                ob["vp_radius"] = radius
                scn.ignit_panel.vp_radius = radius

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
