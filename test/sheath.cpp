#include <vector>
#include <Eigen/Dense>
#include "const.hpp"
#include "domain.hpp"
#include "species.hpp"
#include "source.hpp"
#include "solver.hpp"

using namespace std;
using namespace Const;
using namespace Eigen;
using PBC = ParticleBCtype;
using FBC = FieldBCtype;

int main()
{
	Vector3d x_min = {0.00, -0.00075, -0.00075};
	Vector3d x_max = {0.03,  0.00075,  0.00075};

	Domain domain("test/simulation/sheath", 21, 2, 2);
	domain.set_dimensions(x_min, x_max);
	domain.set_time_step(2e-10);
	domain.set_iter_max(100000);

	domain.set_bc_at(Xmin, BC(PBC::Open,     FBC::Dirichlet));
	domain.set_bc_at(Xmax, BC(PBC::Open,     FBC::Dirichlet, -0.18011));
	domain.set_bc_at(Ymin, BC(PBC::Periodic, FBC::Periodic));
	domain.set_bc_at(Ymax, BC(PBC::Periodic, FBC::Periodic));
	domain.set_bc_at(Zmin, BC(PBC::Periodic, FBC::Periodic));
	domain.set_bc_at(Zmax, BC(PBC::Periodic, FBC::Periodic));

	vector<Species> species;
	species.push_back(Species("Xe+", 54*AMU,  QE, 10, domain));
	species.push_back(Species("e-",      ME, -QE, 10, domain));

	const double n = 1e12;

	vector<unique_ptr<Source>> sources;
	Vector3d x1 = {0.0, -0.00075, -0.00075};
	Vector3d x2 = {0.0,  0.00075,  0.00075};
	Vector3d vi = {11492.19, 0, 0};
	Vector3d ve = {0, 0, 0};
	double   T  = 1000;
	sources.push_back(make_unique<WarmBeam>(species[0], domain, x1, x2, vi, n, T));
	sources.push_back(make_unique<WarmBeam>(species[1], domain, x1, x2, ve, n, T));

	Solver solver(domain, 1000, 1e-4);

	domain.check_formulation(n, T);

	while (domain.advance_time()) {
		domain.calc_charge_density(species);
		solver.calc_potential();
		solver.calc_electric_field();

		for (auto &source : sources)
			source->sample();

		for (Species &sp : species) {
			sp.push_particles_leapfrog();
			sp.remove_dead_particles();
			sp.calc_number_density();
		}

		if (!domain.averaing_time() && domain.steady_state(species, 5000, 0.01)) {
			domain.start_averaging_time();
			for(Species &sp : species)
				sp.start_time_averaging(10000);
		}

		if (domain.get_iter()%1000 == 0 || domain.is_last_iter()) {
			for(Species &sp : species) {
				sp.sample_moments();
				sp.calc_gas_properties();
				sp.calc_macroparticle_count();
			}

			domain.print_info(species);
			domain.write_statistics(species);
			domain.save_fields(species);
			//domain.save_particles(species, 1000);
			//domain.save_velocity_histogram(species);
		}
	}
}
