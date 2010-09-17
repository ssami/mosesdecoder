#include "Optimiser.h"

using namespace Moses;
using namespace std;

namespace Mira {
	void MiraOptimiser::updateWeights(Moses::ScoreComponentCollection& weights,
                         	const std::vector< std::vector<Moses::ScoreComponentCollection> >& scores,
                         	const std::vector< std::vector<float> >& losses,
                         	const Moses::ScoreComponentCollection& oracleScores) {

	for(unsigned batch = 0; batch < scores.size(); batch++) {

	  Moses::ScoreComponentCollection oldWeights(weights);
	  float maxTranslation = -1000.0; //what wrong with FLT_MIN ?!

	  for(unsigned analyseSentence = 0; analyseSentence < scores[batch].size(); analyseSentence++) {

            /* do this:
            for(unsigned score = 0; score < scores[batch][analyseSentence].size(); score++) {
              float currentScoreChange = oracleScores[score] - scores[batch][analyseSentence][score];
              scoreChange += currentScoreChange * weights[score];
              norm += currentScoreChange * currentScoreChange;
            }
            */ 
            Moses::ScoreComponentCollection currentScoreColl = oracleScores;
            currentScoreColl.MinusEquals(scores[batch][analyseSentence]);
	    currentScoreColl.MultiplyEquals(weights);
	    float scoreChange = currentScoreColl.InnerProduct(weights);
	    float norm = currentScoreColl.InnerProduct(currentScoreColl);	     

            float delta;
            if(norm == 0.0) //just in case... :-)
              delta = 0.0;
            else {
              
              delta = (losses[batch][analyseSentence] - scoreChange) / norm;

	      //now get in shape
              if(delta > upperBound_)
                delta = upperBound_;
              else if(delta < lowerBound_)
                delta = lowerBound_;
	
              cout << "scoreChange: " << scoreChange
                   << "\ndelta: " << delta
                   << "\nloss: " << losses[batch][analyseSentence] << endl;
            }
           
 	    // do this:	weights += delta * (oracleScores - scores[batch][analyseSentence])
            Moses::ScoreComponentCollection tempColl = oracleScores;
            tempColl.MinusEquals(scores[batch][analyseSentence]);
	    tempColl.MultiplyEquals(delta);
	    weights.MinusEquals(tempColl);

	    float tmp = losses[batch][analyseSentence] - oracleScores.InnerProduct(weights);
	    if(tmp > maxTranslation)
		maxTranslation = tmp;	
			
            //calculate max. for criterion
            /*
 	    float sumWeightedFeatures = 0.0;
            for(unsigned score = 0; score < scores[analyseSentence]->size(); score++) {
              sumWeightedFeatures += oracleScores[score]*newWeights[score];
            }

	    if((losses[analyseSentence] - sumWeightedFeatures) > maxTranslation_) {
              maxTranslation_ = losses[analyseSentence] - sumWeightedFeatures;
	    } 
	    */
          }
	  oldWeights.MinusEquals(weights);
	  float criterion = 0.5*oldWeights.InnerProduct(oldWeights) + 0.01*maxTranslation;
	  cout << "criterion: " << criterion << endl;
	}
  }
}

