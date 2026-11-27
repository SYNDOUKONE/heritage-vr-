# -*- coding: utf-8 -*-
"""Pipeline de production des artefacts du musee Heritage VR.

Usage (Blender 4.x, headless ou GUI) :

    blender --background scene.blend --python artifact_pipeline.py -- \
        --out ../Export/Artifacts/Baoule --collection MasqueGoli

Le script :
1. valide le nommage (SM_<culture>_<objet>) et les proprietes de collection (culture, id) ;
2. applique transformations et origines au centre de masse ;
3. genere LOD0/LOD1/LOD2 par decimation (naming LOD Unreal) ;
4. exporte en FBX oriente pour Unreal (forward X, up Z, echelle cm).
"""

import argparse
import os
import sys

import bpy

LOD_DECIMATION = {0: 1.0, 1: 0.5, 2: 0.25}
VALID_CULTURES = {"Baoule", "Senoufo", "DanYacouba", "GrandBassam"}


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description="Pipeline artefacts Heritage VR")
    parser.add_argument("--out", required=True, help="Dossier d'export FBX")
    parser.add_argument("--collection", required=True, help="Collection Blender de l'artefact")
    return parser.parse_args(argv)


def fail(message):
    print(f"[artifact_pipeline] ERREUR: {message}")
    sys.exit(1)


def validate_collection(collection):
    culture = collection.get("culture")
    artifact_id = collection.get("artifact_id")
    if not culture or culture not in VALID_CULTURES:
        fail(f"Collection '{collection.name}' : propriete 'culture' manquante ou invalide ({sorted(VALID_CULTURES)})")
    if not artifact_id:
        fail(f"Collection '{collection.name}' : propriete 'artifact_id' manquante")
    for obj in collection.objects:
        if obj.type != "MESH":
            continue
        if not obj.name.startswith("SM_"):
            fail(f"Objet '{obj.name}' : doit commencer par SM_")
    return culture, artifact_id


def prepare_object(obj):
    bpy.context.view_layer.objects.active = obj
    for modifier in list(obj.modifiers):
        obj.modifiers.remove(modifier)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bpy.ops.object.origin_set(type="ORIGIN_CENTER_OF_MASS", center="BOUNDS")


def build_lods(obj):
    lods = []
    for lod_index, ratio in LOD_DECIMATION.items():
        if lod_index == 0:
            lod = obj
        else:
            lod = obj.copy()
            lod.data = obj.data.copy()
            bpy.context.collection.objects.link(lod)
            decimate = lod.modifiers.new(name="Decimate", type="DECIMATE")
            decimate.ratio = ratio
        lod.name = f"{obj.name}_LOD{lod_index}"
        lod.data.name = lod.name
        lods.append(lod)
    return lods


def export_fbx(lods, out_dir, culture, artifact_id):
    os.makedirs(out_dir, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    for lod in lods:
        lod.select_set(True)
    out_path = os.path.abspath(os.path.join(out_dir, f"SM_{culture}_{artifact_id}.fbx"))
    bpy.ops.export_scene.fbx(
        filepath=out_path,
        use_selection=True,
        global_scale=100.0,
        axis_forward="X",
        axis_up="Z",
        apply_scale_options="FBX_SCALE_UNITS",
        mesh_smooth_type="FACE",
        bake_space_transform=True,
    )
    print(f"[artifact_pipeline] Export: {out_path}")
    return out_path


def main():
    args = parse_args()
    collection = bpy.data.collections.get(args.collection)
    if not collection:
        fail(f"Collection '{args.collection}' introuvable")

    culture, artifact_id = validate_collection(collection)
    meshes = [obj for obj in collection.objects if obj.type == "MESH"]
    if not meshes:
        fail(f"Aucun mesh dans la collection '{collection.name}'")

    all_lods = []
    for obj in meshes:
        prepare_object(obj)
        all_lods.extend(build_lods(obj))

    export_fbx(all_lods, args.out, culture, artifact_id)
    triangles = sum(sum(p.loop_total for p in lod.data.polygons) for lod in all_lods)
    print(f"[artifact_pipeline] OK - {culture}/{artifact_id} - {triangles} triangles cumules (LOD0+1+2)")


if __name__ == "__main__":
    main()
