#include "hermes2d.h"

using namespace Hermes;
using namespace Hermes::Hermes2D;
using namespace Hermes::Hermes2D::RefinementSelectors;
using namespace Hermes::Hermes2D::Views;

class CustomWeakForm : public WeakForm<double>
{
public:

  CustomWeakForm(std::string left_bottom_bnd_part, bool DG = true);

private:
  class MatrixFormVol : public Hermes::Hermes2D::MatrixFormVol<double>
  {
  public:
    MatrixFormVol(int i, int j);

    template<typename Real, typename Scalar>
    Scalar matrix_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

    virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;
    Hermes::Hermes2D::MatrixFormVol<double>* clone();
  };

  class VectorFormVol : public Hermes::Hermes2D::VectorFormVol<double>
  {
  public:
    VectorFormVol(int i);

    template<typename Real, typename Scalar>
    Scalar vector_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

    virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;
    Hermes::Hermes2D::VectorFormVol<double>* clone();

    template<typename Real>
    Real F(Real x, Real y) const;
  };

  class MatrixFormSurface : public Hermes::Hermes2D::MatrixFormSurf<double>
  {
  public:
    MatrixFormSurface(int i, int j);

    template<typename Real, typename Scalar>
    Scalar matrix_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

    virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;
    Hermes::Hermes2D::MatrixFormSurf<double>* clone();
  };

  class MatrixFormInterface : public Hermes::Hermes2D::MatrixFormSurf<double>
  {
  public:
    MatrixFormInterface(int i, int j);

    template<typename Real, typename Scalar>
    Scalar matrix_form(int n, double *wt, Func<Scalar> *u_ext[], Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext) const;

    virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;
    Hermes::Hermes2D::MatrixFormSurf<double>* clone();
  };

  class VectorFormSurface : public Hermes::Hermes2D::VectorFormSurf<double>
  {
  public:
    VectorFormSurface(int i, std::string left_bottom_bnd_part);

    virtual double value(int n, double *wt, Func<double> *u_ext[], Func<double> *v, Geom<double> *e, ExtData<double> *ext) const;

    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e, ExtData<Ord> *ext) const;

    template<typename Real>
    Real F(Real x, Real y) const;
    Hermes::Hermes2D::VectorFormSurf<double>* clone();
  };

  double calculate_a_dot_v(double x, double y, double vx, double vy) const;

  Ord calculate_a_dot_v(Ord x, Ord y, Ord vx, Ord vy) const;

  double upwind_flux(double u_cent, double u_neib, double a_dot_n) const;

  Ord upwind_flux(Ord u_cent, Ord u_neib, Ord a_dot_n) const;
};
