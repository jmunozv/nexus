// ----------------------------------------------------------------------------
// nexus | OpticalTestGeometry.h
//
// Detector geometry for optical setting tests.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef OPT_TEST_H
#define OPT_TEST_H

#include "BaseGeometry.h"

#include <G4ThreeVector.hh>

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;
class G4GenericMessenger;


namespace nexus {

  // This is a test geometry for optical setiing tests 

  class OpticalTestGeometry : public BaseGeometry
  {

  public:
    // Constructor
    OpticalTestGeometry();

    // Destructor
    ~OpticalTestGeometry();

    // Builder
    void Construct();

    // Generate vertices within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;


  private:

    // Defines Configuration parameters
    void DefineConfigurationParameters();

    // Defines materials needed
    void DefineMaterials();

    // Different builders
    void BuildSetup  (G4LogicalVolume* mother_logic);
    void BuildSensors(G4LogicalVolume* mother_logic);

  private:

    // Verbosity of the geometry
    G4bool verbosity_;

    // Visibility of the geometry
    G4bool setup_visibility_;
    G4bool sensors_visibility_;

    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

    // Setup configuration & dimensions
    G4double  setup_radius_;    
    G4double  sensor_radius_;
    G4double  setup_ini_z_;

    // Materials
    G4String    gas_name_;
    G4double    gas_pressure_, gas_temperature_;
    G4Material* xenon_gas_;
    G4Material* tpb_mat_;
    G4Material* teflon_mat_;
    G4Material* ito_mat_;
    G4Material* quartz_mat_;

    // AD-HOC vertex
    G4double adhoc_x_, adhoc_y_, adhoc_z_;

    // Relevant PhysicalVolume
    G4VPhysicalVolume* gas_phys_;
  };

} // end namespace nexus

#endif
