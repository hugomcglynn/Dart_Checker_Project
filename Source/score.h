// Hugo McGlynn declared single-dart scoring. Codex added visit scoring.

#ifndef SCORE_H
#define SCORE_H

enum DartResult { CONTINUE, BUST, WIN };

int calculateDartScore(int number, char multiplier);
bool isValidDart(int number, char multiplier);
DartResult applyDart(int& remScore, int dartScore, char multiplier);

bool isPossibleVisit(int total, int darts);
bool isValidVisit(int remaining, int total, int darts, bool finishedOnDouble);
DartResult applyVisit(int& remaining, int total, bool finishedOnDouble);

#endif
