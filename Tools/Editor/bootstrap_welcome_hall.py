# -*- coding: utf-8 -*-
"""Bootstrap du niveau d'accueil du musee Heritage VR.

A executer dans l'editeur Unreal 5.4 (Output Log -> Python, ou via
Tools > Execute Python Script) :

    exec(open('<chemin>/Tools/Editor/bootstrap_welcome_hall.py').read())

Cree (ou met a jour) /Game/Maps/WelcomeHall avec :
- sol, lumieres (directionnelle + sky), PlayerStart ;
- deux artefacts de demonstration (AArtifactActor) sur leur socle ;
- une AVillageZone de demonstration ;
- sauvegarde du niveau.
"""

import unreal

WORLD_PATH = "/Game/Maps/WelcomeHall"
HALL_HALF = 800.0  # demi-taille du hall en cm


def get_actor_subsystem():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def spawn(cls, location, rotation=(0.0, 0.0, 0.0), label=None):
    actor = get_actor_subsystem().spawn_actor_from_class(
        cls, unreal.Vector(*location), unreal.Rotator(*rotation)
    )
    if actor and label:
        actor.set_actor_label(label)
    return actor


def add_floor():
    floor = spawn(unreal.StaticMeshActor, (0.0, 0.0, -50.0), label="SM_Hall_Floor")
    mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
    floor.static_mesh_component.set_static_mesh(mesh)
    floor.set_actor_scale3d(unreal.Vector(16.0, 16.0, 1.0))
    return floor


def add_pedestal(location, label):
    pedestal = spawn(unreal.StaticMeshActor, location, label=label)
    mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
    pedestal.static_mesh_component.set_static_mesh(mesh)
    pedestal.set_actor_scale3d(unreal.Vector(0.6, 0.6, 1.0))
    return pedestal


def make_artifact_info(artifact_id, title_fr, title_local, culture, region, description):
    info = unreal.ArtifactInfo()
    info.set_editor_property("artifact_id", artifact_id)
    info.set_editor_property("title_fr", unreal.Text(title_fr))
    info.set_editor_property("title_local", unreal.Text(title_local))
    info.set_editor_property("culture", unreal.Text(culture))
    info.set_editor_property("region", unreal.Text(region))
    info.set_editor_property("description_fr", unreal.Text(description))
    return info


def add_demo_artifact(location, label, info):
    pedestal = add_pedestal((location[0], location[1], 0.0), f"Pedestal_{label}")
    top_z = 0.0 + 100.0  # cube 100 cm, echelle Z 1.0 -> sommet a +50, artefact pose a +100

    artifact = spawn(unreal.ArtifactActor, (location[0], location[1], top_z), label=f"Artifact_{label}")
    cube = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")
    artifact.set_editor_property("mesh", cube)
    artifact.set_editor_property("info", info)
    artifact.set_actor_scale3d(unreal.Vector(0.4, 0.4, 0.4))
    return pedestal, artifact


def add_village_zone(center, label, village_id, display_name, description):
    zone = spawn(unreal.VillageZone, center, label=f"Zone_{label}")
    zone.configure_zone(
        village_id,
        unreal.Text(display_name),
        unreal.Text(description),
    )
    volume = zone.get_editor_property("zone_volume")
    if volume:
        volume.set_box_extent(unreal.Vector(600.0, 600.0, 300.0))
    return zone


def main():
    els = unreal.get_editor_subsystem(unreal.EditorLevelSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(WORLD_PATH):
        world = unreal.EditorAssetLibrary.load_asset(WORLD_PATH)
    else:
        world = els.new_level(WORLD_PATH)

    if not world:
        unreal.log_warning("[bootstrap] impossible de creer ou charger " + WORLD_PATH)
        return

    add_floor()

    spawn(unreal.DirectionalLight, (0.0, 0.0, 400.0), rotation=(-45.0, 30.0, 0.0), label="Light_Directional")
    spawn(unreal.SkyLight, (0.0, 0.0, 400.0), label="Light_Sky")
    spawn(unreal.PlayerStart, (0.0, 600.0, 100.0), rotation=(0.0, 180.0, 0.0), label="PlayerStart")

    add_demo_artifact(
        (-250.0, 0.0, 0.0),
        "MasqueGoli",
        make_artifact_info(
            "baoule_masque_goli",
            "Masque Goli",
            "Glao",
            "Baoulé",
            "Région de Bouaké",
            "Masque du rituel Goli, dansé pour apaiser les esprits et protéger "
            "le village. Bois sculpté, face ronde aux cornes de buffle.",
        ),
    )
    add_demo_artifact(
        (250.0, 0.0, 0.0),
        "DeanGle",
        make_artifact_info(
            "dan_dean_gle",
            "Masque Dean Gle",
            "Dean Gle",
            "Dan / Yacouba",
            "Région de Man",
            "Masque de la danse Dean Gle, visage fin et souriant, porté lors "
            "des fêtes de réconciliation. Symbole de grâce et de paix sociale.",
        ),
    )

    add_village_zone(
        (0.0, 0.0, 100.0),
        "DemoBaoule",
        "baoule_demo",
        "Village Baoulé (démo)",
        "Zone de démonstration : remplacer par le village reconstitué complet.",
    )

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log("[bootstrap] WelcomeHall pret : 2 artefacts, 1 zone village, sauvegarde effectuee.")


main()
