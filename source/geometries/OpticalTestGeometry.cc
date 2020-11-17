// ----------------------------------------------------------------------------
// nexus | OpticalTestGeometry.h
//
// Detector geometry for optical setting tests.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "OpticalTestGeometry.h"

#include "MaterialsList.h"
#include "OpticalMaterialProperties.h"
#include "XenonGasProperties.h"
#include "Visibilities.h"
#include "PmtSD.h"

#include <G4GenericMessenger.hh>
#include <G4LogicalVolume.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4OpticalSurface.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4Material.hh>
#include <G4VisAttributes.hh>
#include <G4SDManager.hh>
#include <G4RunManager.hh>
#include <G4UnitsTable.hh>


using namespace nexus;


OpticalTestGeometry::OpticalTestGeometry():
  BaseGeometry(),
  verbosity_          (false),
  setup_visibility_   (false),
  sensors_visibility_ (false),
  msg_                (nullptr),
  setup_radius_       (10. * cm),
  sensor_radius_      (10. * cm),
  gas_name_           ("enrichedXe"),
  gas_pressure_       (15.  * bar),
  gas_temperature_    (300. * kelvin),
  adhoc_x_            (0.),             // Vertex-X in case of AD_HOC region
  adhoc_y_            (0.),             // Vertex-Y in case of AD_HOC region
  adhoc_z_            (0.)              // Vertex-Z in case of AD_HOC region
{

  // Messenger
  msg_ = new G4GenericMessenger(this, "/Geometry/OpticalTest/",
                                "Control commands of the OpticalTest geometry.");

  // Parametrized dimensions
  DefineConfigurationParameters();
}



OpticalTestGeometry::~OpticalTestGeometry()
{
  delete msg_;
}



void OpticalTestGeometry::DefineConfigurationParameters()
{
  // Verbosity
  msg_->DeclareProperty("verbosity", verbosity_, "Verbosity");

  // Visibilities
  msg_->DeclareProperty("setup_vis",   setup_visibility_,   "Setup visibility");
  msg_->DeclareProperty("sensors_vis", sensors_visibility_, "Sensors visibility");

  // Setup radius
  G4GenericMessenger::Command& setup_radius_cmd =
    msg_->DeclareProperty("setup_radius", setup_radius_,
                          "Setup radius.");
  setup_radius_cmd.SetUnitCategory("Length");
  setup_radius_cmd.SetParameterName("setup_radius", false);
  setup_radius_cmd.SetRange("setup_radius>0.");

  // Sensor radius
  G4GenericMessenger::Command& sensor_radius_cmd =
    msg_->DeclareProperty("sensor_radius", sensor_radius_,
                          "Sensor radius.");
  sensor_radius_cmd.SetUnitCategory("Length");
  sensor_radius_cmd.SetParameterName("sensor_radius", false);
  sensor_radius_cmd.SetRange("sensor_radius>0.");

  // Gas properties
  msg_->DeclareProperty("gas", gas_name_, "Xenon gas type.");

  G4GenericMessenger::Command& gas_pressure_cmd =
    msg_->DeclareProperty("gas_pressure", gas_pressure_,
                          "Pressure of the xenon gas.");
  gas_pressure_cmd.SetUnitCategory("Pressure");
  gas_pressure_cmd.SetParameterName("gas_pressure", false);
  gas_pressure_cmd.SetRange("gas_pressure>0.");

  G4GenericMessenger::Command& gas_temperature_cmd =
    msg_->DeclareProperty("gas_temperature", gas_temperature_,
                          "Temperature of the xenon gas.");
  gas_temperature_cmd.SetUnitCategory("Temperature");
  gas_temperature_cmd.SetParameterName("gas_temperature", false);
  gas_temperature_cmd.SetRange("gas_temperature>0.");

  // Specific vertex in case region to shoot from is AD_HOC
  G4GenericMessenger::Command& adhoc_x_cmd =
    msg_->DeclareProperty("specific_vertex_X", adhoc_x_,
      "If region is AD_HOC, x coord where particles are generated");
  adhoc_x_cmd.SetParameterName("specific_vertex_X", false);
  adhoc_x_cmd.SetUnitCategory("Length");

  G4GenericMessenger::Command& adhoc_y_cmd =
    msg_->DeclareProperty("specific_vertex_Y", adhoc_y_,
      "If region is AD_HOC, y coord where particles are generated");
  adhoc_y_cmd.SetParameterName("specific_vertex_Y", false);
  adhoc_y_cmd.SetUnitCategory("Length");

  G4GenericMessenger::Command& adhoc_z_cmd =
    msg_->DeclareProperty("specific_vertex_Z", adhoc_z_,
      "If region is AD_HOC, z coord where particles are generated");
  adhoc_z_cmd.SetParameterName("specific_vertex_Z", false);
  adhoc_z_cmd.SetUnitCategory("Length");
}



void OpticalTestGeometry::DefineMaterials()
{
  // Defining the gas xenon
  if (gas_name_ == "naturalXe")
    xenon_gas_ = MaterialsList::GXe(gas_pressure_, gas_temperature_);

  else if (gas_name_ == "enrichedXe")
    xenon_gas_ = MaterialsList::GXeEnriched(gas_pressure_, gas_temperature_);

  else if (gas_name_ == "depletedXe")
    xenon_gas_ = MaterialsList::GXeDepleted(gas_pressure_, gas_temperature_);

  else
    G4Exception("[NextParam]", "DefineMaterials()", FatalException,
    "Unknown xenon gas type. Valid options are naturalXe, enrichedXe or depletedXe.");

  xenon_gas_->SetMaterialPropertiesTable(OpticalMaterialProperties::GXe(gas_pressure_,
                                                                        gas_temperature_,
                                                                        25510 * (1./MeV)));

  // Defining wls TPB
  tpb_mat_ = MaterialsList::TPB();
  tpb_mat_->SetMaterialPropertiesTable(OpticalMaterialProperties::TPB());

  // Derfining the TEFLON
  teflon_mat_ = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");

  // Defining ITO coating
  ito_mat_ = MaterialsList::ITO();
  ito_mat_->SetMaterialPropertiesTable(OpticalMaterialProperties::ITO());

  // Defining Quartz
  quartz_mat_ = MaterialsList::FusedSilica();
  quartz_mat_->SetMaterialPropertiesTable(OpticalMaterialProperties::FusedSilica());
  //quartz_mat_->SetMaterialPropertiesTable(OpticalMaterialProperties::FusedSilica90());
}



void OpticalTestGeometry::Construct()
{
  // Verbosity
  if(verbosity_) {
    G4cout << G4endl << "***** Verbosing OPTICAL_TEST geometry *****" << G4endl;
  }

  // Define materials.
  DefineMaterials();


  // The lab
  G4String lab_name = "LABORATORY";
  G4double lab_size = 50. * m;

  G4Box* lab_solid = new G4Box(lab_name, lab_size/2.,
                               lab_size/2., lab_size/2.);

  G4LogicalVolume* lab_logic = new G4LogicalVolume(lab_solid,
                                                   xenon_gas_, lab_name);

  BaseGeometry::SetLogicalVolume(lab_logic);

  lab_logic->SetVisAttributes(G4VisAttributes::Invisible);


  // The gas
  G4String gas_name = "GAS";
  G4double gas_size = 40. * m;

  G4Box* gas_solid = new G4Box(gas_name, gas_size/2.,
                               gas_size/2., gas_size/2.);

  G4LogicalVolume* gas_logic = new G4LogicalVolume(gas_solid, xenon_gas_, gas_name);
  gas_logic->SetVisAttributes(G4VisAttributes::Invisible);

  gas_phys_ = new G4PVPlacement(nullptr, G4ThreeVector(0.,0.,0.), gas_logic,
                                gas_name, lab_logic, false, 0, true);

  // Build the setup to study
  BuildSetup(gas_logic);

  // Build the detectors
  BuildSensors(gas_logic);

  // Verbosity
  if(verbosity_) G4cout << G4endl;
}



void OpticalTestGeometry::BuildSetup(G4LogicalVolume* mother_logic) {

  /// Verbosity
  if(verbosity_) {
    G4cout << G4endl << "*** OpticalTest SETUP ***" << G4endl;
  }

  /// Building layers ... ///

  // Layers starting at this z
  setup_ini_z_         = 0.;
  G4int    num_layers  = 0;
  G4String layer_name;
  G4double layer_thickn;
  G4double layer_rad;
  G4double layer_pos_z;

  G4Tubs*            layer_solid;
  G4LogicalVolume*   layer_logic;
  G4VPhysicalVolume* layer_phys;
  G4OpticalSurface*  opt_surf;
  G4VisAttributes    layer_color;


  /// Layer TPB
  num_layers   += 1;
  layer_name    = "TPB";
  layer_thickn  = 1.  * um;
  layer_rad     = setup_radius_;
  layer_pos_z   = setup_ini_z_ - layer_thickn/2.;
  setup_ini_z_ -= layer_thickn;
  layer_color   = nexus::LightBlue();

  if (verbosity_)
    G4cout << "*** Layer " << num_layers-1 << ": " << layer_name << G4endl <<
              "    Rad: "  << layer_rad    << "  Zs from " << setup_ini_z_  <<
              " to "       << setup_ini_z_ + layer_thickn << G4endl;

  layer_solid = new G4Tubs(layer_name, 0., layer_rad, layer_thickn/2., 0, twopi);

  layer_logic = new G4LogicalVolume(layer_solid, tpb_mat_, layer_name);

  layer_phys  = new G4PVPlacement(nullptr, G4ThreeVector(0., 0., layer_pos_z), layer_logic,
                                  layer_name, mother_logic, false, 0, verbosity_);

  opt_surf = new G4OpticalSurface(layer_name, glisur, ground, dielectric_dielectric, 0.01);
  new G4LogicalBorderSurface(layer_name + "OPSURF", layer_phys, gas_phys_, opt_surf);
  new G4LogicalBorderSurface(layer_name + "OPSURF", gas_phys_, layer_phys, opt_surf);

  if (setup_visibility_) layer_logic->SetVisAttributes(layer_color);
  else                   layer_logic->SetVisAttributes(G4VisAttributes::Invisible);



  /// Layer TEFLON
  num_layers   += 1;
  layer_name    = "TEFLON";
  layer_thickn  = 5. * mm;
  layer_rad     = setup_radius_;
  layer_pos_z   = setup_ini_z_ - layer_thickn/2.;
  setup_ini_z_ -= layer_thickn;
  layer_color   = nexus::Blue();

  if (verbosity_)
    G4cout << "*** Layer " << num_layers-1 << ": " << layer_name << G4endl <<
              "    Rad: "  << layer_rad    << "  Zs from " << setup_ini_z_  <<
              " to "       << setup_ini_z_ + layer_thickn << G4endl;

  layer_solid = new G4Tubs(layer_name, 0., layer_rad, layer_thickn/2., 0, twopi);

  layer_logic = new G4LogicalVolume(layer_solid, teflon_mat_, layer_name);

  layer_phys  = new G4PVPlacement(nullptr, G4ThreeVector(0., 0., layer_pos_z), layer_logic,
                                  layer_name, mother_logic, false, 0, verbosity_);

  opt_surf = new G4OpticalSurface(layer_name + "OPSURF", unified, ground, dielectric_metal);
  opt_surf->SetMaterialPropertiesTable(OpticalMaterialProperties::PTFE());
  new G4LogicalSkinSurface(layer_name + "OPSURF", layer_logic, opt_surf);

  if (setup_visibility_) layer_logic->SetVisAttributes(layer_color);
  else                   layer_logic->SetVisAttributes(G4VisAttributes::Invisible);
}



void OpticalTestGeometry::BuildSensors(G4LogicalVolume* mother_logic) {

  /// Verbosity
  if(verbosity_) {
    G4cout << G4endl << "*** OpticalTest SENSORS ***" << G4endl;
  }

  /// Building the sensors
  G4String sensor_name       = "SENSOR";
  G4String front_sensor_name = "FRONT_SENSOR";
  G4String rear_sensor_name  = "REAR_SENSOR";
  G4String lat_sensor_name   = "LATERAL_SENSOR";

  G4Material* sensor_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Si");

  // Front & Rear
  G4double sensor_thickness  =   1. * mm;
  G4double front_sensor_iniZ =  10. * mm;
  //G4double rear_sensor_iniZ  = -10. * mm;
  G4double rear_sensor_iniZ  = setup_ini_z_ - sensor_thickness; // Placement sticked to the setup

  G4double front_sensor_posZ = front_sensor_iniZ + sensor_thickness/2.;
  G4double rear_sensor_posZ  = rear_sensor_iniZ  + sensor_thickness/2.;

  G4Tubs* sensor_solid =
    new G4Tubs(sensor_name, 0., sensor_radius_, sensor_thickness/2., 0., twopi);

  G4LogicalVolume* front_sensor_logic =
    new G4LogicalVolume(sensor_solid, sensor_mat, front_sensor_name);

  G4LogicalVolume* rear_sensor_logic =
    new G4LogicalVolume(sensor_solid, sensor_mat, rear_sensor_name);

  // Lateral
  G4double lat_sensor_irad   = setup_radius_;
  G4double lat_sensor_orad   = lat_sensor_irad + sensor_thickness;
  G4double lat_sensor_length = front_sensor_iniZ - rear_sensor_iniZ - sensor_thickness;
  G4double lat_sensor_posZ   = front_sensor_iniZ - lat_sensor_length/2.;

  // Verbosity
  if (verbosity_)
    G4cout << "*** FRONT & REAR sensors ... "  << G4endl <<
              "    radius: " << sensor_radius_ << G4endl <<
              "    FRONT Zs from "  << front_sensor_iniZ << " to " <<
              front_sensor_iniZ + sensor_thickness << G4endl <<
              "    REAR  Zs from "  << rear_sensor_iniZ  << " to " <<
              rear_sensor_iniZ  + sensor_thickness << G4endl <<
              "*** LATERAL sensor ... "  << G4endl <<
              "    iRad: "  << lat_sensor_irad << "  oRad: " << lat_sensor_orad << G4endl;

  // Building the sensors
  G4Tubs* lat_sensor_solid =
    new G4Tubs(lat_sensor_name, lat_sensor_irad, lat_sensor_orad,
               lat_sensor_length/2., 0., twopi);

  G4LogicalVolume* lat_sensor_logic =
    new G4LogicalVolume(lat_sensor_solid, sensor_mat, lat_sensor_name);


  /// Optical properties
  G4MaterialPropertiesTable* sensor_opt_prop = new G4MaterialPropertiesTable();

  const G4int entries = 2;
  G4double energies[entries] = {0.20 * eV, 12.4 * eV};

  // It does not reflect anything
  G4double sensor_reflectivity[entries] = {0., 0.};
  sensor_opt_prop->AddProperty("REFLECTIVITY", energies,
                              sensor_reflectivity, entries);

  // It detects everything
  G4double sensor_efficiency[entries] = {1., 1.};
  sensor_opt_prop->AddProperty("EFFICIENCY", energies,
                              sensor_efficiency, entries);

  // Building the optical surface
  G4OpticalSurface* sensor_opt_surf = 
    new G4OpticalSurface(sensor_name, unified, polished, dielectric_metal);
  sensor_opt_surf->SetMaterialPropertiesTable(sensor_opt_prop);

  // Giving the optical surface to sensors
  new G4LogicalSkinSurface(sensor_name, front_sensor_logic, sensor_opt_surf);
  new G4LogicalSkinSurface(sensor_name, rear_sensor_logic,  sensor_opt_surf);
  new G4LogicalSkinSurface(sensor_name, lat_sensor_logic,   sensor_opt_surf);


  /// Sensitive detector 
  G4SDManager* sdmgr = G4SDManager::GetSDMpointer();
  if (!sdmgr->FindSensitiveDetector(sensor_name, false)) {
    PmtSD* sensor_sd = new PmtSD(sensor_name);
    sensor_sd->SetDetectorVolumeDepth(0);
    sensor_sd->SetMotherVolumeDepth(0);
    sensor_sd->SetTimeBinning(1. * us);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensor_sd);
    front_sensor_logic->SetSensitiveDetector(sensor_sd);
    rear_sensor_logic ->SetSensitiveDetector(sensor_sd);
    lat_sensor_logic  ->SetSensitiveDetector(sensor_sd);
  }

  // Placing the sensors
  new G4PVPlacement(nullptr, G4ThreeVector(0., 0., front_sensor_posZ), front_sensor_logic,
                    front_sensor_name, mother_logic, false, 0, true);

  new G4PVPlacement(nullptr, G4ThreeVector(0., 0., rear_sensor_posZ), rear_sensor_logic,
                    rear_sensor_name, mother_logic, false, 0, true);

  new G4PVPlacement(nullptr, G4ThreeVector(0., 0., lat_sensor_posZ), lat_sensor_logic,
                    lat_sensor_name, mother_logic, false, 0, true);


  // Visibilities
  if (sensors_visibility_) {
    front_sensor_logic->SetVisAttributes(nexus::BloodRed());
    rear_sensor_logic ->SetVisAttributes(nexus::BloodRed());
    lat_sensor_logic  ->SetVisAttributes(nexus::BloodRed());
  }
  else {
    front_sensor_logic->SetVisAttributes(G4VisAttributes::Invisible);
    rear_sensor_logic ->SetVisAttributes(G4VisAttributes::Invisible);
    lat_sensor_logic  ->SetVisAttributes(G4VisAttributes::Invisible);
  }
}



G4ThreeVector OpticalTestGeometry::GenerateVertex(const G4String& region) const
{
  G4ThreeVector vertex;

  if (region == "AD_HOC") {
    vertex = G4ThreeVector(adhoc_x_, adhoc_y_, adhoc_z_);
  }

  else {
    G4Exception("[OpticalTestGeometry]", "GenerateVertex()", FatalException,
      "Unknown vertex generation region!");
  }

  return vertex;
}
