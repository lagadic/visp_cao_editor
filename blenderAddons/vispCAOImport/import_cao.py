import array
import os
import time
import re
import bpy
import bmesh
import mathutils
import math
from random import *
from bpy_extras.io_utils import unpack_list
from progress_report import ProgressReport, ProgressReportSubstep

def create_cylinder(axis_1, axis_2, radius):
    x1,y1,z1 = axis_1
    x2,y2,z2 = axis_2
    r = radius
    dx = x2 - x1
    dy = y2 - y1
    dz = z2 - z1
    dist = math.sqrt(dx**2 + dy**2 + dz**2)

    bpy.ops.mesh.primitive_cylinder_add(
      radius = r, 
      depth = dist,
      location = (dx/2 + x1, dy/2 + y1, dz/2 + z1)
    )

    bpy.context.object.rotation_euler[1] = math.acos(dz/dist) # theta
    bpy.context.object.rotation_euler[2] = math.atan2(dy, dx) # phi

def create_circle(circum_1,circum_2,center,radius):
    x1,y1,z1 = circum_1
    x2,y2,z2 = center
    x3,y3,z3 = circum_2
    r = radius

    dx1 = x1 - x2
    dy1 = y1 - y2
    dz1 = z1 - z2
    dx2 = x3 - x2
    dy2 = y3 - y2
    dz2 = z3 - z2

    normal = mathutils.Vector.cross(mathutils.Vector((dx1,dy1,dz1)), mathutils.Vector((dx2,dy2,dz2)))

    bpy.ops.mesh.primitive_circle_add(
        radius = r, 
        location = center
    )

    # From  parametric equation of a circle: 
    # P(t) = r.cos(t).u + r.sin(t).n x u + C, where u = [-sin(phi),cos(phi),0].

    theta = -math.atan( dy1 / dz1 )
    phi = math.acos( dz1 / r )
    # angle_z = math.atan( - math.cos( angle_x ) * math.sin( angle_y ) / math.sin( angle_x) ) - math.atan( dx / dy )

    bpy.context.object.rotation_euler[1] = theta
    bpy.context.object.rotation_euler[2] = phi

def create_mesh(global_matrix,
                verts_loc,
                lines,
                face_lines,
                face_points,
                cylinders,
                circles,
                TEMPLATE_FLAG,
                dataname
                ):

    scene = bpy.context.scene

    if TEMPLATE_FLAG in ["3D_F_PTS","3D_F_LNS"]:
        mesh_data = bpy.data.meshes.new(dataname)
        if face_points:
            mesh_data.from_pydata(verts_loc, [], face_points)
        else:
            bm = bmesh.new()
            for v in verts_loc:
                bm.verts.new(v)
            bm.to_mesh(mesh_data)
            bm.free()
        mesh_data.update()
        obj = bpy.data.objects.new(dataname, mesh_data)
        scene.objects.link(obj)
        scene.objects.active = obj
        obj.select = True
        obj.matrix_world = global_matrix

        # Import only lines : TODO
        if TEMPLATE_FLAG == "3D_F_LNS" and face_points == []:
            bpy.ops.object.mode_set(mode='EDIT')
            ob_edit = bpy.context.edit_object
            me = ob_edit.data
            bm = bmesh.from_edit_mesh(me)

            for key in lines:
                if hasattr(bm.verts, "ensure_lookup_table"): 
                    bm.verts.ensure_lookup_table()
                    for i in range(0,len(bm.verts)):
                        bm.verts[i].select = False
                    bm.verts[key[0]].select = True
                    bm.verts[key[1]].select = True
                    print(bm.verts[key[0]],bm.verts[key[1]])
                    bpy.ops.mesh.edge_face_add()
            try:
                bpy.ops.mesh.delete(type='ONLY_FACE')
            except:
                pass

            bpy.ops.object.mode_set(mode='OBJECT')
            # mesh_data.edges.add(lines) # edges should be a list of (a, b) tuples # mesh_data.edges.foreach_set("vertices", unpack_list(lines))

    elif TEMPLATE_FLAG == "3D_CYL":
        create_cylinder( *cylinders)

    elif TEMPLATE_FLAG == "3D_CIR":
        create_circle( *circles)

    # me.loops.foreach_set("vertex_index", loops_vert_idx)
    # me.polygons.foreach_set("loop_start", faces_loop_start)
    # me.polygons.foreach_set("loop_total", faces_loop_total)


    # use_edges = use_edges and bool(edges)
    # if use_edges:
        # me.edges.add(len(edges))
        # edges should be a list of (a, b) tuples
        # me.edges.foreach_set("vertices", unpack_list(edges))

    # me.validate(clean_customdata=False)
    # me.update(calc_edges=use_edges)

    # new_objects.append(obj)

def load(operator, context, filepath,
         global_clamp_size=0.0,
         relpath=None,
         global_matrix=None,
         ):

    TEMPLATE_FLAG = ""

    def regex_search(text,line):
        return bool(re.search(text,line,re.IGNORECASE))

    nPoints = 0
    nLines = 0
    nFacelines = 0
    nFacepoints = 0
    nCylinder = 0
    nCircles = 0

    seed = ["b","l","e","n","d","r","v","t","i","s","p","a","z","y"]
    verts_loc = []
    lines = []
    face_points = []
    face_lines = []
    cylinders = []
    circles = []


    with ProgressReport(context.window_manager) as progress:
        progress.enter_substeps(1, "Importing CAO %r..." % filepath)

        if global_matrix is None:
            global_matrix = mathutils.Matrix()

        time_main = time.time()

        # deselect all
        if bpy.ops.object.select_all.poll():
            # bpy.ops.object.mode_set(mode='OBJECT') # TODO: check if object is mesh
            bpy.ops.object.select_all(action='DESELECT')

        scene = context.scene
        new_objects = []  # put new objects here

        progress.enter_substeps(3, "Parsing CAO file...")
        with open(filepath, 'rb') as f:
            for line in f:
                line_split = line.split()

                if not line_split:
                    continue

                data = line.split(b'#')[0].split()

                if regex_search(b'^.*# +3d +points.*',line):
                    TEMPLATE_FLAG = "3D_PTS"
                    data = []

                elif regex_search(b'^.*# +3d +lines.*',line):
                    TEMPLATE_FLAG = "3D_LNS"
                    data = []

                elif regex_search(b'^.*# +Faces +from +3D +lines.*',line):
                    TEMPLATE_FLAG = "3D_F_LNS"
                    data = []

                elif regex_search(b'^.*# +Faces +from +3D +points.*',line):
                    TEMPLATE_FLAG = "3D_F_PTS"
                    data = []

                elif regex_search(b'^.*# +3d +cylinders.*',line):
                    TEMPLATE_FLAG = "3D_CYL"
                    data = []

                elif regex_search(b'^.*# +3d +circles.*',line):
                    TEMPLATE_FLAG = "3D_CIR"
                    data = []

                if TEMPLATE_FLAG == "3D_PTS":
                    if len(data) == 1:
                        nPoints = int(data[0])
                    elif len(data) == 3:
                        verts_loc.append(list(map(float,[x for x in data])))
                    elif len(data) == 0:
                        pass
                    else:
                        print("\tERROR: Invalid CAO File")

                elif TEMPLATE_FLAG == "3D_LNS":
                    if len(data) == 1:
                        nLines =int(data[0])
                    elif len(data) == 2:
                        lines.append(list(map(int,[x for x in data])))
                    elif len(data) == 0:
                        pass
                    else:
                        print("\tERROR: Invalid CAO File")

                elif TEMPLATE_FLAG == "3D_F_LNS":
                    if len(data) == 1:
                        nFacelines =int(data[0])
                        if nFacelines == 0 and len(lines) > 0:
                            shuffle(seed)
                            # import only lines
                            create_mesh(global_matrix,verts_loc,lines,[],[],[],[],TEMPLATE_FLAG,"3D Lines" + "_" + "".join(seed))                                                
                            pass
                    elif len(data) >= 4:
                        face_lines.append(list(map(int,[x for x in data[1:]])))
                        shuffle(seed)
                        verts = [verts_loc[i] for i in [lines[i][0] for i in face_lines[-1]]]
                        create_mesh(global_matrix,verts,[],[],[list(range(len(verts)))],[],[],TEMPLATE_FLAG,"3D Lines" + "_" + "".join(seed))                    
                    elif len(data) == 0:
                        pass
                    else:
                        print("\tERROR: Invalid CAO File")

                elif TEMPLATE_FLAG == "3D_F_PTS":
                    if len(data) == 1:
                        nFacepoints =int(data[0])
                    elif len(data) >= 4:
                        face_points.append(list(map(int,[x for x in data[1:]])))
                        shuffle(seed)
                        verts = [verts_loc[i] for i in face_points[-1]]
                        create_mesh(global_matrix,verts,[],[],[list(range(len(verts)))],[],[],TEMPLATE_FLAG,"3D Faces" + "_" + "".join(seed))
                    elif len(data) == 0:
                        pass
                    else:
                        print("\tERROR: Invalid CAO File")

                elif TEMPLATE_FLAG == "3D_CYL":
                    if len(data) == 1:
                        nCylinder =int(data[0])
                    elif len(data) == 3:
                        cylinders = [verts_loc[int(data[0])],verts_loc[int(data[1])],float(data[2])]
                        shuffle(seed)
                        create_mesh(global_matrix,verts_loc,[],[],[],cylinders,[],TEMPLATE_FLAG,"3D Cylinder" + "_" + "".join(seed))                        
                    elif len(data) == 0:
                        pass
                    else:
                        print("\tERROR: Invalid CAO File")

                elif TEMPLATE_FLAG == "3D_CIR":
                    if len(data) == 1:
                        nCircles =int(data[0])
                    elif len(data) == 4:
                        circles = [verts_loc[int(data[2])],verts_loc[int(data[3])],verts_loc[int(data[1])],float(data[0])]
                        shuffle(seed)
                        create_mesh(global_matrix,verts_loc,[],[],[],[],circles,TEMPLATE_FLAG,"3D Cylinder" + "_" + "".join(seed))                        
                    elif len(data) == 0:
                        pass
                    else:
                        print("\tERROR: Invalid CAO File")

        progress.step("Done")

        scene.update()

        axis_min = [1000000000] * 3
        axis_max = [-1000000000] * 3

        # if global_clamp_size:
        #     # Get all object bounds
        #     for ob in new_objects:
        #         for v in ob.bound_box:
        #             for axis, value in enumerate(v):
        #                 if axis_min[axis] > value:
        #                     axis_min[axis] = value
        #                 if axis_max[axis] < value:
        #                     axis_max[axis] = value

        #     # Scale objects
        #     max_axis = max(axis_max[0] - axis_min[0], axis_max[1] - axis_min[1], axis_max[2] - axis_min[2])
        #     scale = 1.0

        #     while global_clamp_size < max_axis * scale:
        #         scale = scale / 10.0

            # for obj in new_objects:
            #     obj.scale = scale, scale, scale

        progress.leave_substeps("Done.")
        progress.leave_substeps("Finished importing: %r" % filepath)

    return {'FINISHED'}
