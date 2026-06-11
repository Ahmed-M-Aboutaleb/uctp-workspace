#ifndef MATCHING_HPP
#define MATCHING_HPP

#include "Individual.hpp"
#include "Problem.hpp"

// Bipartite max-cardinality matching between events currently scheduled in a
// timeslot and the suitable rooms for those events (Event-Room matrix G).
//
// This is the matching algorithm referenced in Rossi-Doria's UCTP work and
// used by the interchange and simple-mutation operators "to increase their
// rate of success".  After a structural move the room assignments inside an
// affected timeslot may break H2/H3; calling repairSlot reassigns rooms via
// max matching so that as many events as possible are placed legally.
namespace Matching {

// Reassign the rooms of all events currently in `slot` so that:
//   - each event sits in a room that is suitable for it (H2)
//   - no two events share a room in `slot`               (H3)
// Events for which no compatible room is available become unassigned.
// H1 is unaffected because no event leaves the timeslot.
void repairSlot(Individual& ind, const Problem& p, int slot);

}  // namespace Matching

#endif
