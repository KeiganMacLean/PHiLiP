set(TESTS_SOURCE
    tests.cpp
    grid_study.cpp
    burgers_stability.cpp
    diffusion_exact_adjoint.cpp
    euler_gaussian_bump.cpp
    euler_naca0012.cpp
    euler_bump_optimization.cpp
    euler_naca0012_optimization.cpp
    shock_1d.cpp
#euler_gaussian_bump_adjoint.cpp
    euler_cylinder.cpp
    euler_cylinder_adjoint.cpp
    euler_vortex.cpp
    euler_entropy_waves.cpp
    advection_explicit_periodic.cpp
    euler_split_inviscid_taylor_green_vortex.cpp
 optimization_inverse_manufactured/optimization_inverse_manufactured.cpp
)

foreach(dim RANGE 1 3)
    # Output library
    string(CONCAT TestsLib Tests_${dim}D)
    add_library(${TestsLib} STATIC ${TESTS_SOURCE})
    # Replace occurences of PHILIP_DIM with 1, 2, or 3 in the code
    target_compile_definitions(${TestsLib} PRIVATE PHILIP_DIM=${dim})

    # Library dependency
    string(CONCAT GridsLib Grids_${dim}D)
    string(CONCAT NumericalFluxLib NumericalFlux_${dim}D)
    string(CONCAT PhysicsLib Physics_${dim}D)
    string(CONCAT DiscontinuousGalerkinLib DiscontinuousGalerkin_${dim}D)
    string(CONCAT FunctionalLib Functional_${dim}D)
    string(CONCAT ODESolverLib ODESolver_${dim}D)
    string(CONCAT OptimizationLib Optimization_${dim}D)
    string(CONCAT LinearSolverLib LinearSolver)
    target_link_libraries(${TestsLib} ${GridsLib})
    target_link_libraries(${TestsLib} ${NumericalFluxLib})
    target_link_libraries(${TestsLib} ${PhysicsLib})
    target_link_libraries(${TestsLib} ${FunctionalLib})
    target_link_libraries(${TestsLib} ${DiscontinuousGalerkinLib})
    target_link_libraries(${TestsLib} ${ODESolverLib})
    target_link_libraries(${TestsLib} ${LinearSolverLib})
    target_link_libraries(${TestsLib} ${OptimizationLib})
    # Setup target with deal.II
    if(NOT DOC_ONLY)
        DEAL_II_SETUP_TARGET(${TestsLib})
    endif()

    unset(GridsLib)
    unset(TestsLib)
    unset(DiscontinuousGalerkinLib)
    unset(FunctionalLib)
    unset(ODESolverLib)
    unset(NumericalFluxLib)
    unset(PhysicsLib)
    unset(LinearSolverLib)
    unset(OptimizationLib)

endforeach()
