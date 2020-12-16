#pragma once

#include <array>
#include <vector>

#include "Definitions.h"
#include "Functions.h"


namespace SIM {

	enum DiseaseState { SUSCEPTIBLE, INCUBANT, SYMPTOMATIC, /*QUARANTINED,*/ RECOVERED, DEAD };

	struct DemoSpec {
		uint population;
	};

	struct DiseaseSpec {
		uint infection_rate;
		uint manifestation_rate;
		uint recovery_rate;
		uint death_rate;
		bool acquires_immunity;
	};

	struct SocioSpec {
		uint motion_rate;
		uint teleport_rate;
		uint discovery_rate;
		uint patient_capacity;
	};


	// A struct that stores the specifications for simulation to be passed to the simulation function.
	// All probability variables represent the probability per tick or frame.
	struct Specification {

		DemoSpec demography;
		DiseaseSpec disease;
		SocioSpec society;

		/*
		// Demographic specifications.
		uint population;



		// Disease specifications.
		uint infection_rate;
		uint manifestation_rate;
		uint recovery_rate;
		uint death_rate;
		bool acquires_immunity;

		// Policy specifications.


		//unsigned int prob_mortality;
		//unsigned int min_dur_incubation;
		//unsigned int max_dur_incubation;
		//unsigned int dur_treatment;

		// Policy specifications.
		uint motion_rate;
		uint teleport_rate;
		uint discovery_rate;
		uint patient_capacity;
		*/

	};






	// A struct that stores status of various infection control policies.
	struct Policy {

		// Reduces probability of movement by 90%.
		bool enables_immobilization = false;
		// Disables all teleport.
		bool enables_teleport_ban = false;

	};



	struct Person {

		uint tile_index;
		DiseaseState state;
		bool is_hospitalized;
		//int tick_to_symptom;

	};



	// Defines a zone whose borders are impermeable to any person attempting to cross it.
	// It is not a true zone, but rather a set of boundaries
	// whose permeability depends on its effectiveness.
	// For computation, column1 must be <= column2, row1 must be <= row2.
	struct QuarantineZone {

		// Row and column numbers should be half integral numbers.
		float half_column1, half_row1, half_column2, half_row2;
		// Represents the percentage probability that a person cannot traverse its borders.
		uint effectiveness;

	};

	// Specifies the end point of simulation.
	// Interrupted: Simulation is interrupted by the user.
	// Overwhelmed: All population is dead.
	// Eradicated: All infected cases are cleared.
	enum EndType { INTERRUPTED, OVERWHELMED, ERADICATED, EQUILIBRATED };

	struct Result {

		EndType end_point = INTERRUPTED;
		// Index represents number of ticks.
		std::vector<uint> survival_counts;
		std::vector<uint> infection_counts;
		std::vector<uint> death_counts;
		std::vector<uint> recovery_counts;
	};


	inline bool IsVicinal(Person p1, Person p2, uint column_count, uint distance = 1) {
		int p1_column = GetColumn(p1.tile_index, column_count);
		int p1_row = GetRow(p1.tile_index, column_count);
		int p2_column = GetColumn(p2.tile_index, column_count);
		int p2_row = GetRow(p2.tile_index, column_count);
		return abs(p1_column - p2_column) <= distance && abs(p1_row - p2_row) <= distance;
	}

	inline bool Intersects(float column, float row, QuarantineZone zone) {
		return IsBetween(column, zone.half_column1, zone.half_column2) &&
			IsBetween(row, zone.half_row1, zone.half_row2);
	}

}


// Some sample specifications for trial purposes.
SIM::DiseaseSpec GetDiseaseSpec(const char *identifier) {

	SIM::DiseaseSpec disease_spec;
	
	// Pestilence.
	if (strcmp(identifier, "1") == 0) {

		disease_spec.infection_rate = 80;
		disease_spec.manifestation_rate = 10;
		disease_spec.death_rate = 5;
		disease_spec.recovery_rate = 2;
		disease_spec.acquires_immunity = false;

	// Smallpox.
	} else if (strcmp(identifier, "2") == 0) {

		disease_spec.infection_rate = 100;
		disease_spec.manifestation_rate = 5;
		disease_spec.death_rate = 2;
		disease_spec.recovery_rate = 4;
		disease_spec.acquires_immunity = true;

	// Measles.
	} else if (strcmp(identifier, "3") == 0) {

		disease_spec.infection_rate = 100;
		disease_spec.manifestation_rate = 8;
		disease_spec.death_rate = 1;
		disease_spec.recovery_rate = 10;
		disease_spec.acquires_immunity = false;

	// Covid.
	} else if (strcmp(identifier, "4") == 0) {

		disease_spec.infection_rate = 40;
		disease_spec.manifestation_rate = 2;
		disease_spec.death_rate = 1;
		disease_spec.recovery_rate = 7;
		disease_spec.acquires_immunity = false;

	} else {

		// Returns a zero disease_spec if there is no match for the disease query.
		disease_spec.infection_rate = 0;
		disease_spec.manifestation_rate = 0;
		disease_spec.death_rate = 0;
		disease_spec.recovery_rate = 0;
		disease_spec.acquires_immunity = false;

	}

	return disease_spec;

}