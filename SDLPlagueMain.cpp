/*
TO DO LIST:
# Finalize console interface.
# Provide a few options for policy.
# Vet documentation text.
*/
#include <SDL.h>
#ifdef main
#undef main
#endif

#include <conio.h>
#include <algorithm>
#include <array>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <ctime>

#include "Constants.h"
#include "Classes.h"
#include "Definitions.h"
#include "Functions.h"


using namespace std;

SIM::Result Simulate(SIM::Specification);
int DrawResult(SIM::Result);



int main() {

	// Receives specifications for simulator from users.
	SIM::Specification spec;

	char c;

	char input[100] = { 0 };
	printf("Enter a population between %d and %d: ", MIN_POPULATION, MAX_POPULATION);
	scanf("%s", input);
	spec.demography.population = Truncate((uint)atoi(input), MIN_POPULATION, MAX_POPULATION);
	while (true) {
		printf("Input the number of the model and press enter:\n");
		printf("< Model 1 >: Infectivity = 80. Mortality = 5.\n");
		printf("< Model 2 >: Infectivity = 100. Mortality = 2. Has acquired immunity.\n");
		printf("< Model 3 >: Infectivity = 100. Mortality = 1.\n");
		printf("< Model 4 >: Infectivity = 40. Mortality = 1.\n");
		scanf("%s", input);
		// Defines percent probability of spread of disease.
		spec.disease = GetDiseaseSpec(input);
		if (spec.disease.infection_rate != 0) {
			break;
		} else {
			printf("Invalid disease query. Please retry.\n");
		}
	}
	spec.society.motion_rate = DEFAULT_MOTION_RATE;
	spec.society.teleport_rate = DEFAULT_TELEPORT_RATE;
	spec.society.discovery_rate = DEFAULT_DISCOVERY_RATE;
	spec.society.patient_capacity = DEFAULT_PATIENT_CAPACITY;

	// Commences simulation and returns a struct of results.
	SIM::Result result = Simulate(spec);

	

	// Saves simulation result.
	if (result.end_point == SIM::INTERRUPTED) {
		printf("Simulation is interrupted prematurely.\n");
	} else if (result.end_point == SIM::OVERWHELMED) {
		printf("Your population has been overwhelmed.\n");
	} else if (result.end_point == SIM::ERADICATED) {
		printf("The disease has been eradicated.\n");
	} else if (result.end_point == SIM::EQUILIBRATED) {
		printf("Disease transmission likely reaches an equilibrium.\n");
	}

	// Displays plot for simulation result.
	printf("Press any key to display the plot.\n");
	_getch();
	
	DrawResult(result);

	// Writes result into a temp file for further analysis.
	printf("Pipe result to a separate text file? 1: Yes. Others: No.\n");
	c = _getch();

	if (c == '1') {
		FILE *p_result_file = fopen("result.txt", "w");
		fprintf(p_result_file, "Copy this result for further evaluation.\n");
		fprintf(p_result_file, "-- Result --\n");
		fprintf(p_result_file, "Tick\tSick\tCured\tDead\n");
		for (size_t i = 0; i < result.infection_counts.size(); i++) {
			fprintf(p_result_file, "%d\t%d\t%d\t%d\n",
				i, result.infection_counts[i], result.recovery_counts[i], result.death_counts[i]);
		}
		fclose(p_result_file);
		system("start notepad result.txt");
	}

	printf("Press any key to exit the application ...\n");
	_getch();

	return 0;

}



// Uses an SDL window to display simulation for the given population and infectivity.
SIM::Result Simulate(SIM::Specification spec) {

	// Retrieves simulation specifications and initializes policy settings.
	uint population = spec.demography.population;

	uint infection_rate = spec.disease.infection_rate;
	uint manifestation_rate = spec.disease.manifestation_rate;
	uint recovery_rate = spec.disease.recovery_rate;
	uint death_rate = spec.disease.death_rate;
	bool acquires_immunity = spec.disease.acquires_immunity;

	uint motion_rate = spec.society.motion_rate;
	uint teleport_rate = spec.society.teleport_rate;
	uint discovery_rate = spec.society.discovery_rate;
	uint patient_capacity = spec.society.patient_capacity;

	SIM::Policy policy;
	policy.enables_immobilization = false;
	policy.enables_teleport_ban = false;

	// Initializes SDL.
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Event event;

	// Initializes window and renderer.
	// Displays the main window. Sets default draw color of renderer.
	//const int WIN_W = POINT_SPACE * (ROW_COUNT + 1);
	//const int WIN_H = POINT_SPACE * (COLUMN_COUNT + 1);
	SDL_Window *p_window;
	p_window = SDL_CreateWindow(WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_SHOWN);

	// Gets relevant surfaces and rectangles.
	SDL_Surface *p_window_surface = SDL_GetWindowSurface(p_window);
	SDL_Surface *p_susceptible_surface = SDL_LoadBMP(PERSON_SUSCEPTIBLE_BMP);
	SDL_Surface *p_incubant_surface = SDL_LoadBMP(PERSON_INCUBANT_BMP);
	SDL_Surface *p_symptomatic_surface = SDL_LoadBMP(PERSON_SYMPTOMATIC_BMP);
	SDL_Surface *p_quarantined_surface = SDL_LoadBMP(PERSON_QUARANTINED_BMP);
	SDL_Surface *p_hospitalized_surface = SDL_LoadBMP(PERSON_HOSPITALIZED_BMP);
	SDL_Surface *p_dead_surface = SDL_LoadBMP(PERSON_DEAD_BMP);
	SDL_Surface *p_recovered_surface = SDL_LoadBMP(PERSON_RECOVERED_BMP);
	
	SDL_Surface *p_quarantine_border_surface = SDL_LoadBMP(QUARANTINE_BORDER_BMP);
	
	//const int PERSON_W = p_susceptible_surface->w;
	//const int PERSON_H = p_susceptible_surface->h;
	SDL_Rect window_rect;
	window_rect.x = 0;
	window_rect.y = 0;
	window_rect.w = WIN_W;
	window_rect.h = WIN_H;

	// Prepares storage for relevant data.
	// Initially, people are randomly scattered on the map.
	// Randomly generates indices for persons.
	int* tile_indices = new int[population] { -1 };
	for (size_t i = 0; i < population; i++) {
		while (true) {
			uint tile_index = rand() % TILE_COUNT;
			bool has_duplicate = false;
			for (size_t i2 = 0; i2 < population; i2++) {
				if (tile_indices[i2] == tile_index) {
					has_duplicate = true;
					break;
				}
			}
			if (!has_duplicate) {
				tile_indices[i] = tile_index;
				break;
			}
		}
	}
	SIM::Person* persons = new SIM::Person[population];
	for (size_t i = 0; i < population; i++) {
		persons[i].tile_index = tile_indices[i];
		persons[i].state = SIM::SUSCEPTIBLE;
		persons[i].is_hospitalized = false;
	}
	// Randomly assigns a Case 0.
	uint case_0_index = rand() % population;
	persons[case_0_index].state = SIM::INCUBANT;

	// Stores a collection of quarantine zones and prepares a temporary storage for mouse drags.
	vector<SIM::QuarantineZone> quarantine_zones;
	SIM::QuarantineZone temp_quarantine_zone;
	bool left_button_is_down = false;
	float mouse_down_half_column = 0.0, mouse_down_half_row = 0.0;

	// Counts the progress of simulation.
	//int msec_elapsed = 0;
	//int msec_fraction_elapsed = 0;
	uint infection_count = 1; // Case 0 is there.
	uint death_count = 0;
	uint recovery_count = 0;
	SIM::Result result;
	uint tick_count = 0;

	// Establishes simulation controls.
	bool shows_incubant = false;

	// Creates a loop that executes once per frame.
	// At each frame, events are processed, data are updated and persons are displayed.
	// Updates location status.
	// A person randomly chooses to move or not to move, as well as which direction to move.
	// Simulation ends when the whole population is infected.
	bool runs_simulation_loop = true;
	while (runs_simulation_loop) {

		// Obtains time at the start of loop.
		clock_t t_msec = clock();

		// Processes user events.
		SDL_PollEvent(&event);

		// Exits loop for SDL_QUIT.
		if (event.type == SDL_QUIT) {

			result.end_point = SIM::INTERRUPTED;
			runs_simulation_loop = false;

		// Defines quarantine zones when left mouse buttons clicks down and up.
		} else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {

			// Records the 1st pair of coordinates.
			left_button_is_down = true;
			mouse_down_half_column = event.button.x / POINT_SPACE - 0.5;
			mouse_down_half_row = event.button.y / POINT_SPACE - 0.5;

		} else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {

			// Records the 2nd pair of coordinates, reshuffles order and appends the new zone.
			left_button_is_down = false;
			float mouse_up_half_column = event.button.x / POINT_SPACE - 0.5;
			float mouse_up_half_row = event.button.y / POINT_SPACE - 0.5;
			temp_quarantine_zone.half_column1 = min(mouse_down_half_column, mouse_up_half_column);
			temp_quarantine_zone.half_row1 = min(mouse_down_half_row, mouse_up_half_row);
			temp_quarantine_zone.half_column2 = max(mouse_down_half_column, mouse_up_half_column);
			temp_quarantine_zone.half_row2 = max(mouse_down_half_row, mouse_up_half_row);
			quarantine_zones.push_back(temp_quarantine_zone);

		// Toggles displaying of incubant cases when left mouse button clicks down and up.
		} else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {

			shows_incubant = true;

		} else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {

			shows_incubant = false;

		// Processes key presses.
		} else if (event.type == SDL_KEYDOWN) {

			// Key press toggles status of the associated policy.
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_M:
				policy.enables_immobilization = !policy.enables_immobilization;
				motion_rate = policy.enables_immobilization ?
					spec.society.motion_rate / 10 : spec.society.motion_rate;
				break;
			case SDL_SCANCODE_T:
				policy.enables_teleport_ban = !policy.enables_teleport_ban;
				teleport_rate = policy.enables_teleport_ban ? 0 : spec.society.teleport_rate;
			}

		}

		// Updates movement for each person.
		for (size_t i = 0; i < population; i++) {
			// Determines if the person moves. If so, determines whether the person teleports.
			
			if (rand() % 100 < motion_rate) {

				uint old_tile_index = persons[i].tile_index;
				uint old_column = GetColumn(old_tile_index, COLUMN_COUNT);
				uint old_row = GetRow(old_tile_index, COLUMN_COUNT);

				// TO DOUBLE CHECK THIS SEGMENT.
				if (rand() % 100 < teleport_rate) {
					// If teleport, keeps generating a new index until a satisfactory one.
					uint new_teleport_index;
					bool is_valid_index = false;
					while (!is_valid_index) {
						new_teleport_index = rand() % TILE_COUNT;
						is_valid_index = true;
						// Makes the new index invalid if it clashes with another person.
						for (size_t i = 0; i < population; i++) {
							if (persons[i].tile_index == new_teleport_index) {
								is_valid_index = false;
								break;
							}
						}
						// Makes the new index invalid if it traverses any quarantine zone.
						uint new_column = GetColumn(new_teleport_index, COLUMN_COUNT);
						uint new_row = GetRow(new_teleport_index, COLUMN_COUNT);
						for (SIM::QuarantineZone zone : quarantine_zones) {
							if ((Intersects(old_column, old_row, zone) &&
								!Intersects(new_column, new_row, zone)) ||
								(!Intersects(old_column, old_row, zone) &&
								Intersects(new_column, new_row, zone))) {
								is_valid_index = false;
								break;
							}
						}
					}
					persons[i].tile_index = new_teleport_index;

				} else {

					// If no teleport, determines movement direction if he moves.
					// Generates all possible new indices for movement that do not overflow.
					vector<uint> new_move_indices;
					if (ALLOWS_DIAGONAL_MOVE) {
						if (old_column > 0 && old_row > 0) {
							new_move_indices.push_back(persons[i].tile_index - 1 - COLUMN_COUNT);
						}
						if (old_column < COLUMN_COUNT - 1 && old_row > 0) {
							new_move_indices.push_back(persons[i].tile_index + 1 - COLUMN_COUNT);
						}
						if (old_column < COLUMN_COUNT - 1 && old_row < ROW_COUNT - 1) {
							new_move_indices.push_back(persons[i].tile_index + 1 + COLUMN_COUNT);
						}
						if (old_column > 0 && old_row < ROW_COUNT - 1) {
							new_move_indices.push_back(persons[i].tile_index - 1 + COLUMN_COUNT);
						}
					}
					if (old_column > 0) {
						new_move_indices.push_back(persons[i].tile_index - 1);
					}
					if (old_row > 0) {
						new_move_indices.push_back(persons[i].tile_index - COLUMN_COUNT);
					}
					if (old_column < COLUMN_COUNT - 1) {
						new_move_indices.push_back(persons[i].tile_index + 1);
					}
					if (old_row < ROW_COUNT - 1) {
						new_move_indices.push_back(persons[i].tile_index + COLUMN_COUNT);
					}
					// Keeps only tile indices that does not intersect the quarantine border.
					vector<uint>::iterator iter = new_move_indices.begin();
					while (iter < new_move_indices.end() && new_move_indices.size() > 0) {
						bool is_erasable = false;
						// Erases new indices that overlaps with another person's position.
						for (size_t i = 0; i < population; i++) {
							if (*iter == persons[i].tile_index) {
								is_erasable = true;
								break;
							}
						}
						// Erases new indices that violates quarantine borders.
						int new_column = GetColumn(*iter, COLUMN_COUNT);
						int new_row = GetRow(*iter, COLUMN_COUNT);
						for (SIM::QuarantineZone zone : quarantine_zones) {
							if ((Intersects(old_column, old_row, zone) &&
								!Intersects(new_column, new_row, zone)) ||
								(!Intersects(old_column, old_row, zone) &&
								Intersects(new_column, new_row, zone))) {
								is_erasable = true;
								break;
							}
						}
						if (is_erasable) {
							new_move_indices.erase(iter);
						} else {
							iter++;
						}
					}

					// Randomly selects one new index to be assigned to the person.
					// If there is no available location to move, no movement is made.
					int new_tile_index;
					if (new_move_indices.size() != 0) {
						new_tile_index = new_move_indices[rand() % new_move_indices.size()];
						persons[i].tile_index = new_tile_index;
					}

				}
			}
		}

		// Updates state information for each person.
		for (size_t i = 0; i < population; i++) {
			// If a person is susceptible, it may become infected upon contact with an infected person.
			if (persons[i].state == SIM::SUSCEPTIBLE) {
				int index_1 = persons[i].tile_index;
				int column_1 = GetColumn(index_1, COLUMN_COUNT);
				int row_1 = GetRow(index_1, COLUMN_COUNT);
				// Transmission only occurs if the two persons are vicinal and not hospitalized.
				for (size_t i2 = 0; i2 < population; i2++) {
					if ((persons[i2].state == SIM::INCUBANT ||
						persons[i2].state == SIM::SYMPTOMATIC) &&
						(!persons[i2].is_hospitalized) &&
						IsVicinal(persons[i], persons[i2], COLUMN_COUNT)) {
						int index_2 = persons[i2].tile_index;
						int column_2 = GetColumn(index_2, COLUMN_COUNT);
						int row_2 = GetRow(index_2, COLUMN_COUNT);
						// Transmission only occurs if their status to all quarantine zones are the same.
						bool is_transmissible = true;
						for (SIM::QuarantineZone zone : quarantine_zones) {
							if ((Intersects(column_1, row_1, zone) && !Intersects(column_2, row_2, zone)) ||
								(!Intersects(column_1, row_1, zone) && Intersects(column_2, row_2, zone))) {
								is_transmissible = false;
								break;
							}
						}
						if (Executes(infection_rate)) {
							persons[i].state = SIM::INCUBANT;
							//persons[i].tick_to_symptom = min_dur_incubation +
								//rand() % (max_dur_incubation - min_dur_incubation + 1);
							// Increments infection count each time a new person is infected.
							infection_count++;
							break;
						}
					}
				}
			// If a person is incubant, it may become symptomatic.
			} else if (persons[i].state == SIM::INCUBANT) {
				if (Executes(manifestation_rate)) {
					persons[i].state = SIM::SYMPTOMATIC;
				}
			// If a person is symptomatic, it may or may not be hospitalized.
			// It may remain as such, recover or die depending on hospitalization status.
			} else if (persons[i].state == SIM::SYMPTOMATIC) {
				if (Executes(discovery_rate)) {
					persons[i].is_hospitalized = true;
				}
				unsigned int real_recovery_rate = recovery_rate *
					(persons[i].is_hospitalized ? RECOVERY_MULTIPLIER : 1);
				unsigned int real_death_rate = death_rate /
					(persons[i].is_hospitalized ? DEATH_DIVISOR : 1);
				if (Executes(real_recovery_rate)) {
					persons[i].state = acquires_immunity ? SIM::RECOVERED : SIM::SUSCEPTIBLE;
					recovery_count += acquires_immunity ? 1 : 0;
					infection_count--;
				} else {
					if (Executes(real_death_rate)) {
						persons[i].state = SIM::DEAD;
						death_count++;
						infection_count--;
					}
				}
			}
		}

		// Stores infection, death and recovery count every second and resets the msec fraction.
		result.survival_counts.push_back(population - death_count);
		result.infection_counts.push_back(infection_count);
		result.death_counts.push_back(death_count);
		result.recovery_counts.push_back(recovery_count);

		// Copies graphic information of new location and infection status into buffer.
		SDL_Rect display_rect;
		SDL_FillRect(p_window_surface, &window_rect, COLOR_BLACK);
		for (size_t i = 0; i < population; i++) {
			div_t coordinates = div((int)persons[i].tile_index, (int)COLUMN_COUNT);
			display_rect.x = POINT_SPACE * (coordinates.rem + 1) - PERSON_W / 2;
			display_rect.y = POINT_SPACE * (coordinates.quot + 1) - PERSON_H / 2;
			SDL_Surface *p_surface_to_blit = nullptr;
			switch (persons[i].state) {
			case SIM::SUSCEPTIBLE:
				p_surface_to_blit = p_susceptible_surface;
				break;
			case SIM::INCUBANT:
				p_surface_to_blit = shows_incubant ? p_incubant_surface : p_susceptible_surface;
				break;
			case SIM::SYMPTOMATIC:
				p_surface_to_blit = persons[i].is_hospitalized ?
					p_hospitalized_surface : p_symptomatic_surface;
				break;
			case SIM::RECOVERED:
				p_surface_to_blit = p_recovered_surface;
				break;
			case SIM::DEAD:
				p_surface_to_blit = p_dead_surface;
				break;
			}
			SDL_BlitSurface(p_surface_to_blit, nullptr, p_window_surface, &display_rect);
		}
		// Displays a series of dots representing the border of quanrantine zones.
		for (const SIM::QuarantineZone zone : quarantine_zones) {
			for (float half_column = zone.half_column1; half_column <= zone.half_column2;
				half_column += 0.5) {
				display_rect.x = POINT_SPACE * (half_column + 1) - PERSON_W / 2;
				display_rect.y = POINT_SPACE * (zone.half_row1 + 1) - PERSON_H / 2;
				SDL_BlitSurface(p_quarantine_border_surface, nullptr, p_window_surface, &display_rect);
				display_rect.y = POINT_SPACE * (zone.half_row2 + 1) - PERSON_H / 2;
				SDL_BlitSurface(p_quarantine_border_surface, nullptr, p_window_surface, &display_rect);
			}
			for (float half_row = zone.half_row1; half_row <= zone.half_row2; half_row += 0.5) {
				display_rect.x = POINT_SPACE * (zone.half_column1 + 1) - PERSON_W / 2;
				display_rect.y = POINT_SPACE * (half_row + 1) - PERSON_H / 2;
				SDL_BlitSurface(p_quarantine_border_surface, nullptr, p_window_surface, &display_rect);
				display_rect.x = POINT_SPACE * (zone.half_column2 + 1) - PERSON_H / 2;
				SDL_BlitSurface(p_quarantine_border_surface, nullptr, p_window_surface, &display_rect);
			}
		}
		SDL_UpdateWindowSurface(p_window);

		t_msec = clock() - t_msec;
		int msec_left = max(MSEC_PER_FRAME - t_msec, (ul)0L);
		//printf("Execution time: %d ms.\n", t_msec);

		// Determines if the loop should be exited.
		if (infection_count == 0) {
			result.end_point = SIM::ERADICATED;
			runs_simulation_loop = false;
		} else if (death_count == population) {
			result.end_point = SIM::OVERWHELMED;
			runs_simulation_loop = false;
		} else if (tick_count >= MAX_TICK_COUNT) {
			result.end_point = SIM::EQUILIBRATED;
			runs_simulation_loop = false;
		}

		// Passes one frame.
		SDL_Delay(msec_left);

		tick_count++;

	}

	// Deallocates dynamic arrays.
	delete[] tile_indices;
	delete[] persons;

	SDL_Quit();

	return result;

}



// Plots simulation results in a separate SDL window.
int DrawResult(SIM::Result result) {

	// Initializes SDL.
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window *p_window;
	SDL_Renderer *p_renderer;
	SDL_CreateWindowAndRenderer(DRAW_WIN_W, DRAW_WIN_H, SDL_WINDOW_SHOWN, &p_window, &p_renderer);
	SDL_SetWindowTitle(p_window, DRAW_WIN_TITLE);
	SDL_SetWindowPosition(p_window, DRAW_WIN_X, DRAW_WIN_Y);

	// Calculates horizontal and vertical scales of plot.
	uint min_count = min({
		*min_element(result.survival_counts.cbegin(), result.survival_counts.cend()),
		*min_element(result.infection_counts.cbegin(), result.infection_counts.cend()),
		*min_element(result.death_counts.cbegin(), result.death_counts.cend()),
		*min_element(result.recovery_counts.cbegin(), result.recovery_counts.cend())
	});
	uint max_count = max({
		*max_element(result.survival_counts.cbegin(), result.survival_counts.cend()),
		*max_element(result.infection_counts.cbegin(), result.infection_counts.cend()),
		*max_element(result.death_counts.cbegin(), result.death_counts.cend()),
		*max_element(result.recovery_counts.cbegin(), result.recovery_counts.cend())
	});
	float x_interval = ((float)PLOT_W) / result.infection_counts.size();
	float y_interval = ((float)PLOT_H) / (max_count - min_count);
	//printf("X_INTERVAL, Y_INTERVAL: %f, %f.\n", x_interval, y_interval);

	// Renders and displays the plot.
	// Clears window screen according to result type.
	switch (result.end_point) {
	case SIM::INTERRUPTED:
		SDL_SetRenderDrawColor(p_renderer, 0x10, 0x00, 0x10, SDL_ALPHA_OPAQUE);
		break;
	case SIM::OVERWHELMED:
		SDL_SetRenderDrawColor(p_renderer, 0x20, 0x00, 0x00, SDL_ALPHA_OPAQUE);
		break;
	case SIM::ERADICATED:
		SDL_SetRenderDrawColor(p_renderer, 0x00, 0x20, 0x00, SDL_ALPHA_OPAQUE);
		break;
	case SIM::EQUILIBRATED:
		SDL_SetRenderDrawColor(p_renderer, 0x0A, 0x0A, 0x0A, SDL_ALPHA_OPAQUE);
	}
	SDL_RenderClear(p_renderer);
	// Plots trend for survival count.
	SDL_SetRenderDrawColor(p_renderer, 0xFF, 0xFF, 0x00, SDL_ALPHA_OPAQUE);
	for (size_t i = 0; i < result.survival_counts.size() - 1; i++) {
		float x1 = PLOT_X_OFFSET + x_interval * i;
		float y1 = PLOT_Y_OFFSET + PLOT_H - y_interval * result.survival_counts[i];
		float x2 = PLOT_X_OFFSET + x_interval * (i + 1);
		float y2 = PLOT_Y_OFFSET + PLOT_H - y_interval * result.survival_counts[i + 1];
		//printf("Line drawn.\n");
		SDL_RenderDrawLine(p_renderer, x1, y1, x2, y2);
	}
	// Plots trend for infection count.
	SDL_SetRenderDrawColor(p_renderer, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE);
	for (size_t i = 0; i < result.infection_counts.size() - 1; i++) {
		float x1 = PLOT_X_OFFSET + x_interval * i;
		float y1 = PLOT_Y_OFFSET + PLOT_H - y_interval * result.infection_counts[i];
		float x2 = PLOT_X_OFFSET + x_interval * (i + 1);
		float y2 = PLOT_Y_OFFSET + PLOT_H - y_interval * result.infection_counts[i + 1];
		//printf("Line drawn.\n");
		SDL_RenderDrawLine(p_renderer, x1, y1, x2, y2);
	}
	// Plots trend for death count.
	SDL_SetRenderDrawColor(p_renderer, 0x80, 0x80, 0x80, SDL_ALPHA_OPAQUE);
	for (size_t i = 0; i < result.death_counts.size() - 1; i++) {
		float x1 = PLOT_X_OFFSET + x_interval * i;
		float y1 = PLOT_Y_OFFSET + PLOT_H - y_interval * result.death_counts[i];
		float x2 = PLOT_X_OFFSET + x_interval * (i + 1);
		float y2 = PLOT_Y_OFFSET + PLOT_H - y_interval * result.death_counts[i + 1];
		//printf("x1: %d. y1: %d. x2: %d. y2: %d.\n", (int)x1, (int)y1, (int)x2, (int)y2);
		SDL_RenderDrawLine(p_renderer, x1, y1, x2, y2);
	}
	// Plots trend for recovery count.
	SDL_SetRenderDrawColor(p_renderer, 0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE);
	for (size_t i = 0; i < result.recovery_counts.size() - 1; i++) {
		float x1 = PLOT_X_OFFSET + x_interval * i;
		float y1 = PLOT_Y_OFFSET + PLOT_H - y_interval * result.recovery_counts[i];
		float x2 = PLOT_X_OFFSET + x_interval * (i + 1);
		float y2 = PLOT_Y_OFFSET + PLOT_H - y_interval * result.recovery_counts[i + 1];
		SDL_RenderDrawLine(p_renderer, x1, y1, x2, y2);
	}
	// Updates screen.
	SDL_RenderPresent(p_renderer);

	SDL_Event event;
	// Initiates an event loop that awaits the user to quit SDL window.
	while (true) {
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {
			SDL_Quit();
			break;
		}
	}

	return 0;

}