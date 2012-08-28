#ifndef LOWESSHELPERS_H
#define LOWESSHELPERS_H


//Min and max macros that avoid double evaluation. Always good to have
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a < _b ? _a : _b; })
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a > _b ? _a : _b; })

inline float median(float *sortedArr, int n)
{
    if (n % 2 == 0)
        return (sortedArr[n/2-1] + sortedArr[n/2]) / 2.0f;
    else
        return sortedArr[n/2];
}

//Single-pass, numerically stable algorithm for calculating the mean and stddev of a data set.
//Found on http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance.
//Credited to Donald Knuth, who cites B.P. Welford (1962).
void calcDescriptiveStats(fvec const &data, float *mean, float *stdev, float *iqr)
{
    int   n  = 0;
    float m  = 0.0f;
    float m2 = 0.0f;
    float delta;

    FOR(i, data.size())
    {
        n++;
        delta = data[i] - m;
        m    += delta/(float)n;
        m2   += delta*(data[i] - m);
    }

    if (mean  != NULL)
        *mean  = m;
    if (stdev != NULL)
        *stdev = sqrtf(m2/(float)n);

    if (iqr != NULL)
    {
        float q1, q3;
        fvec sorted = data;
        std::sort(sorted.begin(), sorted.end());

        if (n < 2)
            q3 = q1 = 0.0f;
        else if (n % 2 == 0)
        {
            q1 = median(&sorted[0],   n/2);
            q3 = median(&sorted[n/2], n/2);
        }
        else
        {
            int   iPart;
            float fPart;

            float q1Pos = 0.25f*n + 0.5f;
            iPart = (int)q1Pos;
            fPart = q1Pos - iPart;
            q1 = (1-fPart) * sorted[iPart-1] + fPart * sorted[iPart];

            float q3Pos = 0.75f*n + 0.5;
            iPart = (int)q3Pos;
            fPart = q3Pos - iPart;
            q3 = (1-fPart) * sorted[iPart-1] + fPart * sorted[iPart];
        }
        *iqr = q3 - q1;
    }
}


/*
 * @(#)MergeSortAlgorithm.java	1.0 95/06/23 Jason Harrison
 *
 * Copyright (c) 1995 University of British Columbia
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for NON-COMMERCIAL purposes and without
 * fee is hereby granted provided that this copyright notice
 * appears in all copies. Please refer to the file "copyright.html"
 * for further important copyright and licensing information.
 *
 * UBC MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF
 * THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. UBC SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

/**
 * A merge sort demonstration algorithm
 * SortAlgorithm.java, Thu Oct 27 10:32:35 1994
 *
 * @author Jason Harrison@cs.ubc.ca
 * @version 	1.1, 12 Jan 1998
 */

//Modified by R. Ullmann to update a permutation array instead of modifying the input data

void mergesort_perm(float const data[], size_t permIndexes[], int lo0, int hi0)
{
    int lo = lo0;
    int hi = hi0;
    if (lo >= hi) {
        return;
    }
    int mid = (lo + hi) / 2;

    /*
     *  Partition the list into two lists and sort them recursively
     */
    mergesort_perm(data, permIndexes, lo, mid);
    mergesort_perm(data, permIndexes, mid + 1, hi);

    /*
     *  Merge the two sorted lists
     */
    int end_lo = mid;
    int start_hi = mid + 1;
    while ((lo <= end_lo) && (start_hi <= hi))
    {
        if (data[permIndexes[lo]] < data[permIndexes[start_hi]])
            lo++;
        else
        {
            /*
             *  data[permIndexes[lo]] >= data[permIndexes[start_hi]]
             *  The next element comes from the second list,
             *  move the data[permIndexes[start_hi]] element into the next
             *  position and shuffle all the other elements up.
             */
            int T = permIndexes[start_hi];
            for (int k = start_hi - 1; k >= lo; k--) {
                permIndexes[k+1] = permIndexes[k];
            }
            permIndexes[lo] = T;
            lo++;
            end_lo++;
            start_hi++;
        }
    }
}

#endif // LOWESSHELPERS_H
