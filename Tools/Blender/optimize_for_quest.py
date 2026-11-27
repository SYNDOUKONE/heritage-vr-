# -*- coding: utf-8 -*-
"""Validation des budgets Meta Quest pour les assets du musee Heritage VR.

Usage :

    blender --background scene.blend --python optimize_for_quest.py -- \
        --collection MasqueGoli [--export-gltf ../Export/glTF/MasqueGoli.glb]

Budgets appliques (cible Quest 3, UE5 forward mobile) :
- artefact interactif : 10 000 triangles en LOD0, 2 materiaux max ;
- scene de village complete : 150 000 triangles au total ;
- textures : 2048 px maximum, preferer 1024 pour les tissus repetes.
"""

import argparse
import os
import sys

import bpy

MAX_TRIANGLES_PER_ARTIFACT = 10_000
MAX_TRIANGLES_PER_SCENE = 150_000
MAX_TEXTURE_SIZE = 2048
MAX_MATERIALS_PER_MESH = 2


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description="Budgets Quest Heritage VR")
    parser.add_argument("--collection", default=None, help="Limiter la validation a une collection")
    parser.add_argument("--export-gltf", default=None, help="Exporter aussi en glTF binary")
    return parser.parse_args(argv)


def triangle_count(mesh_obj):
    mesh_obj.update_from_editmode()
    return sum(p.loop_total for p in mesh_obj.data.polygons)


def collect_meshes(collection_name):
    if collection_name:
        collection = bpy.data.collections.get(collection_name)
        if not collection:
            print(f"[optimize] ERREUR: collection '{collection_name}' introuvable")
            sys.exit(1)
        return [o for o in collection.all_objects if o.type == "MESH"]
    return [o for o in bpy.data.objects if o.type == "MESH"]


def main():
    args = parse_args()
    meshes = collect_meshes(args.collection)

    issues = []
    warnings = []
    total_triangles = 0

    for obj in meshes:
        tris = triangle_count(obj)
        total_triangles += tris

        if tris > MAX_TRIANGLES_PER_ARTIFACT:
            issues.append(f"{obj.name}: {tris} triangles > {MAX_TRIANGLES_PER_ARTIFACT} (decimer ou LOD)")

        materials = [slot.material for slot in obj.material_slots if slot.material]
        if len(materials) > MAX_MATERIALS_PER_MESH:
            issues.append(f"{obj.name}: {len(materials)} materiaux > {MAX_MATERIALS_PER_MESH}")

        for material in materials:
            node_tree = getattr(material, "node_tree", None)
            if not node_tree:
                continue
            for node in node_tree.nodes:
                if node.type != "TEX_IMAGE" or not node.image:
                    continue
                width, height = node.image.size
                if width > MAX_TEXTURE_SIZE or height > MAX_TEXTURE_SIZE:
                    issues.append(
                        f"{obj.name}/{material.name}/{node.image.name}: "
                        f"{width}x{height} > {MAX_TEXTURE_SIZE}px"
                    )
                if node.image.colorspace_settings.name == "sRGB" and "ORM" in node.image.name:
                    warnings.append(f"{node.image.name}: ORM devrait etre en espace Non-Color")

    if total_triangles > MAX_TRIANGLES_PER_SCENE:
        issues.append(f"Scene: {total_triangles} triangles > {MAX_TRIANGLES_PER_SCENE}")

    for warning in warnings:
        print(f"[optimize] ATTENTION: {warning}")
    for issue in issues:
        print(f"[optimize] ERREUR: {issue}")
    if issues:
        sys.exit(1)
    print("[optimize] OK - tous les budgets Quest sont respectes")

    if args.export_gltf:
        os.makedirs(os.path.dirname(os.path.abspath(args.export_gltf)), exist_ok=True)
        bpy.ops.export_scene.gltf(
            filepath=args.export_gltf,
            export_format="GLB",
            export_apply=True,
            export_texcoords=True,
            export_normals=True,
            export_materials="EXPORT",
        )
        print(f"[optimize] Export glTF: {args.export_gltf}")


if __name__ == "__main__":
    main()
