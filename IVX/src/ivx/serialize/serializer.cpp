#include "ivxpch.h"
#include "serializer.h"

#include "ivx/core/logger.h"
#include "ivx/scene/components.h"
#include "ivx/scene/entity.h"
#include "ivx/scene/scriptable_entity.h" // NativeScriptComponent definition
#include "ivx/utils/math.h"
#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ivx {
namespace {

const char *light_type_name(LightType type) {
    switch (type) {
    case LightType::IVX_Point:
        return "Point";
    case LightType::IVX_Spot:
        return "Spot";
    case LightType::IVX_Directional:
    default:
        return "Directional";
    }
}

LightType light_type_from_name(const std::string &name) {
    if (name == "Point") {
        return LightType::IVX_Point;
    }
    if (name == "Spot") {
        return LightType::IVX_Spot;
    }
    return LightType::IVX_Directional;
}

// ---- writing ---------------------------------------------------------------

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &value) {
    out << YAML::Flow << YAML::BeginSeq << value.x << value.y << YAML::EndSeq;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &value) {
    out << YAML::Flow << YAML::BeginSeq << value.x << value.y << value.z
        << YAML::EndSeq;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &value) {
    out << YAML::Flow << YAML::BeginSeq << value.x << value.y << value.z
        << value.w << YAML::EndSeq;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out,
                          const TransformComponent &transform) {
    out << YAML::Key << "Transform" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Translation" << YAML::Value << transform.translation;
    // Degrees on disk: the file is meant to be read and edited by hand, the
    // engine keeps radians.
    out << YAML::Key << "Rotation" << YAML::Value
        << glm::degrees(transform.rotation);
    out << YAML::Key << "Scale" << YAML::Value << transform.scale;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out,
                          const CameraComponent &component) {
    const SceneCamera &camera = component.camera;

    out << YAML::Key << "Camera" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Projection" << YAML::Value
        << (camera.get_projection_type() ==
                    SceneCamera::ProjectionType::Perspective
                ? "Perspective"
                : "Orthographic");

    // Both sets are written even though only one is active: switching
    // projection in the editor is a dropdown, and losing the other type's
    // numbers on every save would be annoying.
    out << YAML::Key << "Orthographic" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Size" << YAML::Value << camera.get_orthographic_size();
    out << YAML::Key << "Near" << YAML::Value << camera.get_orthographic_near();
    out << YAML::Key << "Far" << YAML::Value << camera.get_orthographic_far();
    out << YAML::EndMap;

    out << YAML::Key << "Perspective" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "VerticalFov" << YAML::Value
        << camera.get_perspective_vertical_fov();
    out << YAML::Key << "Near" << YAML::Value << camera.get_perspective_near();
    out << YAML::Key << "Far" << YAML::Value << camera.get_perspective_far();
    out << YAML::EndMap;

    out << YAML::Key << "Primary" << YAML::Value << component.primary;
    out << YAML::Key << "FixedAspectRatio" << YAML::Value
        << component.fixed_aspect_ratio;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out,
                          const SpriteRendererComponent &sprite) {
    out << YAML::Key << "SpriteRenderer" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Size" << YAML::Value << sprite.size;
    out << YAML::Key << "TilingFactor" << YAML::Value << sprite.tiling_factor;
    out << YAML::Key << "Color" << YAML::Value << sprite.color;

    // Informational only: it tells the reader that this quad *had* a texture,
    // so loading can warn instead of pretending the flat colour version is the
    // original.
    out << YAML::Key << "HasTexture" << YAML::Value
        << static_cast<bool>(sprite.sub_texture || sprite.texture);
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out,
                          const NativeScriptComponent &script) {
    out << YAML::Key << "NativeScript" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Script" << YAML::Value << script.script_name;
    out << YAML::Key << "Enabled" << YAML::Value << script.enabled;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out,
                          const MeshRendererComponent &mesh_renderer) {
    const Mesh *mesh = mesh_renderer.mesh.get();
    const MeshPrimitive primitive =
        mesh ? mesh->get_primitive() : MeshPrimitive::IVX_Custom;

    out << YAML::Key << "MeshRenderer" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Primitive" << YAML::Value
        << (primitive == MeshPrimitive::IVX_Cube ? "Cube" : "Custom");
    out << YAML::Key << "PrimitiveSize" << YAML::Value
        << (mesh ? mesh->get_primitive_size() : 0.f);
    out << YAML::Key << "Albedo" << YAML::Value
        << mesh_renderer.material.albedo;
    out << YAML::Key << "TilingFactor" << YAML::Value
        << mesh_renderer.material.tiling_factor;
    // Informational only: the texture is a device handle with no path yet.
    out << YAML::Key << "HasAlbedoMap" << YAML::Value
        << static_cast<bool>(mesh_renderer.material.albedo_map);
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out,
                          const LightComponent &light_component) {
    const LightSettings &settings = light_component.settings;

    out << YAML::Key << "Light" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Type" << YAML::Value << light_type_name(settings.type);
    out << YAML::Key << "Color" << YAML::Value << settings.color;
    out << YAML::Key << "Intensity" << YAML::Value << settings.intensity;
    out << YAML::Key << "Constant" << YAML::Value << settings.constant;
    out << YAML::Key << "Linear" << YAML::Value << settings.linear;
    out << YAML::Key << "Quadratic" << YAML::Value << settings.quadratic;
    out << YAML::Key << "InnerAngle" << YAML::Value << settings.inner_angle;
    out << YAML::Key << "OuterAngle" << YAML::Value << settings.outer_angle;
    out << YAML::Key << "CastShadows" << YAML::Value
        << light_component.cast_shadows;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, Entity entity) {
    out << YAML::BeginMap;
    out << YAML::Key << "Entity" << YAML::Value
        << entity.get_component<IdComponent>().uid;

    out << YAML::Key << "Tag" << YAML::Value
        << entity.get_component<TagComponent>().value;

    out << entity.get_component<TransformComponent>();

    out << YAML::Key << "Relationship" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Parent" << YAML::Value;

    if (const auto &parent =
            entity.get_component<RelationshipComponent>().parent;
        parent != entt::null && entity.scene()) {
        // The parent's uid, not its entt handle: the reader keys entities by
        // uid.
        out << Entity{parent, entity.scene()}.get_component<IdComponent>().uid;
    } else {
        out << YAML::Null; // root
    }
    out << YAML::EndMap;

    if (auto sprite = entity.try_get_component<SpriteRendererComponent>()) {
        out << *sprite;
    }
    if (auto mesh_renderer =
            entity.try_get_component<MeshRendererComponent>()) {
        out << *mesh_renderer;
    }
    if (auto camera = entity.try_get_component<CameraComponent>()) {
        out << *camera;
    }
    if (auto light = entity.try_get_component<LightComponent>()) {
        out << *light;
    }
    if (auto native_script =
            entity.try_get_component<NativeScriptComponent>()) {
        out << *native_script;
    }

    out << YAML::EndMap;

    return out;
}

// ---- reading ---------------------------------------------------------------

glm::vec2 read_vec2(const YAML::Node &node, const glm::vec2 &fallback) {
    if (!node || !node.IsSequence() || node.size() != 2) {
        return fallback;
    }
    return glm::vec2{node[0].as<float>(), node[1].as<float>()};
}

glm::vec3 read_vec3(const YAML::Node &node, const glm::vec3 &fallback) {
    if (!node || !node.IsSequence() || node.size() != 3) {
        return fallback;
    }
    return glm::vec3{
        node[0].as<float>(), node[1].as<float>(), node[2].as<float>()};
}

glm::vec4 read_vec4(const YAML::Node &node, const glm::vec4 &fallback) {
    if (!node || !node.IsSequence() || node.size() != 4) {
        return fallback;
    }
    return glm::vec4{node[0].as<float>(),
                     node[1].as<float>(),
                     node[2].as<float>(),
                     node[3].as<float>()};
}

template <typename T>
T read_scalar(const YAML::Node &node, T fallback) {
    if (!node) {
        return fallback;
    }
    return node.as<T>();
}

void read_transform(const YAML::Node &node, TransformComponent &transform) {
    if (!node) {
        return;
    }
    transform.translation =
        read_vec3(node["Translation"], transform.translation);

    // Degrees on disk, radians in the component. Wrapping keeps a hand written
    // multi turn value showing as a sane angle in the inspector.
    const glm::vec3 degrees =
        read_vec3(node["Rotation"], glm::degrees(transform.rotation));
    transform.rotation = glm::radians(glm::vec3{wrap_degrees(degrees.x),
                                                wrap_degrees(degrees.y),
                                                wrap_degrees(degrees.z)});

    transform.scale = read_vec3(node["Scale"], transform.scale);
}

void read_camera(const YAML::Node &node, CameraComponent &component) {
    if (!node) {
        return;
    }
    SceneCamera &camera = component.camera;

    if (const YAML::Node projection = node["Projection"]; projection) {
        camera.set_projection_type(
            projection.as<std::string>() == "Perspective"
                ? SceneCamera::ProjectionType::Perspective
                : SceneCamera::ProjectionType::Orthographic);
    }

    const YAML::Node orthographic = node["Orthographic"];
    if (orthographic) {
        camera.set_orthographic_size(
            read_scalar(orthographic["Size"], camera.get_orthographic_size()));
        camera.set_orthographic_near(
            read_scalar(orthographic["Near"], camera.get_orthographic_near()));
        camera.set_orthographic_far(
            read_scalar(orthographic["Far"], camera.get_orthographic_far()));
    }

    const YAML::Node perspective = node["Perspective"];
    if (perspective) {
        camera.set_perspective_vertical_fov(read_scalar(
            perspective["VerticalFov"], camera.get_perspective_vertical_fov()));
        camera.set_perspective_near(
            read_scalar(perspective["Near"], camera.get_perspective_near()));
        camera.set_perspective_far(
            read_scalar(perspective["Far"], camera.get_perspective_far()));
    }

    component.primary = read_scalar(node["Primary"], component.primary);
    component.fixed_aspect_ratio =
        read_scalar(node["FixedAspectRatio"], component.fixed_aspect_ratio);
}

void read_sprite(const YAML::Node &node, SpriteRendererComponent &sprite) {
    if (!node) {
        return;
    }
    sprite.size = read_vec2(node["Size"], sprite.size);
    sprite.tiling_factor =
        read_scalar(node["TilingFactor"], sprite.tiling_factor);
    sprite.color = read_vec4(node["Color"], sprite.color);
}

void read_mesh_renderer(const YAML::Node &node,
                        MeshRendererComponent &mesh_renderer) {
    if (!node) {
        return;
    }
    Material &material = mesh_renderer.material;
    // "Color" is the pre material field name.
    material.albedo =
        read_vec4(node["Albedo"], read_vec4(node["Color"], material.albedo));
    material.tiling_factor =
        read_scalar(node["TilingFactor"], material.tiling_factor);

    // Only primitives are stored; a mesh built from raw vertex data has nothing
    // to reload.
    const std::string primitive =
        read_scalar(node["Primitive"], std::string{"Custom"});
    const float size = read_scalar(node["PrimitiveSize"], 1.f);
    if (primitive == "Cube") {
        mesh_renderer.mesh = Mesh::create_cube(size > 0.f ? size : 1.f);
    } else {
        mesh_renderer.mesh.reset();
    }
}

void read_light(const YAML::Node &node, LightComponent &light_component) {
    if (!node) {
        return;
    }

    LightSettings &settings = light_component.settings;
    settings.type = light_type_from_name(
        read_scalar(node["Type"], std::string{"Directional"}));
    settings.color = read_vec3(node["Color"], settings.color);
    settings.intensity = read_scalar(node["Intensity"], settings.intensity);
    settings.constant = read_scalar(node["Constant"], settings.constant);
    settings.linear = read_scalar(node["Linear"], settings.linear);
    settings.quadratic = read_scalar(node["Quadratic"], settings.quadratic);
    settings.inner_angle =
        read_scalar(node["InnerAngle"], settings.inner_angle);
    settings.outer_angle =
        read_scalar(node["OuterAngle"], settings.outer_angle);
    light_component.cast_shadows =
        read_scalar(node["CastShadows"], light_component.cast_shadows);
}

void read_script(const YAML::Node &node, NativeScriptComponent &script) {
    if (!node) {
        return;
    }
    // The instance (and therefore the type) is unknown here: the host binds by
    // name after the scene is loaded, which is also how a hot reload restores
    // bindings.
    script.script_name = read_scalar(node["Script"], script.script_name);
    script.enabled = read_scalar(node["Enabled"], script.enabled);
}

} // namespace

Ref<Scene> Serializer::deserialize(const std::filesystem::path &filepath) {
    YAML::Node root;
    try {
        root = YAML::LoadFile(filepath.string());
    } catch (const YAML::Exception &exception) {
        IVX_ERROR("Scene load failed: '{}' is not usable YAML ({})",
                  filepath.string(),
                  exception.what());
        return {};
    }

    if (!root.IsMap() || !root["Entities"].IsSequence()) {
        IVX_ERROR("Scene load failed: '{}' has no 'Entities' sequence",
                  filepath.string());
        return {};
    }

    if (const YAML::Node format = root["Format"];
        format && format.as<int>() > k_format_version) {
        IVX_WARN(
            "Scene '{}': format {} is newer than {}; loading with defaults for "
            "anything unknown",
            filepath.string(),
            format.as<int>(),
            k_format_version);
    }

    Ref<Scene> scene = make_ref<Scene>();
    if (const YAML::Node name = root["Scene"]) {
        scene->set_name(name.as<std::string>());
    }
    if (const YAML::Node environment = root["Environment"]) {
        Environment &target = scene->environment();
        target.ambient = read_vec3(environment["Ambient"], target.ambient);
        target.exposure = read_scalar(environment["Exposure"], target.exposure);
    }

    std::unordered_map<std::uint32_t, Entity> by_id;
    std::uint32_t next_id = 0;
    std::uint32_t max_uid = 0;
    std::size_t textured_sprites = 0;

    // Pass 1: entities and their components. Parent links are resolved
    // afterwards, so the file may list a child before its parent.
    for (const YAML::Node &node : root["Entities"]) {
        if (!node.IsMap()) {
            continue;
        }

        const std::uint32_t uid =
            read_scalar<std::uint32_t>(node["Entity"], next_id);
        ++next_id;
        max_uid = std::max(max_uid, uid);

        const std::string tag =
            read_scalar<std::string>(node["Tag"], std::string{"Entity"});
        Entity entity = scene->create_entity(tag);
        entity.get_component<IdComponent>().uid = uid;
        by_id[uid] = entity;

        try {
            if (const YAML::Node transform = node["Transform"];
                transform && !entity.has_component<TransformComponent>()) {
                entity.add_component<TransformComponent>();
            }
            if (auto *transform =
                    entity.try_get_component<TransformComponent>()) {
                read_transform(node["Transform"], *transform);
            }

            if (node["SpriteRenderer"]) {
                SpriteRendererComponent &sprite =
                    entity.has_component<SpriteRendererComponent>()
                        ? entity.get_component<SpriteRendererComponent>()
                        : entity.add_component<SpriteRendererComponent>();
                read_sprite(node["SpriteRenderer"], sprite);
                if (read_scalar<bool>(node["SpriteRenderer"]["HasTexture"],
                                      false)) {
                    ++textured_sprites;
                }
            }

            if (node["Light"]) {
                LightComponent &light =
                    entity.has_component<LightComponent>()
                        ? entity.get_component<LightComponent>()
                        : entity.add_component<LightComponent>();
                read_light(node["Light"], light);
            }

            if (node["Camera"]) {
                CameraComponent &camera =
                    entity.has_component<CameraComponent>()
                        ? entity.get_component<CameraComponent>()
                        : entity.add_component<CameraComponent>();
                read_camera(node["Camera"], camera);
            }

            if (node["MeshRenderer"]) {
                MeshRendererComponent &mesh_renderer =
                    entity.has_component<MeshRendererComponent>()
                        ? entity.get_component<MeshRendererComponent>()
                        : entity.add_component<MeshRendererComponent>();
                read_mesh_renderer(node["MeshRenderer"], mesh_renderer);
            }

            if (node["NativeScript"]) {
                NativeScriptComponent &script =
                    entity.has_component<NativeScriptComponent>()
                        ? entity.get_component<NativeScriptComponent>()
                        : entity.add_component<NativeScriptComponent>();
                read_script(node["NativeScript"], script);
            }
        } catch (const YAML::Exception &exception) {
            IVX_ERROR(
                "Scene '{}': entity {} ('{}') has an unreadable component "
                "({}); the rest of it was loaded",
                filepath.string(),
                uid,
                tag,
                exception.what());
        }
    }

    // Pass 2: hierarchy. set_parent() maintains both directions (and rejects
    // cycles), so this also repairs files edited by hand.
    for (const YAML::Node &node : root["Entities"]) {
        const YAML::Node parent_node = node["Relationship"]["Parent"];
        if (!node.IsMap() || !parent_node || parent_node.IsNull()) {
            continue;
        }

        const std::uint32_t child_id =
            read_scalar<std::uint32_t>(node["Entity"], 0);
        const std::uint32_t parent_id = parent_node.as<std::uint32_t>();

        const auto child = by_id.find(child_id);
        const auto parent = by_id.find(parent_id);
        if (child == by_id.end() || parent == by_id.end()) {
            IVX_WARN("Scene '{}': entity {} points at missing parent {}",
                     filepath.string(),
                     child_id,
                     parent_id);
            continue;
        }
        if (child->second == parent->second) {
            continue; // self parent, ignore
        }
        scene->set_parent(child->second, parent->second);
    }

    // New entities must not reuse a uid that came from the file.
    scene->reserve_uid(max_uid);
    scene->update_world_transforms();

    IVX_INFO("Scene loaded: {} ({} entities)", filepath.string(), by_id.size());
    if (textured_sprites > 0) {
        IVX_WARN("Scene '{}': {} sprite(s) referenced a texture, which is not "
                 "stored "
                 "in scene files yet — they load as flat colour quads",
                 filepath.string(),
                 textured_sprites);
    }
    return scene;
}

void Serializer::serialize(const Ref<Scene> &scene,
                           const std::filesystem::path &filepath) {
    if (!scene) {
        IVX_ERROR("Scene save failed: no scene");
        return;
    }

    Scene &world = *scene;

    std::vector<Entity> order;
    const auto collect = [&](auto &&self, Entity e) -> void {
        order.push_back(e);
        for (entt::entity child : e.children()) {
            if (world.registry().valid(child)) {
                self(self, Entity{child, &world});
            }
        }
    };
    for (Entity root : world.get_root_entities()) {
        collect(collect, root);
    }

    YAML::Emitter out;
    out.SetIndent(2);
    out << YAML::BeginMap;
    out << YAML::Key << "Format" << YAML::Value << k_format_version;
    out << YAML::Key << "Scene" << YAML::Value << world.get_name();
    out << YAML::Key << "Environment" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Ambient" << YAML::Value << world.environment().ambient;
    out << YAML::Key << "Exposure" << YAML::Value
        << world.environment().exposure;
    out << YAML::EndMap;
    out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
    for (Entity entity : order) {
        out << entity;
    }
    out << YAML::EndSeq;
    out << YAML::EndMap;

    if (!out.good()) {
        IVX_ERROR("Scene save failed: {}", out.GetLastError());
        return;
    }

    std::error_code ignored;
    if (const std::filesystem::path parent = filepath.parent_path();
        !parent.empty()) {
        std::filesystem::create_directories(parent, ignored);
    }

    std::ofstream file{filepath, std::ios::trunc | std::ios::binary};
    if (!file) {
        IVX_ERROR("Scene save failed: cannot write '{}'", filepath.string());
        return;
    }
    file << out.c_str() << '\n';
    if (!file) {
        IVX_ERROR("Scene save failed: write error on '{}'", filepath.string());
        return;
    }

    IVX_INFO("Scene saved: {} ({} entities)", filepath.string(), order.size());
}
} // namespace ivx
