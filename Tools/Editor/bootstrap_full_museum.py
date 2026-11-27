# -*- coding: utf-8 -*-
"""Generation complete du musee Heritage VR : hall + 4 villages.

A executer dans l'editeur Unreal 5.4 (Output Log -> Python) :

    exec(open('<chemin>/Tools/Editor/bootstrap_full_museum.py').read())

Cree ou complete :
- /Game/Maps/WelcomeHall (4 portails vers les villages) ;
- /Game/Maps/Villages/Baoule_Village      (3 cases rondes, aire du Goli, 4 artefacts) ;
- /Game/Maps/Villages/Senoufo_Village     (case sacrée, greniers, forge, 4 artefacts) ;
- /Game/Maps/Villages/Dan_Yacouba_Village (huttes, clairiere sous canopée, 3 artefacts) ;
- /Game/Maps/Villages/GrandBassam_Quartier (maisons a veranda, comptoir, 3 artefacts).

Les architectures sont des placeholders (formes de base du moteur) a remplacer
par les kits produits via Tools/Blender/artifact_pipeline.py.
Le script est re-executable : le hall n'ajoute pas les portails deux fois.
"""

import unreal

WELCOME = "/Game/Maps/WelcomeHall"

CUBE = "/Engine/BasicShapes/Cube"
CYLINDER = "/Engine/BasicShapes/Cylinder"
CONE = "/Engine/BasicShapes/Cone"
SPHERE = "/Engine/BasicShapes/Sphere"


def actor_subsys():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def spawn(cls, loc, rot=(0.0, 0.0, 0.0), label=None):
    actor = actor_subsys().spawn_actor_from_class(cls, unreal.Vector(*loc), unreal.Rotator(*rot))
    if actor and label:
        actor.set_actor_label(label)
    return actor


def place_static(loc, scale, mesh_path, label="SM"):
    actor = spawn(unreal.StaticMeshActor, loc, (0.0, 0.0, 0.0), label)
    actor.static_mesh_component.set_static_mesh(unreal.EditorAssetLibrary.load_asset(mesh_path))
    actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor


def level_is_new(path):
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.get_editor_subsystem(unreal.EditorLevelSubsystem).load_level(path)
        return False
    unreal.get_editor_subsystem(unreal.EditorLevelSubsystem).new_level(path)
    return True


def ensure_lighting():
    """Cree ou repare le soleil (movable, intensite explicite) et la lumiere de ciel."""
    sun = find_by_label("Sun")
    if not sun:
        sun = spawn(unreal.DirectionalLight, (0.0, 0.0, 500.0), (-45.0, 30.0, 0.0), label="Sun")
    sun_component = sun.get_editor_property("light_component")
    sun_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sun_component.set_editor_property("intensity", 10.0)

    sky = find_by_label("Sky")
    if not sky:
        sky = spawn(unreal.SkyLight, (0.0, 0.0, 500.0), label="Sky")
    sky_component = sky.get_editor_property("light_component")
    sky_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

    unreal.log("[bootstrap] eclairage verifie : soleil 10 lux movable + sky movable")


def setup_ground_and_lights():
    place_static((0.0, 0.0, -50.0), (16.0, 16.0, 1.0), CUBE, label="Ground")
    ensure_lighting()
    spawn(unreal.PlayerStart, (0.0, 600.0, 100.0), (0.0, 180.0, 0.0), label="PlayerStart")


def make_artifact_info(artifact_id, title, local, culture, region, description):
    info = unreal.ArtifactInfo()
    info.set_editor_property("artifact_id", artifact_id)
    info.set_editor_property("title_fr", unreal.Text(title))
    info.set_editor_property("title_local", unreal.Text(local))
    info.set_editor_property("culture", unreal.Text(culture))
    info.set_editor_property("region", unreal.Text(region))
    info.set_editor_property("description_fr", unreal.Text(description))
    return info


def artifact(loc, artifact_id, title, local, culture, region, description):
    place_static((loc[0], loc[1], 0.0), (0.6, 0.6, 1.0), CYLINDER, label="Pedestal_" + artifact_id)
    art = spawn(unreal.ArtifactActor, (loc[0], loc[1], 100.0), label="Artifact_" + artifact_id)
    art.set_editor_property("mesh", unreal.EditorAssetLibrary.load_asset(CUBE))
    art.set_actor_scale3d(unreal.Vector(0.4, 0.4, 0.4))
    art.set_editor_property("info", make_artifact_info(artifact_id, title, local, culture, region, description))
    return art


def village_zone(loc, village_id, display_name, description, extent=(600.0, 600.0, 300.0)):
    zone = spawn(unreal.VillageZone, loc, label="Zone_" + village_id)
    zone.configure_zone(village_id, unreal.Text(display_name), unreal.Text(description))
    volume = zone.get_editor_property("zone_volume")
    if volume:
        volume.set_box_extent(unreal.Vector(*extent))
    return zone


def portal(loc, target_level, label_text, label):
    place_static((loc[0], loc[1], 0.0), (1.6, 1.6, 0.06), CYLINDER, label="PortalMarker_" + label)
    p = spawn(unreal.VillagePortal, (loc[0], loc[1], 150.0), label="Portal_" + label)
    p.set_editor_property("target_level_name", target_level)
    p.set_editor_property("portal_label", unreal.Text(label_text))
    trigger = p.get_editor_property("trigger_volume")
    if trigger:
        trigger.set_box_extent(unreal.Vector(120.0, 120.0, 220.0))
    return p


def hut(loc, radius_scale=2.0, wall_height=1.6, label="Hut"):
    place_static((loc[0], loc[1], 0.0), (radius_scale, radius_scale, wall_height), CYLINDER, label=label + "_Wall")
    place_static((loc[0], loc[1], 100.0 * wall_height + 55.0),
                 (radius_scale * 1.3, radius_scale * 1.3, 1.2), CONE, label=label + "_Roof")


def tree(loc, label="Tree"):
    place_static((loc[0], loc[1], 150.0), (0.3, 0.3, 3.0), CYLINDER, label=label + "_Trunk")
    place_static((loc[0], loc[1], 380.0), (3.0, 3.0, 2.6), SPHERE, label=label + "_Foliage")


def find_by_label(prefix):
    for actor in actor_subsys().get_all_level_actors():
        if actor.get_actor_label().startswith(prefix):
            return actor
    return None


# ---------------------------------------------------------------- villages ---

def build_baoule():
    for pos, name in [((-320.0, 320.0), "Case_Nord"), ((320.0, 320.0), "Case_Est"), ((0.0, 480.0), "Case_Djara")]:
        hut(pos, label="Baoule_" + name)
    place_static((0.0, 0.0, -40.0), (7.0, 7.0, 0.2), CYLINDER, label="Baoule_AireGoli")

    artifact((-250.0, 0.0), "baoule_masque_goli", "Masque Goli", "Glao", "Baoulé", "Région de Bouaké",
             "Masque du rituel Goli, dansé pour apaiser les esprits et proteger le village. "
             "Face ronde aux cornes de buffle, bois sculpté.")
    artifact((-250.0, -250.0), "baoule_masque_kpan", "Masque Kpan", "Kpan", "Baoulé", "Région de Bouaké",
             "Masque portrait célébrant la beauté féminine, dansé lors des réjouissances "
             "publiques pour la paix sociale.")
    artifact((250.0, 0.0), "baoule_asie_usu", "Statuette Asiè usu", "Asie usu", "Baoulé", "Région de Bouaké",
             "Épouse spirituelle du devin, statuette du culte bouclo posée sur l'autel "
             "et honorée d'offrandes.")
    artifact((250.0, -250.0), "baoule_pagne_ikat", "Pagne ikat baoulé", "Ntita", "Baoulé", "Région de Bouaké",
             "Tissu de prestige tissé en bandes de coton, motifs teints par réserve. "
             "Vêtement de cérémonie et monnaie d'échange.")

    village_zone((0.0, 0.0), "baoule", "Village Baoulé",
                 "Architecture akan de l'intérieur : cases rondes en terre, toits de chaume, "
                 "aire du Goli au centre du village.")


def build_senoufo():
    place_static((-320.0, 320.0, 0.0), (3.0, 2.5, 1.5), CUBE, label="Senoufo_CaseSacree")
    for pos, name in [((320.0, 320.0), "Grenier_1"), ((470.0, 150.0), "Grenier_2")]:
        place_static((pos[0], pos[1], 0.0), (1.2, 1.2, 1.8), CYLINDER, label="Senoufo_" + name)
        place_static((pos[0], pos[1], 190.0), (1.6, 1.6, 1.0), CONE, label="Senoufo_" + name + "_Roof")
    place_static((-470.0, -150.0, 0.0), (2.0, 1.5, 1.0), CUBE, label="Senoufo_Forge")
    place_static((-470.0, -150.0, 110.0), (0.25, 0.25, 2.0), CYLINDER, label="Senoufo_Forge_Cheminee")

    artifact((-250.0, 0.0), "senoufo_kponiugo", "Masque Kponiugo", "Kponiugo", "Sénoufo", "Région de Korhogo",
             "Le cracheur de feu : masque d'hybride chien-crocodile qui poursuit les "
             "mauvais esprits hors du village.")
    artifact((-250.0, -250.0), "senoufo_masque_poro", "Masque du Poro", "Koroboro", "Sénoufo", "Région de Korhogo",
             "Masque d'initiation de la société du Poro, employé pendant les rites de "
             "passage des jeunes gens.")
    artifact((250.0, 0.0), "senoufo_trompe", "Trompe d'appel", "Sie", "Sénoufo", "Région de Korhogo",
             "Longue trompe de bois dont la voix grave incarne celle des ancêtres "
             "lors des cérémonies du Poro.")
    artifact((250.0, -250.0), "senoufo_dege", "Statuette Dege", "Dege", "Sénoufo", "Région de Korhogo",
             "Statuette de divination utilisée par le devin pour interroger les esprits "
             "et révéler les causes d'un malheur.")

    village_zone((0.0, 0.0), "senoufo", "Village Sénoufo",
                 "Village du nord : case sacrée du Poro, greniers à mil et forge du "
                 "forgeron-fondeur, maitre du feu.")


def build_dan_yacouba():
    for pos, name in [((-320.0, 320.0), "Hutte_Ouest"), ((320.0, 320.0), "Hutte_Est"), ((0.0, 480.0), "Hutte_Nord")]:
        hut(pos, radius_scale=1.8, label="Dan_" + name)
    for i, pos in enumerate([(-600.0, 500.0), (600.0, 500.0), (-600.0, -200.0), (600.0, -200.0),
                             (-450.0, 650.0), (450.0, 650.0), (0.0, -500.0), (700.0, 0.0)]):
        tree(pos, label="Dan_Arbre_%d" % i)

    artifact((-250.0, 0.0), "dan_dean_gle", "Masque Dean Gle", "Dean Gle", "Dan / Yacouba", "Région de Man",
             "Masque de grâce au visage fin et souriant, porté lors des fêtes de "
             "réconciliation. Symbole de paix sociale.")
    artifact((250.0, 0.0), "dan_gunye_ge", "Masque Gunye Ge", "Gunye Ge", "Dan / Yacouba", "Région de Man",
             "Masque de course entre villages : le porteur poursuit un animal libéré, "
             "au rythme des tambours.")
    artifact((0.0, -250.0), "dan_wakemia", "Cuillère d'hospitalité", "Wakemia", "Dan / Yacouba", "Région de Man",
             "Grande cuillère sculptée offerte à l'hôte de marque, emblème de "
             "l'hospitalité et du rang de la maîtresse de maison.")

    village_zone((0.0, 0.0), "dan_yacouba", "Village Dan / Yacouba",
                 "Village de forêt : huttes sous la canopée, clairière des masques, "
                 "paysage des montagnes de Man.")


def build_grand_bassam():
    for pos, name in [((-350.0, 320.0), "Maison_Ouest"), ((350.0, 320.0), "Maison_Est")]:
        place_static((pos[0], pos[1], 100.0), (6.0, 4.0, 2.0), CUBE, label="Bassam_" + name)
        for dx in (-2.0, 2.0):
            for dy in (0.0,):
                place_static((pos[0] + dx * 100.0, pos[1] - 220.0, 100.0),
                             (0.2, 0.2, 1.0), CYLINDER, label="Bassam_%s_Poteau" % name)
    place_static((0.0, -350.0, 0.0), (2.0, 1.5, 1.0), CUBE, label="Bassam_Comptoir")

    artifact((-250.0, 0.0), "bassam_fauteuil_veranda", "Fauteuil de véranda", "—", "Grand-Bassam", "Quartier colonial",
             "Mobilier colonial fin XIXe : fauteuil de véranda en bois tourné, "
             "symbole de la ville capitale de la Côte d'Ivoire française.")
    artifact((250.0, 0.0), "bassam_balance_comptoir", "Balance de comptoir", "—", "Grand-Bassam", "Quartier colonial",
             "Balance des maisons de commerce : pesée du café, du cacao et de "
             "l'huile de palme aux comptoirs de la traite.")
    artifact((0.0, -250.0), "bassam_pirogue", "Pirogue de pêche", "—", "Grand-Bassam", "Lagune Aby/Tendo",
             "Pirogue à balancier des pêcheurs de la lagune, art de la navigation "
             "lagunaire autour de la ville historique.")

    village_zone((0.0, 0.0), "grand_bassam", "Grand-Bassam — Quartier colonial",
                 "Ville historique inscrite à l'UNESCO : maisons à véranda, comptoirs "
                 "commerciaux et mémoire de la première capitale (1893-1896).", extent=(700.0, 700.0, 300.0))


# ------------------------------------------------------------------- hall ----

def build_hall_portals():
    portals = [
        ((-450.0, 0.0), "/Game/Maps/Villages/Baoule_Village", "Village Baoulé", "Baoule"),
        ((450.0, 0.0), "/Game/Maps/Villages/Senoufo_Village", "Village Sénoufo", "Senoufo"),
        ((0.0, 450.0), "/Game/Maps/Villages/Dan_Yacouba_Village", "Village Dan / Yacouba", "DanYacouba"),
        ((0.0, -450.0), "/Game/Maps/GrandBassam_Quartier", "Grand-Bassam", "GrandBassam"),
    ]
    for loc, target, label_text, label in portals:
        if find_by_label("Portal_" + label):
            unreal.log("[bootstrap] portail deja present : " + label)
            continue
        portal(loc, target, label_text, label)

    if not find_by_label("Artifact_baoule_masque_goli"):
        artifact((-250.0, 0.0), "baoule_masque_goli", "Masque Goli (original du hall)", "Glao", "Baoulé",
                 "Région de Bouaké", "Pièce maîtresse du hall : masque Goli baoulé, "
                 "à comparer avec les villages visités.")

    if not find_by_label("Zone_welcome"):
        village_zone((0.0, 0.0), "welcome", "Hall du musée",
                     "Point de départ : portails vers les quatre villages historiques.")


VILLAGE_BUILDERS = {
    "/Game/Maps/Villages/Baoule_Village": build_baoule,
    "/Game/Maps/Villages/Senoufo_Village": build_senoufo,
    "/Game/Maps/Villages/Dan_Yacouba_Village": build_dan_yacouba,
    "/Game/Maps/GrandBassam_Quartier": build_grand_bassam,
}


def main():
    if level_is_new(WELCOME):
        setup_ground_and_lights()
    else:
        ensure_lighting()
        unreal.log("[bootstrap] WelcomeHall existant : ajout/mise a jour des portails")
    build_hall_portals()

    for path, builder in VILLAGE_BUILDERS.items():
        if level_is_new(path):
            setup_ground_and_lights()
        else:
            ensure_lighting()
            unreal.log("[bootstrap] niveau existant : " + path)
        builder()
        portal((0.0, -500.0), WELCOME, "Retour au hall du musée", "RetourHall_" + path.rsplit("/", 1)[-1])

    unreal.get_editor_subsystem(unreal.EditorLevelSubsystem).load_level(WELCOME)
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log("[bootstrap] Musee complet genere : WelcomeHall + 4 villages. Sauvegarde effectuee.")


main()
