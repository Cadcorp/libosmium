#ifndef OSMIUM_BUILDER_OSM_OBJECT_BUILDER_HPP
#define OSMIUM_BUILDER_OSM_OBJECT_BUILDER_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2015 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <cstring>
#include <initializer_list>
#include <new>
#include <string>
#include <utility>

#include <osmium/builder/builder.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/util/cast.hpp>

namespace osmium {

    namespace memory {
        class Buffer;
    }

    namespace builder {

        class TagListBuilder : public ObjectBuilder<TagList> {

        public:

            explicit TagListBuilder(osmium::memory::Buffer& buffer, Builder* parent = nullptr) :
                ObjectBuilder<TagList>(buffer, parent) {
            }

            ~TagListBuilder() {
                add_padding();
            }

            /**
             * Add tag to buffer.
             *
             * @param key Tag key (0-terminated string).
             * @param value Tag value (0-terminated string).
             */
            void add_tag(const char* key, const char* value) {
                if (std::strlen(key) > osmium::max_osm_string_length) {
                    throw std::length_error("OSM tag key is too long");
                }
                if (std::strlen(value) > osmium::max_osm_string_length) {
                    throw std::length_error("OSM tag value is too long");
                }
                add_size(append(key) + append(value));
            }

            /**
             * Add tag to buffer.
             *
             * @param key Pointer to tag key.
             * @param key_length Length of key (not including the \0 byte).
             * @param value Pointer to tag value.
             * @param value_length Length of value (not including the \0 byte).
             */
            void add_tag(const char* key, const size_t key_length, const char* value, const size_t value_length) {
                if (key_length > osmium::max_osm_string_length) {
                    throw std::length_error("OSM tag key is too long");
                }
                if (value_length > osmium::max_osm_string_length) {
                    throw std::length_error("OSM tag value is too long");
                }
                add_size(append(key,   osmium::memory::item_size_type(key_length))   + append_zero() +
                         append(value, osmium::memory::item_size_type(value_length)) + append_zero());
            }

            /**
             * Add tag to buffer.
             *
             * @param key Tag key.
             * @param value Tag value.
             */
            void add_tag(const std::string& key, const std::string& value) {
                if (key.size() > osmium::max_osm_string_length) {
                    throw std::length_error("OSM tag key is too long");
                }
                if (value.size() > osmium::max_osm_string_length) {
                    throw std::length_error("OSM tag value is too long");
                }
                add_size(append(key.data(),   osmium::memory::item_size_type(key.size())   + 1) +
                         append(value.data(), osmium::memory::item_size_type(value.size()) + 1));
            }

        }; // class TagListBuilder

        template <class T>
        class NodeRefListBuilder : public ObjectBuilder<T> {

        public:

            explicit NodeRefListBuilder(osmium::memory::Buffer& buffer, Builder* parent = nullptr) :
                ObjectBuilder<T>(buffer, parent) {
            }

            ~NodeRefListBuilder() {
                static_cast<Builder*>(this)->add_padding();
            }

            void add_node_ref(const NodeRef& node_ref) {
                new (static_cast<Builder*>(this)->reserve_space_for<osmium::NodeRef>()) osmium::NodeRef(node_ref);
                static_cast<Builder*>(this)->add_size(sizeof(osmium::NodeRef));
            }

            void add_node_ref(const object_id_type ref, const osmium::Location location = Location()) {
                add_node_ref(NodeRef(ref, location));
            }

        }; // class NodeRefListBuilder

        typedef NodeRefListBuilder<WayNodeList> WayNodeListBuilder;
        typedef NodeRefListBuilder<OuterRing> OuterRingBuilder;
        typedef NodeRefListBuilder<InnerRing> InnerRingBuilder;

        class RelationMemberListBuilder : public ObjectBuilder<RelationMemberList> {

            /**
             * Add role to buffer.
             *
             * @param member Relation member object where the length of the role
             *               will be set.
             * @param role The role.
             * @param length Length of role (without \0 termination).
             * @throws std:length_error If role is longer than osmium::max_osm_string_length
             */
            void add_role(osmium::RelationMember& member, const char* role, const size_t length) {
                if (length > osmium::max_osm_string_length) {
                    throw std::length_error("OSM relation member role is too long");
                }
                member.set_role_size(osmium::string_size_type(length) + 1);
                add_size(append(role, osmium::memory::item_size_type(length)) + append_zero());
                add_padding(true);
            }

        public:

            explicit RelationMemberListBuilder(osmium::memory::Buffer& buffer, Builder* parent = nullptr) :
                ObjectBuilder<RelationMemberList>(buffer, parent) {
            }

            ~RelationMemberListBuilder() {
                add_padding();
            }

            /**
             * Add a member to the relation.
             *
             * @param type The type (node, way, or relation).
             * @param ref The ID of the member.
             * @param role The role of the member.
             * @param role_length Length of the role (without \0 termination).
             * @param full_member Optional pointer to the member object. If it
             *                    is available a copy will be added to the
             *                    relation.
             * @throws std:length_error If role_length is greater than
             *         osmium::max_osm_string_length
             */
            void add_member(osmium::item_type type, object_id_type ref, const char* role, const size_t role_length, const osmium::OSMObject* full_member = nullptr) {
                osmium::RelationMember* member = reserve_space_for<osmium::RelationMember>();
                new (member) osmium::RelationMember(ref, type, full_member != nullptr);
                add_size(sizeof(RelationMember));
                add_role(*member, role, role_length);
                if (full_member) {
                    add_item(full_member);
                }
            }

            /**
             * Add a member to the relation.
             *
             * @param type The type (node, way, or relation).
             * @param ref The ID of the member.
             * @param role The role of the member (\0 terminated string).
             * @param full_member Optional pointer to the member object. If it
             *                    is available a copy will be added to the
             *                    relation.
             * @throws std:length_error If role is longer than osmium::max_osm_string_length
             */
            void add_member(osmium::item_type type, object_id_type ref, const char* role, const osmium::OSMObject* full_member = nullptr) {
                add_member(type, ref, role, std::strlen(role), full_member);
            }

            /**
             * Add a member to the relation.
             *
             * @param type The type (node, way, or relation).
             * @param ref The ID of the member.
             * @param role The role of the member.
             * @param full_member Optional pointer to the member object. If it
             *                    is available a copy will be added to the
             *                    relation.
             * @throws std:length_error If role is longer than osmium::max_osm_string_length
             */
            void add_member(osmium::item_type type, object_id_type ref, const std::string& role, const osmium::OSMObject* full_member = nullptr) {
                add_member(type, ref, role.data(), role.size(), full_member);
            }

        }; // class RelationMemberListBuilder

        template <class T>
        class OSMObjectBuilder : public ObjectBuilder<T> {

        public:

            explicit OSMObjectBuilder(osmium::memory::Buffer& buffer, Builder* parent = nullptr) :
                ObjectBuilder<T>(buffer, parent) {
                static_cast<Builder*>(this)->reserve_space_for<string_size_type>();
                static_cast<Builder*>(this)->add_size(sizeof(string_size_type));
            }

            void add_tags(const std::initializer_list<std::pair<const char*, const char*>>& tags) {
                osmium::builder::TagListBuilder tl_builder(static_cast<Builder*>(this)->buffer(), this);
                for (const auto& p : tags) {
                    tl_builder.add_tag(p.first, p.second);
                }
            }

        }; // class OSMObjectBuilder

        typedef OSMObjectBuilder<osmium::Node> NodeBuilder;
        typedef OSMObjectBuilder<osmium::Relation> RelationBuilder;

        class WayBuilder : public OSMObjectBuilder<osmium::Way> {

        public:

            explicit WayBuilder(osmium::memory::Buffer& buffer, Builder* parent = nullptr) :
                OSMObjectBuilder<osmium::Way>(buffer, parent) {
            }

            void add_node_refs(const std::initializer_list<osmium::NodeRef>& nodes) {
                osmium::builder::WayNodeListBuilder builder(buffer(), this);
                for (const auto& node_ref : nodes) {
                    builder.add_node_ref(node_ref);
                }
            }

        }; // class WayBuilder

        class AreaBuilder : public OSMObjectBuilder<osmium::Area> {

        public:

            explicit AreaBuilder(osmium::memory::Buffer& buffer, Builder* parent = nullptr) :
                OSMObjectBuilder<osmium::Area>(buffer, parent) {
            }

            /**
             * Initialize area attributes from the attributes of the given object.
             */
            void initialize_from_object(const osmium::OSMObject& source) {
                osmium::Area& area = object();
                area.set_id(osmium::object_id_to_area_id(source.id(), source.type()));
                area.set_version(source.version());
                area.set_changeset(source.changeset());
                area.set_timestamp(source.timestamp());
                area.set_visible(source.visible());
                area.set_uid(source.uid());

                add_user(source.user());
            }

        }; // class AreaBuilder

        typedef ObjectBuilder<osmium::Changeset> ChangesetBuilder;

    } // namespace builder

} // namespace osmium

#endif // OSMIUM_BUILDER_OSM_OBJECT_BUILDER_HPP
