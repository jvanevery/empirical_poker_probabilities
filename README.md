# empirical_poker_probabilities

Strictly for educational purposes. Developed in CS241 at UNM.

Empirical Poker Hand Probability Generator * Jordan VanEvery ~ Last Modified: 2017-3-2  * * The program in its current state determines the probability of  * improving a five card poker hand when allowed to discard a single * card from the hand and replace it with a remaining card in the deck * An empirical method is used, where a card it replaced at random  * many times, and upon each replacement it is determined if the  * hand state improved. *  * The precision of the probabilities determined arbitrarily at this * point. No check for the convergence of the probability is made,  * rather a "sampleNumber" is preset that determines what number of * times a new card will be drawn. If time permits: the difference in * subsequent probabilities could be checked upon each replacement * until the change in a prob. is less than some desired precision. * * Example input/output: 2D 2C 5H 2H 2S  * --->2D 2C 5H 2H 2S >>>Four of a Kind 0.0% 0.0% 0.0% 0.0% 0.0% 

