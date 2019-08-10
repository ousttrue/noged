#include "plugnode.h"
#include "nodegraph.h"
#include "nodedefinition.h"
#include <perilune/perilune.h>

namespace perilune
{

template <>
struct LuaTable<plugnode::NodeSocket>
{
    static plugnode::NodeSocket Get(lua_State *L, int index)
    {
        auto table = LuaTableToTuple<std::string, std::string>(L, index);
        return plugnode::NodeSocket{
            std::get<0>(table),
            std::get<1>(table),
        };
    }
};

} // namespace perilune

void lua_require_plugnode(lua_State *L)
{
    auto top = lua_gettop(L);
    lua_newtable(L);

#pragma region definitions
    static perilune::UserType<plugnode::NodeSocket> nodeSocket;
    nodeSocket
        .StaticMethod("new", [](std::string name, std::string type) {
            return plugnode::NodeSocket{name, type};
        })
        .MetaMethod(perilune::MetaKey::__tostring, [](plugnode::NodeSocket *socket) {
            std::stringstream ss;
            ss << "[" << socket->type << "]" << socket->name;
            return ss.str();
        })
        .LuaNewType(L);
    lua_setfield(L, -2, "node_socket");

    static perilune::UserType<std::vector<plugnode::NodeSocket> *>
        stringList;
    perilune::AddDefaultMethods(stringList);
    stringList
        .LuaNewType(L);
    lua_setfield(L, -2, "string_list");

    static perilune::UserType<std::shared_ptr<plugnode::NodeDefinition>> nodeDefinition;
    nodeDefinition
        .MetaIndexDispatcher([](auto d) {
            d->Getter("name", &plugnode::NodeDefinition::Name);
            d->Getter("inputs", [](plugnode::NodeDefinition *p) { return &p->Inputs; });
            d->Getter("outputs", [](plugnode::NodeDefinition *p) { return &p->Outputs; });
        })
        .LuaNewType(L);
    lua_setfield(L, -2, "definition");

    static perilune::UserType<plugnode::NodeDefinitionManager *> definitions;
    definitions
        .DefaultConstructorAndDestructor()
        .MetaIndexDispatcher([](auto d) {
            d->Method("create", [](plugnode::NodeDefinitionManager *p, std::string name) {
                return p->Create(name);
            });
            d->Method("get_count", &plugnode::NodeDefinitionManager::GetCount);
            d->IndexGetter([](plugnode::NodeDefinitionManager *l, int i) {
                return l->Get(i - 1);
            });
        })
        .LuaNewType(L);
    lua_setfield(L, -2, "definition_manager");
#pragma endregion

#pragma region scene
    static perilune::UserType<std::shared_ptr<plugnode::Node>> node;
    node
        .LuaNewType(L);
    lua_setfield(L, -2, "node");

    static perilune::UserType<std::shared_ptr<plugnode::NodeLink>> link;
    link
        .LuaNewType(L);
    lua_setfield(L, -2, "link");

    static perilune::UserType<plugnode::NodeScene *> nodescene;
    nodescene
        .DefaultConstructorAndDestructor()
        .MetaIndexDispatcher([](auto d) {
            d->Method("create", [](plugnode::NodeScene *p, const std::shared_ptr<plugnode::NodeDefinition> &def, float x, float y) {
                return p->CreateNode(def, x, y);
            });
            d->Method("link", [](plugnode::NodeScene *p,
                                 const std::shared_ptr<plugnode::Node> &src, int src_slot,
                                 const std::shared_ptr<plugnode::Node> &dst, int dst_slot) {
                return p->Link(src, src_slot - 1, dst, dst_slot - 1);
            });
        })
        .LuaNewType(L);
    lua_setfield(L, -2, "scene");
#pragma endregion

    static perilune::UserType<plugnode::NodeGraph *> nodegraph;
    nodegraph
        .DefaultConstructorAndDestructor()
        .MetaIndexDispatcher([](auto d) {
            d->Method("imgui", &plugnode::NodeGraph::ImGui);
        })
        .LuaNewType(L);
    lua_setfield(L, -2, "graph");

    lua_setglobal(L, "plugnode");
    assert(top == lua_gettop(L));
}