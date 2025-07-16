#include "Meanshift.hpp"

using namespace clustering;

Meanshift::Meanshift(int kernel_type, double kernel_bandwidth,
    int dim, double mode_tolerance)
    : dim(dim), kernel_type(kernel_type),
        kernel_bandwidth(kernel_bandwidth),
        mode_tolerance(mode_tolerance) {
    assert(kernel_type >= 0 && kernel_type <= 1);
    assert(kernel_bandwidth > 0.0);
    assert(dim > 0);

    // Compute normalization constant
    if (kernel_type == 0) {
        kernel_c = 0.5 * (static_cast<double>(dim) + 2.0) / 1.234;
        // TODO: replace 1.234 with the volume of the d-dimensional unit
        // sphere FIXME
    } else if (kernel_type == 1) {
        kernel_c = pow(2.0 * M_PI, -static_cast<double>(dim)/2.0);
    }
}

void Meanshift::FindModes(const std::vector<std::vector<double> >& X,
    std::vector<std::vector<double> >& modes,
    std::vector<int>& indexmap,
    unsigned int procedure_count) const
{
    //assert(indexmap.size() == X.size());
    indexmap.clear();
    indexmap.resize(X.size(), 0);

    modes.clear();

    if (procedure_count != 0 && procedure_count >= X.size())
        procedure_count = 0;

    // Perform dense mode finding: for each sample in X, perform the mean
    // shift procedure
    std::vector<double> mode(X[0].size());
    std::vector<unsigned int> visited(X.size());
    // [mode_index][sample_index] = number of times sample_index was visited
    //   when arriving at the mode with the mode_index.
    std::vector<std::map<unsigned int, unsigned int> > visit_counts;
    for (unsigned int si = 0; (procedure_count == 0 && si < X.size())
        || (procedure_count != 0 && si < procedure_count); ++si) {
        if (procedure_count == 0) {
            gmm::copy(X[si], mode);
        } else {
            // Pick a random one from X
            unsigned sample_index = rand() % X.size();
            gmm::copy(X[sample_index], mode);
        }
        std::cout << "Sample " << si << " of "
            << (procedure_count == 0 ? X.size() : procedure_count)
            << std::endl;
        gmm::clear(visited);
        MeanshiftProcedure(X, mode, visited);

        // Identify whether this is a novel mode or a known one
        bool found = false;
        unsigned int M_cur = 0;
        for (std::vector<std::vector<double> >::iterator Mi =
            modes.begin(); found == false && Mi != modes.end(); ++Mi) {
            if (gmm::vect_dist2(*Mi, mode) < mode_tolerance) {
                M_cur = Mi - modes.begin();
                found = true;
            }
        }
        if (found == false) {
            // Add novel mode
            modes.push_back(mode);
            // TODO: remove this debug output
            std::cout << modes.size() << " mode"
                << (modes.size() >= 2 ? "s" : "") << std::endl;

            // Add a new mapping of which samples have been visited while
            // approaching the novel mode.
            std::map<unsigned int, unsigned int> mode_visits;
            for (std::vector<unsigned int>::const_iterator vi =
                visited.begin(); vi != visited.end(); ++vi) {
                if (*vi != 0)
                    mode_visits[vi - visited.begin()] = 1;
            }
            visit_counts.push_back(mode_visits);
        } else {
            // The mode has been known, but we maybe crossed old and new
            // samples.  Update the counts.
            for (std::vector<unsigned int>::const_iterator vi =
                visited.begin(); vi != visited.end(); ++vi) {
                if (*vi != 0)
                    visit_counts[M_cur][vi - visited.begin()] += 1;
            }
        }
    }
#ifdef DEBUG
    std::cout << "Found " << modes.size() << " modes." << std::endl;
#endif

    // Perform index mapping: each sample gets assigned to one mode index.
    unsigned int unmapped_count = 0;
    for (unsigned int sample_index = 0;
        sample_index < X.size(); ++sample_index) {
        // Find mode index with highest count
        unsigned int maximum_count = 0;
        int maximum_mode_index = -1;
        for (unsigned int mode_index = 0;
            mode_index < modes.size(); ++mode_index) {
            if (visit_counts[mode_index].count(sample_index) == 0)
                continue;

            unsigned int count_cur = visit_counts[mode_index][sample_index];
            std::cout << "  mode " << mode_index << " visited "
                << "sample " << sample_index << " for "
                << count_cur << " times." << std::endl;
            if (count_cur > maximum_count) {
                maximum_mode_index = mode_index;
                maximum_count = count_cur;
            }
        }
        if (maximum_mode_index == -1)
            unmapped_count += 1;
        indexmap[sample_index] = maximum_mode_index;
    }
    std::cout << unmapped_count << " unmapped samples." << std::endl;
}

int Meanshift::MeanshiftProcedure(const std::vector<std::vector<double> >& X,
    std::vector<double>& mode, std::vector<unsigned int>& visited) const {
    assert(X.size() > 0);
    assert(visited.size() == X.size());
    assert(X[0].size() == mode.size());
    std::vector<double> meanshift(mode.size(),0);
    std::vector<double> meanshift_cur(mode.size(),0);
    int iter = 0;
    bool converged = false;
    do {
        // Compute the mean shift vector
        gmm::clear(meanshift);
        double denominator = 0.0;
        for (std::vector<std::vector<double> >::const_iterator Xi = X.begin();
            Xi != X.end(); ++Xi) {
            gmm::copy(mode, meanshift_cur);
            gmm::add(gmm::scaled(*Xi, -1.0), meanshift_cur);
            gmm::scale(meanshift_cur, 1.0 / kernel_bandwidth);
            double weight_cur = gmm::vect_norm2(meanshift_cur);
            if (kernel_type == 0) {
                // Epanechnikov kernel is the shadow of the uniform kernel
                if (weight_cur <= 1.0)
                    weight_cur = 1.0;
                else
                    weight_cur = 0.0;
            } else if (kernel_type == 1) {
                // Multivariate normal kernel is the shadow of itself
                weight_cur = kernel_c * exp(-0.5 * weight_cur);

                // Truncate
                if (weight_cur < 1e-2)
                    weight_cur = 0.0;
            }
            if (weight_cur >= 1e-6)
                visited[Xi - X.begin()] = 1;

            gmm::add(gmm::scaled(*Xi, weight_cur), meanshift);
            denominator += weight_cur;
        }

        gmm::scale(meanshift, 1.0 / denominator);
        double distance_moved = gmm::vect_dist2(meanshift, mode);
        gmm::copy(meanshift, mode);

#ifdef DEBUG
        std::cout << "iter " << iter << ", moved "
            << distance_moved << std::endl;
#endif
        iter += 1;
        if (distance_moved <= 1e-8)
            converged = true;
    } while (converged == false);

    return (iter);
}
