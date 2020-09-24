#include "UTLN_UnsupervisedLearning.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Define.h"


int QneuronsInputLayer = NeuronsInputLayerC;
int maxQClusters = MaxQClustersC;
int actualNumberOfClusters;// = ActualNumberOfClustersC;
int numberSamplesTrainingSet = NumberSamplesTrainingSetC;
int stateKMM = 0;

volatile StateTraining stateTraining = First_Sample;

int iterations = 0;

//Matrix of Clusters
float Clusters[MaxQClustersC][NeuronsInputLayerC] = {0.0f};
//Sample i
float Sample_i[NeuronsInputLayerC] = {0.0f};
//Sample i-1
float Sample_i_1[NeuronsInputLayerC] = {0.0f};

float AlphaLPF;

//Clusters to print
float ClusterToPrint[NeuronsInputLayerC] = {0.0f};
//int neuronsInputLayer = 0;
//int maxQClusters = 0;
//int actualNumberOfClusters = 0;
//int numberSamplesTrainingSet = 0;
//int state = 0;

float distanceMax = 1.0f;

int AnomaliesCounter = 0;

bool increaseNumberOfCluster = false;

//void InitUnsupervisedKMeansModif(int NeuronsInputLayer, int MaxQClusters,
//        int NumberSamplesTrainingSet, float DistanceMax, float alphaLPF)
void InitUnsupervisedKMeansModif(float DistanceMax, float alphaLPF)
{
    //Whole the variables for each cluster
    //QneuronsInputLayer = NeuronsInputLayer;
    //Maximum quantity of clusters
    //maxQClusters = MaxQClusters;
    //Number of samples for the training set
    //numberSamplesTrainingSet = NumberSamplesTrainingSet;
    
    //Distance max between the points and the cluster
    distanceMax = DistanceMax;
    //Initial Number of clusters
    actualNumberOfClusters = 1;
    //alpha low pass filter
    AlphaLPF = alphaLPF;

    increaseNumberOfCluster = false;
}

int GetNumberOfClusters(void){
    return actualNumberOfClusters;
}

//Training function
//Training will return:
//0 if training is not finished yet
//1 if training is finished 
//5 if training does not converge
int Training(float sample[]){

    float dist = 0;

    switch(stateTraining){
        case First_Sample :
        {
            //The first sample is asigned as the first cluster centroid 
            int i;
            for (i = 0; i < QneuronsInputLayer; i++)                
                Clusters[0][i] = sample[i];
            //At this moment we have only one Cluster
            actualNumberOfClusters = 1;

            iterations++;
            stateTraining = Clusters_Comparison;
            break;
        }
        case Clusters_Comparison :
        {
            //Compare with each cluster
            int i;
            for(i = 0; i <actualNumberOfClusters; i++)
            {
                dist = 0;
                int j;
                for (j = 0; j < QneuronsInputLayer; j++)
                {
                    //calculate distance between the sample and the cluster
                    dist += powf((Clusters[i][j] - sample[j])/Clusters[i][j], 2.0f);
                }

                dist = sqrtf(dist);
                if (dist < distanceMax)
                {
                    //If dist is lower than distanceMax, this sample belong to the Actual cluster
                    //This cluster is rectified with the average of each dimensions                    
                    for (j = 0; j < QneuronsInputLayer; j++)
                    {
                        //Clusters[jjjC][jjj] = (sample[jjj] + Clusters[jjjC][jjj]) / 2;
                        Clusters[i][j] = LPFKMeans(sample[j],Clusters[i][j],AlphaLPF);
                    }
                    iterations++;
                    if(iterations < numberSamplesTrainingSet)
                        return 0;
                    else
                        return 1;
                }
            }
            //if all the clusters had been compared
            //New cluster had been detected
            for (i = 0; i < QneuronsInputLayer; i++)
            {
                //Save sample in a new cluster
                Clusters[actualNumberOfClusters][i] = sample[i];
            }
            actualNumberOfClusters++;
            #if COMUSBDEBUG
                PrintClusterUSB(sample, QneuronsInputLayer,
                            actualNumberOfClusters+1);
            #endif
            if(actualNumberOfClusters == maxQClusters)
                return 5;
            iterations++;
            break;
        }
    }

    if(iterations < numberSamplesTrainingSet)
        return 0;
    else
        return 1;
}

int AnomalyDetection(float sample[]){

    float dist = 0;

//    switch(stateTraining){
//        case First_Sample :
//        {
//            //The first sample is asigned as the first cluster centroid 
//            int i;
//            for (i = 0; i < QneuronsInputLayer; i++)                
//                Clusters[0][i] = sample[i];
//            //At this moment we have only one Cluster
//            actualNumberOfClusters = 1;
//
//            iterations++;
//            stateTraining = Clusters_Comparison;
//            break;
//        }
//        case Clusters_Comparison :
//        {
            //Compare with each cluster
            int i;
            for(i = 0; i <actualNumberOfClusters; i++)
            {
                dist = 0;
                int j;
                for (j = 0; j < QneuronsInputLayer; j++)
                {
                    //calculate distance between the sample and the cluster
                    dist += powf((Clusters[i][j] - sample[j])/Clusters[i][j], 2.0f);
                }

                dist = sqrtf(dist);
                if (dist < distanceMax)
                {
                    //If dist is lower than distanceMax, this sample belong to the Actual cluster
                    //This cluster is rectified with the average of each dimensions                    
                    for (j = 0; j < QneuronsInputLayer; j++)
                    {
                        //Clusters[jjjC][jjj] = (sample[jjj] + Clusters[jjjC][jjj]) / 2;
                        Clusters[i][j] = LPFKMeans(sample[j],Clusters[i][j],AlphaLPF);
                    }
//                    iterations++;
//                    if(iterations < numberSamplesTrainingSet)
                        return AnomaliesCounter;
//                    else
//                        return 1;
                }
            }
            //if all the clusters had been compared
            AnomaliesCounter++;
            //Anomaly detected!
            return AnomaliesCounter;
//        }
//    }

        return 0;
}

//Anomaly detection function
//After training function returns 1 we could use this one
//it will return:
//                  0 if the sample belongs to some cluster 
//                  1 if the sample is an anomaly
//int AnomalyDetection(float sample[]){
//
//    float dist = 0;
//    int i;
//    int j;
//    //Compare with each cluster
//    for(i = 0; i <actualNumberOfClusters; i++)
//    {
//        dist = 0;
//        for (j = 0; j < QneuronsInputLayer; j++)
//        {
//            //calculate distance between the sample and the cluster
//            //dist += Math.Pow(Clusters[jjjC, jjj], 2) + Math.Pow(Sample_i[jjj], 2);
//            dist += powf((Clusters[i][j] - sample[j])/Clusters[i][j], 2.0f);
//        }
//        dist = sqrtf(dist);
//        if (dist < distanceMax)
//        {
//            //If dist is lower than distanceMax, this sample belong to the Actual cluster
//            //This cluster is rectified with the average of each dimensions
//            for (j = 0; j < QneuronsInputLayer; j++)
//            {
//                //Clusters[jjjC][jjj] = (sample[jjj] + Clusters[jjjC][jjj]) / 2;
//                Clusters[i][j] = LPFKMeans(Sample_i[j],Clusters[i][j],AlphaLPF);
//            }
//            return AnomaliesCounter;
//        }
//        else
//        {
//            //if all the clusters had been compared
//            if(i == (actualNumberOfClusters - 1))
//            {
//                AnomaliesCounter++;
//                //Anomaly detected!
//                return AnomaliesCounter;
//            }
//        }
//    }
//    return 0;
//
//}

//Low pass filter function to modify the clusters centroids 
float LPFKMeans(float xi, float yi_1, float alpha)
{
    float yi = (1 - alpha) * yi_1 + alpha * xi;
    return yi;
}

int GetClusterDimension(void){
    return NeuronsInputLayerC;
}