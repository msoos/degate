/*

  This file is part of the IC reverse engineering tool degate.

  Copyright 2008, 2009, 2010 by Martin Schobert

  Degate is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  Degate is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with degate. If not, see <http://www.gnu.org/licenses/>.

*/

#include <degate.h>
#include "LogicModelDOTExporter.h"
#include "DOTAttributes.h"
#include "FileSystem.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <list>
#include <tr1/memory>

using namespace std;
using namespace degate;

void LogicModelDOTExporter::export_data(
    std::string const& filename
    , LogicModel_shptr lmodel
) {
    if (lmodel == NULL)
        throw InvalidPointerException("Logic model pointer is NULL.");

    string basename(get_basename(filename));
    std::ostringstream stm;
    stm << "time neato -v -Tsvg"
        << " -o " << basename << ".svg"
        << " " << basename << ".dot"
        ;


    add_header_line("");
    add_header_line("This is a logic model export.");
    add_header_line("");
    add_header_line("You can generate an image of this graph with:");
    add_header_line(stm.str());
    add_header_line("");

    add_graph_setting("");

    try {
        // iterate over logic model objects
        for(LogicModel::object_collection::iterator
            iter = lmodel->objects_begin()
            ; iter != lmodel->objects_end()
            ; ++iter
        ) {
            PlacedLogicModelObject_shptr o = (*iter).second;

            if (Gate_shptr gate = std::tr1::dynamic_pointer_cast<Gate>(o)) {
                add_gate(gate);
            }
        }

        for(std::map<object_id_t, std::vector<GateConn> >::const_iterator
            it = via_to_gates.begin(), end = via_to_gates.end()
            ; it != end
            ; it++
        ) {
            object_id_t from_id;
            std::string from_connection_name;
            bool found_from = false;
            bool clock = false;

            const std::vector<GateConn>& gateConn = it->second;
            for(vector<GateConn>::const_iterator
                it2 = gateConn.begin(), end2 = gateConn.end()
                ; it2 != end2
                ; it2++
            ) {
                if (it2->connection_name == "clk") {
                    clock = true;
                    break;
                }

                if (it2->connection_name != "a"
                    && it2->connection_name != "b"
                    && it2->connection_name != "c"
                    && it2->connection_name != "d"
                    && it2->connection_name != "e"
                    && it2->connection_name != "f"
                    && it2->connection_name != "D"
                    && it2->connection_name != "1"
                    && it2->connection_name != "0"
                    && it2->connection_name != "rst"
                    && it2->connection_name != ""
                ) {
                    from_id = it2->gate_id;
                    from_connection_name = it2->connection_name;

                    found_from = true;
                    break;
                }
            }
            if (clock) {
                cout
                << "Not parsing CLK!"
                << endl;
                continue;
            }

            if (!found_from) {
                cout
                << "Not found FROM :( -- Skipping via"
                << endl;

                continue;
            }
            string from_name(oid_to_str("G", from_id));

            for(vector<GateConn>::const_iterator
                it2 = gateConn.begin(), end2 = gateConn.end()
                ; it2 != end2
                ; it2++
            ) {
                //Skip gate the connection is coming from
                if (it2->gate_id == from_id)
                    continue;

                string to_name(oid_to_str("G", it2->gate_id));
                DOTAttributes edge_attrs;
                edge_attrs.add("headlabel", it2->connection_name);
                edge_attrs.add("taillabel", from_connection_name);
                add_edge(from_name, to_name, edge_attrs.get_string());
            }
        }

        dump_to_file(filename);

    }
    catch(const std::exception& ex)
    {
        std::cout << "Exception caught: " << ex.what() << std::endl;
        throw;
    }

}

std::string LogicModelDOTExporter::oid_to_str(
    std::string const& prefix
    , object_id_t oid
) {
    std::ostringstream stm;
    stm << prefix << oid_rewriter->get_new_object_id(oid) << " ";
    return stm.str();
}

void LogicModelDOTExporter::add_gate(Gate_shptr gate)
{
    object_id_t gate_id = gate->get_object_id();
    string node_name(oid_to_str("G", gate_id));

    std::ostringstream stm;
    stm << (gate->has_name() ? gate->get_name() : node_name);

    if (gate->has_template()) {
        const GateTemplate_shptr tmpl = gate->get_gate_template();
        stm << "\\n" << tmpl->get_name();
    }

    DOTAttributes attrs;
    attrs.add("shape", "box");
    attrs.add("label", stm.str());
    add_node(node_name, attrs.get_string());

    //Edges
    for(Gate::port_iterator
        piter = gate->ports_begin()
        ; piter != gate->ports_end()
        ; ++piter
    ) {
        GatePort_shptr gate_port = *piter;

        //Not available, skip: Oops, net is NULL
        if (!gate_port->get_net()) {
            cout << "OOps, NET is null, skipping" << endl;
            continue;
        }

        object_id_t via_id   = gate_port->get_net()->get_object_id();
        std::string conn_name = gate_port->get_template_port()->get_name();

        std::map<object_id_t, std::vector<GateConn> >::iterator it =
            via_to_gates.find(via_id);

        if (it == via_to_gates.end()) {
            std::vector<GateConn> tmp;
            tmp.push_back(GateConn(gate_id, conn_name));
            via_to_gates[via_id] = tmp;
        } else {
            it->second.push_back(GateConn(gate_id, conn_name));
        }
    }
}
