// ----------------------------------------------------------------------------
// nexus | DefaultRunAction.h
//
// This is the default run action of the NEXT simulations.
// A message at the beginning and at the end of the simulation is printed.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef __DEFAULT_RUN_ACTION__
#define __DEFAULT_RUN_ACTION__

#include <G4UserRunAction.hh>


namespace nexus {

  /// TODO. CLASS DESCRIPTION

  class DefaultRunAction: public G4UserRunAction
  {
  public:
    /// Constructor
    DefaultRunAction();
    /// Destructor
    ~DefaultRunAction();

    virtual void BeginOfRunAction(const G4Run*);
    virtual void EndOfRunAction(const G4Run*);
  };

}

#endif
