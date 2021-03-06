#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <vector>
#include <Eigen/Eigen>
#include "domain.hpp"

class Solver {
	public:
		using T = Eigen::Triplet<double>;
		using SpMat = Eigen::SparseMatrix<double>;
		using Vector3d = Eigen::Vector3d;
		using VectorXd = Eigen::VectorXd;

		Solver(Domain &domain, int iter_max, double tol);

		void set_reference_values(double phi0, double Te0, double n0);

		void calc_potential();

		void calc_potential_BR();

		void calc_electric_field(const Vector3d &E_ext = {0, 0, 0});

	private:
		Domain &domain;

		int n_nodes;
		VectorXd is_regular; /* 1 if node is regular, 0 if not */

		SpMat A;
		VectorXd b0;
		Eigen::BiCGSTAB<SpMat> solver;

		int iter_max, newton_iter_max = 20;
		double tol, newton_tol = 1e-4;

		double phi0, n0, Te0;

		int at(int i, int j, int k) const {return domain.at(i, j, k);}
};

#endif
