#ifndef __ADJOINT_H__
#define __ADJOINT_H__

/* includes */
#include <vector>
#include <iostream>

#include <deal.II/lac/la_parallel_vector.h>
#include <deal.II/distributed/solution_transfer.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_values.h>

#include "parameters/all_parameters.h"

#include "functional.h"
#include "dg/dg.h"
#include "physics/physics.h"

namespace PHiLiP {

/// For storing the current state in the adjoint
enum AdjointEnum {
    coarse, ///< Initial state
    fine,   ///< Refined state
};

/// Adjoint class
/** 
  * This class computes the discrete adjoint of the system based on a functional of interest and
  * a computed DG solution. Uses the Sacado functions Functional::evaluate_dIdw() and DGBase::assemble_residual()
  * to generate and solve the discrete adjoint system
  * 
  * \f[
  *     \left( \frac{\partial \mathbf{R}}{\partial \mathbf{u}} \right)^T \psi 
  *     + \left(\frac{\partial \mathcal{J}}{\partial \mathbf{u}}\right)^T = \mathbf{0}
  * \f]
  * 
  * Includes functions for solving both the coarse and fine \f$p\f$-enriched adjoint problems. Subscripts \f$H\f$ 
  * and \f$h\f$ are used to denote coarse and fine grid variables respectively. 
  */ 
template <int dim, int nstate, typename real>
class Adjoint
{
#if PHILIP_DIM==1 // dealii::parallel::distributed::Triangulation<dim> does not work for 1D
    /// Triangulation type.
    using Triangulation = dealii::Triangulation<dim>;
#else
    /// Triangulation type.
    using Triangulation = dealii::parallel::distributed::Triangulation<dim>;
#endif
public:

    /// Constructor
    /** Initializes the Adjoint as being in the AdjointEnum::coarse state.
     *  Also stores the current solution and distribution of polynomial orders
     *  for the mesh for converting back to coarse state after refinement.
     */
    Adjoint(
        DGBase<dim,real> &_dg,
        Functional<dim, nstate, real> &_functional,
        const Physics::PhysicsBase<dim,nstate,Sacado::Fad::DFad<real>> &_physics);

    ///destructor
    ~Adjoint();

    /// Reinitialize Adjoint with the same pointers
    /** Sets adjoint_state to AdjointEnum::coarse and stores the current
     *  solution and polynomial order distribution
     */
    void reinit();
    // to reinitialize with other pointers, just create a new class

    /// Converts the adjoint to specified state
    /** Calls the functions coarse_to_fine() or fine_to_coarse()
     *  if the Adjoint::adjoint_state is different than the input \p state
     */
    void convert_to_state(AdjointEnum state);

    /// Projects the problem to a p-enriched space
    /** Raises the FE_index on each cell and transfers the coarse 
     *  solution to a fine solution (stored in DGBase::solution)
     */
    void coarse_to_fine();

    /// Return the problem to the original solution and polynomial distribution
    /** Copies the values that were stored in solution_coarse and 
     *  Adjoint::coarse_fe_index at intilization
     */
    void fine_to_coarse();

    /// Computes the fine grid adjoint
    /** Converts the state to a refined grid (if needed) and solves for Adjoint::adjoint_fine from 
     *  \f[
     *      \left(\left. \frac{\partial \mathbf{R}_h}{\partial \mathbf{u}} \right|_{\mathbf{u}_h^H}\right)^T \psi_h 
     *      + \left(\left. \frac{\partial \mathcal{J}_h}{\partial \mathbf{u}} \right|_{\mathbf{u}_h^H}\right)^T=\mathbf{0}
     *  \f]
     *  where \f$\mathbf{u}_h^H\f$ is the projected solution on the fine grid.
     */ 
    dealii::LinearAlgebra::distributed::Vector<real> fine_grid_adjoint();

    /// Computes the coarse grid adjoint
    /** Reverts the state to the coarse grid (if needed) and solves for Adjoint::adjoint_coarse from
     * \f[
     *      \left(\left. \frac{\partial \mathbf{R}_H}{\partial \mathbf{u}} \right|_{\mathbf{u}_H}\right)^T \psi_H 
     *      + \left(\left. \frac{\partial \mathcal{J}_H}{\partial \mathbf{u}} \right|_{\mathbf{u}_H}\right)^T=\mathbf{0}
     * \f]
     */
    dealii::LinearAlgebra::distributed::Vector<real> coarse_grid_adjoint();

    /// compute the Dual Weighted Residual (DWR)
    /** Computes Adjoint::dual_weighted_resiudal_fine (\f$\eta\f$) on the fine grid. This value should be
     *  zero on the coarse grid due to Galerkin Orthogonality. It is calculated from
     *  \f[
     *      \eta = \mathbf{R}_h(\mathbf{u}_h^H)^T \psi_h
     *  \f]
     *  Uses Adjoint::adjoint_fine and should only be called after fine_grid_adjoint().
     */
    dealii::Vector<real> dual_weighted_residual();

    /// Outputs the current solution and adjoint values
    /** Similar to DGBase::output_results_vtk() but will also include the adjoint and dIdw
     *  related to the current adjoint state. Will also output Adjoint::dual_weighted_residual_fine
     *  if currenly on the fine grid.
     */
    void output_results_vtk(const unsigned int cycle);

    /// DG class pointer
    DGBase<dim,real> &dg;
    /// Functional class pointer
    Functional<dim, nstate, real> &functional;
    /// Problem physics (for calling the functional class) 
    const Physics::PhysicsBase<dim,nstate,Sacado::Fad::DFad<real>> &physics;
    
    /// Grid
    Triangulation *const triangulation;
    /// original solution
    dealii::LinearAlgebra::distributed::Vector<real> solution_coarse;
    /// functional derivative (on the fine grid)
    dealii::LinearAlgebra::distributed::Vector<real> dIdw_fine;
    /// functional derivative (on the coarse grid)
    dealii::LinearAlgebra::distributed::Vector<real> dIdw_coarse;
    /// fine grid adjoint (\f$\psi_h\f$)
    dealii::LinearAlgebra::distributed::Vector<real> adjoint_fine;
    /// coarse grid adjoint (\f$\psi_H\f$)
    dealii::LinearAlgebra::distributed::Vector<real> adjoint_coarse;
    /// dual weighted residual
    /** always on the fine grid due to galerkin orthogonality
     */ 
    dealii::Vector<real> dual_weighted_residual_fine;
    
    /// Original FE_index distribution
    dealii::Vector<real> coarse_fe_index;

    /// Current adjoint state
    AdjointEnum adjoint_state;

protected:
    MPI_Comm mpi_communicator; ///< MPI communicator
    dealii::ConditionalOStream pcout; ///< Parallel std::cout that only outputs on mpi_rank==0

}; // Adjoint class


} // PHiLiP namespace

#endif // __ADJOINT_H__
