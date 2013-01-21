/* -*-c++-*-

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

#ifndef __LOGICMODELDOTEXPORTER_H__
#define __LOGICMODELDOTEXPORTER_H__

#include "globals.h"
#include "LogicModel.h"
#include "DOTExporter.h"
#include "ObjectIDRewriter.h"
#include "Layer.h"

#include <stdexcept>

namespace degate {

/**
 * The LogicModelDOTExporter exports the logic model or a part
 * of the logic model as a dot graph.
 */

class LogicModelDOTExporter : public DOTExporter {

protected:

    void add_gate(Gate_shptr gate);
    void add_via(Via_shptr via);
    //void add_wire(Wire_shptr wire);
    void add_net(Net_shptr lmodel);

    void add_connection(Net_shptr net, std::string const& src_name, std::string const& edge_name);

    std::string oid_to_str(std::string const& prefix, object_id_t oid);


private:

    struct GateConn {
        GateConn(
            const object_id_t _gate_id
            , const std::string& _connection_name
        ) :
            gate_id(_gate_id)
            , connection_name(_connection_name)
        {}

        object_id_t gate_id;
        std::string connection_name;
    };

    std::map<object_id_t /* net id */, int> implicit_net_counter;

    ObjectIDRewriter_shptr oid_rewriter;

    //VIA <--> gates
    std::map<object_id_t, std::vector<GateConn> > via_to_gates;

public:

    LogicModelDOTExporter(ObjectIDRewriter_shptr _oid_rewriter) :
        oid_rewriter(_oid_rewriter)
    {
    }

    ~LogicModelDOTExporter() {}

    /**
     * Export the logic model as DOT file.
     * @excpetion InvalidPathException
     * @excpetion InvalidPointerException
     * @excpetion std::runtime_error
     */
    void export_data(std::string const& filename, LogicModel_shptr lmodel);

};

}

#endif
