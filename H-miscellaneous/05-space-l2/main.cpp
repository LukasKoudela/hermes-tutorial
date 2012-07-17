#define HERMES_REPORT_ALL
#define HERMES_REPORT_FILE "application.log"
#include "definitions.h"

using namespace Hermes::Hermes2D::Views;

// This example shows how to use the L2 finite element space and L2 shapeset.
// As a sample problem, a continuous function x^3 + y^3 is projected onto the
// L2 finite element space in the L2 norm. When zero-order is used, the result
// is a piecewice constant function. The class BaseView will show you the basis
// functions.
//
// The following parameters can be changed:

// Number of initial uniform mesh refinements.
const int INIT_REF_NUM = 1;                       
// Polynomial degree of mesh elements.
const int P_INIT = 3;                            
// Matrix solver: SOLVER_AMESOS, SOLVER_AZTECOO, SOLVER_MUMPS,
// SOLVER_PETSC, SOLVER_SUPERLU, SOLVER_UMFPACK.
MatrixSolverType matrix_solver = SOLVER_UMFPACK;  

int main(int argc, char* argv[])
{
  // Load the mesh.
  Mesh mesh;
  MeshReaderH2D mloader;
  mloader.load("square.mesh", &mesh);

  // Perform uniform mesh refinements.
  for (int i = 0; i < INIT_REF_NUM; i++) mesh.refine_all_elements();

  // Create an L2 space with default shapeset.
  L2Space<double> space(&mesh, P_INIT);

  // View basis functions.
  BaseView<double> bview("BaseView", new WinGeom(0, 0, 600, 500));
  bview.show(&space);
  // View::wait(H2DV_WAIT_KEYPRESS);

  // Initialize the exact and projected solution.
  Solution<double> sln;
  CustomExactSolution sln_exact(&mesh);

  // Project the exact function on the FE space.
  OGProjection<double>::project_global(&space, &sln_exact, &sln, matrix_solver);

  // Visualize the projection.
  ScalarView view1("Projection", new WinGeom(610, 0, 600, 500));
  view1.show(&sln);

  // Wait for all views to be closed.
  View::wait();
  return 0;
}

