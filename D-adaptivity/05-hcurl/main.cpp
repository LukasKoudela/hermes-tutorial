#define HERMES_REPORT_ALL
#define HERMES_REPORT_FILE "application.log"
#include "definitions.h"

//  This example comes with an exact solution, and it describes the diffraction
//  of an electromagnetic wave from a re-entrant corner. Convergence graphs saved
//  (both exact error and error estimate, and both wrt. dof number and cpu time).
//
//  PDE: time-harmonic Maxwell's equations
//
//  Known exact solution, see functions exact_sol_val(), exact_sol(), exact()
//
//  Domain: L-shape domain
//
//  Meshes: you can use either "lshape3q.mesh" (quadrilateral mesh) or
//          "lshape3t.mesh" (triangular mesh). See the mesh.load(...) command below.
//
//  BC: perfect conductor on boundary markers 1 and 6 (essential BC)
//      impedance boundary condition on rest of boundary (natural BC)
//
//  The following parameters can be changed:

// Initial polynomial degree. NOTE: The meaning is different from
// standard continuous elements in the space H1. Here, P_INIT refers
// to the maximum poly order of the tangential component, and polynomials
// of degree P_INIT + 1 are present in element interiors. P_INIT = 0
// is for Whitney elements.
const int P_INIT = 2;
// Number of initial uniform mesh refinements.
const int INIT_REF_NUM = 1;
// This is a quantitative parameter of the adapt(...) function and
// it has different meanings for various adaptive strategies.
const double THRESHOLD = 0.3;
// Adaptive strategy:
// STRATEGY = 0 ... refine elements until sqrt(THRESHOLD) times total
//   error is processed. If more elements have similar errors, refine
//   all to keep the mesh symmetric.
// STRATEGY = 1 ... refine all elements whose error is larger
//   than THRESHOLD times maximum element error.
// STRATEGY = 2 ... refine all elements whose error is larger
//   than THRESHOLD.
// More adaptive strategies can be created in adapt_ortho_h1.cpp.
const int STRATEGY = 1;
// Predefined list of element refinement candidates. Possible values are
// H2D_P_ISO, H2D_P_ANISO, H2D_H_ISO, H2D_H_ANISO, H2D_HP_ISO,
// H2D_HP_ANISO_H, H2D_HP_ANISO_P, H2D_HP_ANISO.
const CandList CAND_LIST = H2D_HP_ANISO;
// Maximum allowed level of hanging nodes:
// MESH_REGULARITY = -1 ... arbitrary level hangning nodes (default),
// MESH_REGULARITY = 1 ... at most one-level hanging nodes,
// MESH_REGULARITY = 2 ... at most two-level hanging nodes, etc.
// Note that regular meshes are not supported, this is due to
// their notoriously bad performance.
const int MESH_REGULARITY = -1;
// This parameter influences the selection of
// candidates in hp-adaptivity. Default value is 1.0. 
const double CONV_EXP = 1.0;
// Stopping criterion for adaptivity.
const double ERR_STOP = 1.0;
// Adaptivity process stops when the number of degrees of freedom grows
// over this limit. This is to prevent h-adaptivity to go on forever.
const int NDOF_STOP = 60000;
// Matrix solver: SOLVER_AMESOS, SOLVER_AZTECOO, SOLVER_MUMPS,
// SOLVER_PETSC, SOLVER_SUPERLU, SOLVER_UMFPACK.
MatrixSolverType matrix_solver = SOLVER_UMFPACK;

// Problem parameters.
const double MU_R   = 1.0;
const double KAPPA  = 1.0;
const double LAMBDA = 1.0;

int main(int argc, char* argv[])
{
  // Time measurement
  Hermes::Mixins::TimeMeasurable cpu_time;
  cpu_time.tick();

  // Load the mesh.
  Mesh mesh;
  MeshReaderH2D mloader;
  // Quadrilateral mesh.
  mloader.load("lshape3q.mesh", &mesh);    
  // Triangular mesh.
  //mloader.load("lshape3t.mesh", &mesh);  

  // Perform initial mesh refinemets.
  for (int i = 0; i < INIT_REF_NUM; i++)  mesh.refine_all_elements();

  // Initialize boundary conditions.
  Hermes::Hermes2D::DefaultEssentialBCConst<std::complex<double> > bc_essential(Hermes::vector<std::string>("Corner_horizontal",
    "Corner_vertical"), 0);
  EssentialBCs<std::complex<double> > bcs(&bc_essential);

  // Create an Hcurl space with default shapeset.
  HcurlSpace<std::complex<double> > space(&mesh, &bcs, P_INIT);
  int ndof = space.get_num_dofs();
  Hermes::Mixins::Loggable::Static::info("ndof = %d", ndof);

  // Initialize the weak formulation.
  CustomWeakForm wf(MU_R, KAPPA);

  // Initialize coarse and reference mesh solutions.
  Solution<std::complex<double> > sln, ref_sln;

  // Initialize exact solution.
  CustomExactSolution sln_exact(&mesh);

  // Initialize refinement selector.
  HcurlProjBasedSelector selector(CAND_LIST, CONV_EXP, H2DRS_DEFAULT_ORDER);

  // Initialize views.
  Views::ScalarView v_view("Solution (magnitude)", new Views::WinGeom(0, 0, 460, 350));
  v_view.set_min_max_range(0, 1.5);
  Views::OrderView  o_view("Polynomial orders", new Views::WinGeom(470, 0, 400, 350));

  // DOF and CPU convergence graphs.
  SimpleGraph graph_dof_est, graph_cpu_est,
    graph_dof_exact, graph_cpu_exact;

  DiscreteProblem<std::complex<double> > dp(&wf, &space);

  // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
  Hermes::Hermes2D::NewtonSolver<std::complex<double> > newton(&dp);

  // Adaptivity loop:
  int as = 1; bool done = false;
  do
  {
    Hermes::Mixins::Loggable::Static::info("---- Adaptivity step %d:", as);

    // Construct globally refined reference mesh and setup reference space.
    Space<std::complex<double> >* ref_space = Space<std::complex<double> >::construct_refined_space(&space);
    newton.set_space(ref_space);
    int ndof_ref = ref_space->get_num_dofs();

    // Initial coefficient vector for the Newton's method.
    std::complex<double>* coeff_vec = new std::complex<double>[ndof_ref];
    memset(coeff_vec, 0, ndof_ref * sizeof(std::complex<double>));

    // Time measurement.
    cpu_time.tick();

    try
    {
      newton.solve(coeff_vec);
    }
    catch(std::exception& e)
    {
      std::cout << e.what();
      
    }

    // Translate the resulting coefficient vector into a Solution.    
    Hermes::Hermes2D::Solution<std::complex<double> >::vector_to_solution(newton.get_sln_vector(), ref_space, &ref_sln);

    // Time measurement.
    cpu_time.tick();

    // Project the fine mesh solution onto the coarse mesh.
    Hermes::Mixins::Loggable::Static::info("Projecting reference solution on coarse mesh.");
    OGProjection<std::complex<double> > ogProjection;
    ogProjection.project_global(&space, &ref_sln, &sln);

    // View the coarse mesh solution and polynomial orders.
    RealFilter real(&sln);
    MagFilter<double> magn(&real);
    v_view.show(&magn);
    o_view.show(&space);

    // Calculate element errors and total error estimate.
    Hermes::Mixins::Loggable::Static::info("Calculating error estimate and exact error.");
    Adapt<std::complex<double> >* adaptivity = new Adapt<std::complex<double> >(&space);
    double err_est_rel = adaptivity->calc_err_est(&sln, &ref_sln) * 100;

    // Calculate exact error.
    bool solutions_for_adapt = false;
    double err_exact_rel = adaptivity->calc_err_exact(&sln, &sln_exact, solutions_for_adapt) * 100;

    // Report results.
    Hermes::Mixins::Loggable::Static::info("ndof_coarse: %d, ndof_fine: %d",
      space.get_num_dofs(), ref_space->get_num_dofs());
    Hermes::Mixins::Loggable::Static::info("err_est_rel: %g%%, err_exact_rel: %g%%", err_est_rel, err_exact_rel);

    // Time measurement.
    cpu_time.tick();

    // Add entry to DOF and CPU convergence graphs.
    graph_dof_est.add_values(space.get_num_dofs(), err_est_rel);
    graph_dof_est.save("conv_dof_est.dat");
    graph_cpu_est.add_values(cpu_time.accumulated(), err_est_rel);
    graph_cpu_est.save("conv_cpu_est.dat");
    graph_dof_exact.add_values(space.get_num_dofs(), err_exact_rel);
    graph_dof_exact.save("conv_dof_exact.dat");
    graph_cpu_exact.add_values(cpu_time.accumulated(), err_exact_rel);
    graph_cpu_exact.save("conv_cpu_exact.dat");

    // If err_est_rel too large, adapt the mesh.
    if (err_est_rel < ERR_STOP) done = true;
    else
    {
      Hermes::Mixins::Loggable::Static::info("Adapting coarse mesh.");
      done = adaptivity->adapt(&selector, THRESHOLD, STRATEGY, MESH_REGULARITY);

      // Increase the counter of performed adaptivity steps.
      if (done == false)  as++;
    }
    if (space.get_num_dofs() >= NDOF_STOP) done = true;

    // Clean up.
    delete [] coeff_vec;
    delete adaptivity;
  }
  while (done == false);

  Hermes::Mixins::Loggable::Static::info("Total running time: %g s", cpu_time.accumulated());

  // Show the reference solution - the final result.
  v_view.set_title("Fine mesh solution (magnitude)");
  RealFilter ref_real(&sln);
  MagFilter<double> ref_magn(&ref_real);
  ValFilter ref_limited_magn(&ref_magn, 0.0, 1.0);
  v_view.show(&ref_limited_magn);

  // Output solution in VTK format.
  Views::Linearizer lin;
  bool mode_3D = true;
  lin.save_solution_vtk(&ref_limited_magn, "sln.vtk", "Magnitude of E", mode_3D);
  Hermes::Mixins::Loggable::Static::info("Solution in VTK format saved to file %s.", "sln.vtk");

  // Wait for all views to be closed.
  Views::View::wait();
  return 0;
}

