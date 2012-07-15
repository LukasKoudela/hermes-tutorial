#include "definitions.h"

/* Weak forms */

CustomWeakFormPoissonNeumann::CustomWeakFormPoissonNeumann(std::string mat_al, Hermes1DFunction<double>* lambda_al,
                                                           std::string mat_cu, Hermes1DFunction<double>* lambda_cu,
                                                           Hermes2DFunction<double>* vol_src_term, std::string bdy_heat_flux,
                                                           Hermes2DFunction<double>* surf_src_term) : WeakForm<double>(1)
{
  // Jacobian forms - volumetric.
  add_matrix_form(new DefaultJacobianDiffusion<double>(0, 0, lambda_al, mat_al));
  add_matrix_form(new DefaultJacobianDiffusion<double>(0, 0, lambda_cu, mat_cu));

  // Residual forms - volumetric.
  add_vector_form(new DefaultResidualDiffusion<double>(0, lambda_al, mat_al));
  add_vector_form(new DefaultResidualDiffusion<double>(0, lambda_cu, mat_cu));
  add_vector_form(new DefaultVectorFormVol<double>(0, vol_src_term));

  // Residual forms - surface.
  add_vector_form_surf(new DefaultVectorFormSurf<double>(0, surf_src_term, bdy_heat_flux));
};

/* Custom non-constant Dirichlet condition */

CustomDirichletCondition::CustomDirichletCondition(Hermes::vector<std::string> markers, 
                                                   double A, double B, double C)
    : EssentialBoundaryCondition<double>(markers), A(A), B(B), C(C) 
{ 
}

EssentialBoundaryCondition<double>::EssentialBCValueType CustomDirichletCondition::get_value_type() const
{ 
  return EssentialBoundaryCondition<double>::BC_FUNCTION; 
}

double CustomDirichletCondition::value(double x, double y, double n_x, double n_y, double t_x, double t_y) const 
{
  return A*x + B*y + C;
}

