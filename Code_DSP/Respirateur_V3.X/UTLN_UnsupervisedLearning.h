/* 
 * File:   UTLN_UnsupervisedLearning.h
 * Author: sebas
 *
 * Created on 21 septembre 2020, 17:07
 */

#ifndef UTLN_UNSUPERVISEDLEARNING_H
#define	UTLN_UNSUPERVISEDLEARNING_H

#ifdef	__cplusplus
extern "C" {
#endif

#define NeuronsInputLayerC          6
#define MaxQClustersC               25
#define ActualNumberOfClustersC     1
#define NumberSamplesTrainingSetC   5
    
    
typedef enum{
    First_Sample,
    Clusters_Comparison,        
}StateTraining;
    
    
void InitUnsupervisedKMeansModif(float DistanceMax, float alphaLPF);
int Training(float sample[]);
int AnomalyDetection(float sample[]);
int GetNumberOfClusters(void);
int GetClusterDimension(void);
float LPFKMeans(float xi, float yi_1, float alpha);

#ifdef	__cplusplus
}
#endif

#endif	/* UTLN_UNSUPERVISEDLEARNING_H */

