//score.h
#ifndef SCORE_H
#define SCORE_H


enum DartResult { CONTINUE, BUST, WIN }; // like a bool but with 3 cases

int calculateDartScore(int number, char multiplier); // two inputs number and multiplier
bool isValidDart(int number, char multiplier); // check if inputs are valid
DartResult applyDart(int& remScore, int dartScore, char multiplier); // remScore is referenced because changes should be made to
                                                                    // the variable directly not a copy of the variable

#endif