#ifdef _OPENMP
#include <omp.h>
#endif

#include <cmath>
#include "KMeans_OpenMP.h"





bool checkEqualClusters2(std::vector<Point> dataset, std::vector<Point> oldDataset, int numPoint){
    for (int i = 0; i < numPoint; i++){
        if(dataset[i].clusterId != oldDataset[i].clusterId){
            return false;
        }
    }
    return true;
}


std::tuple<std::vector<Point>,std::vector<Point>> openMP_kmeans(std::vector<Point> dataset, std::vector<Point> centroids, int k) {
    bool convergence = false, firstIteration = true;
    std::vector<Point> oldDataset;
    //Number of dataset points
    auto numPoint = dataset.size();
    //Points dimension
    auto dimPoint = dataset[0].dimensions.size();

    int clusterLabel;
    double privateMinDistance, privateDistance;
    std::vector<int> count(k);



    while (!convergence) {

#pragma omp parallel num_threads(4) default(none) \
    private(privateDistance, privateMinDistance, clusterLabel)\
    shared(numPoint, k, dimPoint, dataset, centroids, count)
        {



            //Find the nearest centroid and assign the point to that cluster
#pragma omp for
            for (int i = 0; i < numPoint; i++) {
                privateMinDistance = std::numeric_limits<double>::max();
                for (int j = 0; j < k; j++) {
                    //distance = 0;
                    privateDistance = 0;
                    for (int h = 0; h < dimPoint; h++) {
                        privateDistance += pow(dataset[i].dimensions[h] - centroids[j].dimensions[h], 2);
                    }
                    privateDistance = sqrt(privateDistance);
                    if (privateDistance < privateMinDistance) {
                        privateMinDistance = privateDistance;
                        clusterLabel = j;
                    }
                }
                dataset[i].clusterId = clusterLabel;
            }



#pragma omp for
            for (int j = 0; j < k; j++) {
                count[j]=0;
            }


#pragma omp for collapse(2)
            for (int j = 0; j < k; j++) {
                for(int h = 0; h<dimPoint; h++){
                    centroids[j].dimensions[h] = 0;
                }
            }


#pragma omp for
            for (int i = 0; i < numPoint; i++) {
                for (int h = 0; h < dimPoint; h++) {
#pragma omp atomic
                    centroids[dataset[i].clusterId].dimensions[h] += dataset[i].dimensions[h];
                }
#pragma omp atomic
                count[dataset[i].clusterId]++;
            }

#pragma omp for collapse(2)
            for (int j = 0; j < k; j++) {
                for (int h = 0; h < dimPoint; h++) {
#pragma omp atomic
                    centroids[j].dimensions[h] /= count[j];
                }
            }
        }

        if (!firstIteration && checkEqualClusters2(dataset, oldDataset, numPoint)) {
            convergence = true;
        } else {
            oldDataset = dataset;
            firstIteration = false;
        }
    }
    return {dataset, centroids};

}


