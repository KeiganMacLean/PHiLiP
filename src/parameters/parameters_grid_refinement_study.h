#ifndef __PARAMETERS_GRID_REFINEMENT_STUDY_H__
#define __PARAMETERS_GRID_REFINEMENT_STUDY_H__

#include <deal.II/base/parameter_handler.h>

#include "parameters/parameters_functional.h"
#include "parameters/parameters_grid_refinement.h"
#include "parameters/parameters_manufactured_solution.h"
#include "parameters/parameters.h"

namespace PHiLiP {

namespace Parameters {

class GridRefinementStudyParam
{
public:
    GridRefinementStudyParam(); ///< Constructor

    FunctionalParam functional_param;
    
    GridRefinementParam grid_refinement_param;

    ManufacturedSolutionParam manufactured_solution_param;

    /// Declares the possible variables and sets the defaults.
    static void declare_parameters(dealii::ParameterHandler &prm);
    /// Parses input file and sets the variables.
    void parse_parameters(dealii::ParameterHandler &prm);
};

} // Parameters namespace

} // PHiLiP namespace

#endif // __PARAMETERS_GRID_REFINEMENT_STUDY_H__